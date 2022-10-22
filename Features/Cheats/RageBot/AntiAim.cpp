// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "antiaim.h"
#include "knifebot.h"
#include "zeusbot.h"
#include "..\misc\fakelag.h"
#include "..\Prediction\EnginePrediction.h"
#include "..\misc\misc.h"
#include "..\lagcompensation\LocalAnimations.h"

bool antiaim::CanAntiAim(CUserCmd* m_pcmd, bool extra_check) {
	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();
	if (!weapon
		|| !m_pcmd
		|| !g_ctx.available()
		|| !g_ctx.local()->is_alive()
		|| g_ctx.local()->m_bGunGameImmunity()
		|| g_ctx.local()->m_fFlags() & FL_FROZEN
		|| g_ctx.local()->get_move_type() == MOVETYPE_NOCLIP
		|| g_ctx.local()->get_move_type() == MOVETYPE_LADDER
		|| (m_pcmd->m_buttons & IN_ATTACK && weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && !weapon->is_non_aim())
		|| (weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2))
		|| ((m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2) && weapon->is_knife())
		|| (extra_check && freeze_check)
		|| (extra_check && m_pcmd->m_buttons & IN_USE)
		|| (extra_check && weapon->is_grenade() && weapon->m_fThrowTime())
		) return false;

	return true;
}

void antiaim::RunAntiaim(CUserCmd* m_pcmd)
{
	if (!CanAntiAim(m_pcmd))
		return;

	if ((!g_ctx.globals.weapon->is_grenade() || cfg::g_cfg.visuals.on_click && !(m_pcmd->m_buttons & IN_ATTACK) && !(m_pcmd->m_buttons & IN_ATTACK2)) && sqrtf((m_pcmd->m_forwardmove * m_pcmd->m_forwardmove) + (m_pcmd->m_sidemove * m_pcmd->m_sidemove)) < 0.1f && sqrtf((g_ctx.local()->m_vecVelocity().y * g_ctx.local()->m_vecVelocity().y) + (g_ctx.local()->m_vecVelocity().x * g_ctx.local()->m_vecVelocity().x)) < 0.1f)
	{
		auto speed = 1.01f;

		if (g_ctx.local()->m_bDucking() || g_ctx.local()->m_fFlags() & FL_DUCKING || g_ctx.globals.fakeducking)
			speed = speed / (((g_ctx.local()->m_flDuckAmount() * 0.34f) + 1.0f) - g_ctx.local()->m_flDuckAmount());

		if (!(m_pcmd->m_command_number & 1))
			speed *= -1;

		m_pcmd->m_forwardmove = speed;
	}

	m_pcmd->m_viewangles.x = GetPitch(m_pcmd);
	m_pcmd->m_viewangles.y = GetYaw(m_pcmd);
	m_pcmd->m_viewangles.z = flip ? cfg::g_cfg.antiaim.roll : -cfg::g_cfg.antiaim.roll;
}

float antiaim::GetPitch(CUserCmd* m_pcmd) {
	float pitch = 0.f;

	switch (cfg::g_cfg.antiaim.pitch) {
	case 1:
		pitch = 88.914012f;
		break;
	case 2:
		pitch = -88.914012f;
		break;
	case 3:
		pitch = cfg::g_cfg.antiaim.custom_pitch;
		break;
	}

	//We use this for local animations
	local_pitch = pitch;
	return pitch;
}

float antiaim::GetYawDirection(CUserCmd* m_pcmd) {
	float yaw = 0.f;
	switch (cfg::g_cfg.antiaim.yaw_direction) {
	case 0:
		yaw = 180.f;
		break;
	case 1:
		yaw = 0;
		break;
	}

	return yaw;
}

float antiaim::GetYaw(CUserCmd* m_pcmd)
{
	//Vars
	auto yaw = 0.0f;
	auto lby_type = 0;
	bool can_jitter = true;
	static auto sway_counter = 0;
	static auto force_choke = false;
	static auto invert_jitter = false;
	static auto should_invert = false;
	auto max_desync_delta = g_ctx.local()->get_max_desync_delta();

	final_manual_side = manual_side;

	//Weird fix, idefk.
	if (g_ctx.send_packet)
		should_invert = true;
	else if (!g_ctx.send_packet && should_invert) {
		should_invert = false;
		invert_jitter = !invert_jitter;
	}

	auto yaw_add_angle = flip ? cfg::g_cfg.antiaim.yaw_add_left : cfg::g_cfg.antiaim.yaw_add_right;
	auto base_angle = m_pcmd->m_viewangles.y + GetYawDirection(m_pcmd) + yaw_add_angle;

	if (manual_side == SIDE_NONE && cfg::g_cfg.antiaim.freestand && key_binds::get().get_key_bind_state(24))
		Freestanding(m_pcmd);

	//Manual sides.
	if (final_manual_side == SIDE_LEFT)
		base_angle -= 90.0f;
	if (final_manual_side == SIDE_RIGHT)
		base_angle += 90.0f;

	//At targets.
	if (cfg::g_cfg.antiaim.base_angle && manual_side == SIDE_NONE)
		base_angle = GetAtTargetAngles();

	if (cfg::g_cfg.antiaim.desync == 1)
		flip = key_binds::get().get_key_bind_state(16);


	auto yaw_angle = 0.0f;

	//Jitter & spin anti-aim.
	switch (cfg::g_cfg.antiaim.yaw)
	{
	case 1:
		if (manual_side == SIDE_NONE)
			yaw_angle = invert_jitter ? 0.f : (float)cfg::g_cfg.antiaim.range;
		break;
	case 2:
		if (manual_side == SIDE_NONE)
		    yaw_angle = invert_jitter ? (float)cfg::g_cfg.antiaim.range / 2 : -(float)cfg::g_cfg.antiaim.range / 2;
		break;
	}

	//Set our default desync angle again.
	desync_angle = 0.0f;

	//if > 0 then its on (static & jitter & center jitter)
	if (cfg::g_cfg.antiaim.desync)
	{
		if (cfg::g_cfg.antiaim.desync == 2)
			flip = invert_jitter;

		auto desync_delta = max_desync_delta;
		desync_delta = (float)math::clamp(cfg::g_cfg.antiaim.desync_range / 3.45, 0, 29);

		if (!flip)
		{
			desync_delta = -desync_delta;
			max_desync_delta = -max_desync_delta;
		}

		base_angle -= desync_delta;
		desync_angle = desync_delta;
	}

	//Finally, set our angle.
	yaw = base_angle + yaw_angle;

	//Return current if we don't have desync on.
	if (!desync_angle)
		return yaw;

	//Do desync shit lololol.
	if (ShouldBreakLowerBody(m_pcmd, 1))
	{
		auto speed = 1.01f;

		if (m_pcmd->m_buttons & IN_DUCK || g_ctx.globals.fakeducking)
			speed *= 2.94117647f;

		static auto switch_move = false;

		if (switch_move)
			m_pcmd->m_sidemove += speed;
		else
			m_pcmd->m_sidemove -= speed;

		switch_move = !switch_move;

		if (sway_counter > 3)
		{
			if (desync_angle > 0.0f)
				yaw -= 180.0f;
			else
				yaw += 180.0f;
		}

		if (sway_counter < 8)
			++sway_counter;
		else
			sway_counter = 0;

		breaking_lby = true;
		force_choke = true;
		g_ctx.send_packet = false;

		return yaw;
	}
	else if (force_choke)
	{
		force_choke = false;
		g_ctx.send_packet = false;

		return yaw;
	}
	else if (g_ctx.send_packet)
		yaw += desync_angle;

	//Finally, return compensated yaw value for desync.
	return yaw;
}

bool antiaim::ShouldBreakLowerBody(CUserCmd* m_pcmd, int lby_type) {
	if (g_ctx.globals.tochargeamount > 0)
		return false;

	if (g_ctx.globals.fakeducking && m_clientstate()->iChokedCommands > 12)
		return false;

	if (!g_ctx.globals.fakeducking && m_clientstate()->iChokedCommands > 14)
	{
		g_ctx.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	auto animstate = g_ctx.local()->get_animation_state(); //-V807

	if (!animstate)
		return false;

	if (animstate->m_velocity > 0.1f || fabs(animstate->flUpVelocity) > 100.0f)
		g_ctx.globals.next_lby_update = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase + 14);
	else
	{
		if (TICKS_TO_TIME(g_ctx.globals.fixed_tickbase) > g_ctx.globals.next_lby_update)
		{
			g_ctx.globals.next_lby_update = 0.0f;
			return true;
		}
	}

	return false;
}

float antiaim::GetAtTargetAngles() {
	player_t* target = nullptr;
	auto best_fov = FLT_MAX;

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!e->valid(true))
			continue;

		auto weapon = e->m_hActiveWeapon().Get();

		if (!weapon)
			continue;

		if (weapon->is_non_aim())
			continue;

		Vector angles;
		m_engine()->GetViewAngles(angles);

		auto fov = math::get_fov(angles, math::calculate_angle(g_ctx.globals.eye_pos, e->GetAbsOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			target = e;
		}
	}

	auto angle = 180.0f;

	if (manual_side == SIDE_LEFT)
		angle = 90.0f;
	else if (manual_side == SIDE_RIGHT)
		angle = -90.0f;

	if (!target)
		return g_ctx.get_command()->m_viewangles.y + angle;

	return math::calculate_angle(g_ctx.globals.eye_pos, target->GetAbsOrigin()).y + angle;
}

void antiaim::Freestanding(CUserCmd* m_pcmd)
{
	float Right, Left;
	Vector src3D, dst3D, forward, right, up;
	trace_t tr;
	Ray_t ray_right, ray_left;
	CTraceFilter filter;

	Vector engineViewAngles;
	m_engine()->GetViewAngles(engineViewAngles);
	engineViewAngles.x = 0.0f;

	math::angle_vectors(engineViewAngles, &forward, &right, &up);

	filter.pSkip = g_ctx.local();
	src3D = g_ctx.globals.eye_pos;
	dst3D = src3D + forward * 100.0f;

	ray_right.Init(src3D + right * 35.0f, dst3D + right * 35.0f);

	g_ctx.globals.autowalling = true;
	m_trace()->TraceRay(ray_right, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	g_ctx.globals.autowalling = false;

	Right = (tr.endpos - tr.startpos).Length();

	ray_left.Init(src3D - right * 35.0f, dst3D - right * 35.0f);

	g_ctx.globals.autowalling = true;
	m_trace()->TraceRay(ray_left, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	g_ctx.globals.autowalling = false;

	Left = (tr.endpos - tr.startpos).Length();

	static auto left_ticks = 0;
	static auto right_ticks = 0;
	static auto back_ticks = 0;

	if (Right - Left > 20.0f)
		left_ticks++;
	else
		left_ticks = 0;

	if (Left - Right > 20.0f)
		right_ticks++;
	else
		right_ticks = 0;

	if (fabs(Right - Left) <= 20.0f)
		back_ticks++;
	else
		back_ticks = 0;

	if (right_ticks > 10)
		final_manual_side = SIDE_RIGHT;
	else if (left_ticks > 10)
		final_manual_side = SIDE_LEFT;
	else if (back_ticks > 10)
		final_manual_side = SIDE_BACK;
}