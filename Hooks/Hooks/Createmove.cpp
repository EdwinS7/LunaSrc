// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\Features\cheats\ragebot\antiaim.h"
#include "..\..\Features\cheats\visuals\OtherEsp.h"
#include "..\..\Features\cheats\misc\fakelag.h"
#include "..\..\Features\cheats\Prediction\EnginePrediction.h"
#include "..\..\Features\cheats\ragebot\ragebot.h"
#include "..\..\Features\cheats\legitbot\legitbot.h"
#include "..\..\Features\cheats\Movement\Bhop.h"
#include "..\..\Features\cheats\Movement\AutoStrafer.h"
#include "..\..\Features\cheats\misc\clantag.h"
#include "..\..\Features\cheats\movement\slowwalk.h"
#include "..\..\Features\cheats\misc\misc.h"
#include "..\..\Features\cheats\misc\logs.h"
#include "..\..\Features\cheats\visuals\GrenadePrediction.h"
#include "..\..\Features\cheats\ragebot\knifebot.h"
#include "..\..\Features\cheats\ragebot\zeusbot.h"
#include "..\..\Features\cheats\lagcompensation\LocalAnimations.h"
#include "..\..\Features\cheats\lagcompensation\LagCompensation.h"
#include "..\..\Features\cheats\exploits\TickBase.h"
#include "..\..\Features\cheats\legitbot\LegitBacktrack.h"
#include "..\..\Features\cheats\buybot\buybot.h"


using CreateMove_t = void(__stdcall*)(int, float, bool);
void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& send_packet)
{
	static auto original_fn = hooks::client_hook->get_func_address <CreateMove_t>(22);
	original_fn(sequence_number, input_sample_frametime, active);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	auto m_pcmd = m_input()->GetUserCmd(sequence_number);
	auto verified = m_input()->GetVerifiedUserCmd(sequence_number);

	g_ctx.globals.in_createmove = false;
	Vector wish_yaw = m_pcmd->m_viewangles;

	if (!m_pcmd)
		return;

	if (!m_pcmd->m_command_number)
		return;

	if (original_fn)
	{
		m_prediction()->SetLocalViewAngles(m_pcmd->m_viewangles);
		m_engine()->SetViewAngles(m_pcmd->m_viewangles);
	}

	if (!g_ctx.available())
		return;

	spammers::get().clan_tag();

	if (!g_ctx.local()->is_alive())
		return;

	g_ctx.globals.weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!g_ctx.globals.weapon)
		return;

	g_ctx.globals.in_createmove = true;
	g_ctx.set_command(m_pcmd);

	if (hooks::menu_open && g_ctx.globals.focused_on_input)
	{
		m_pcmd->m_buttons = 0;
		m_pcmd->m_forwardmove = 0.0f;
		m_pcmd->m_sidemove = 0.0f;
		m_pcmd->m_upmove = 0.0f;
	}

	g_ctx.globals.backup_tickbase = g_ctx.local()->m_nTickBase();


	//if (g_ctx.globals.tickbase_shift)
	//	g_ctx.globals.fixed_tickbase = g_ctx.local()->m_nTickBase() - 15;
	//else
	//	g_ctx.globals.fixed_tickbase = g_ctx.globals.backup_tickbase;

	g_ctx.globals.fixed_tickbase = g_ctx.local()->m_nTickBase() - g_ctx.globals.next_tickbase_shift;

	if (hooks::menu_open)
	{
		m_pcmd->m_buttons &= ~IN_ATTACK;
		m_pcmd->m_buttons &= ~IN_ATTACK2;
	}

	if (m_pcmd->m_buttons & IN_ATTACK2 && cfg::g_cfg.ragebot.enable && g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		m_pcmd->m_buttons &= ~IN_ATTACK2;

	if (cfg::g_cfg.ragebot.enable && !g_ctx.globals.weapon->is_non_aim() && g_ctx.globals.weapon->get_weapon_group(true) != 4 && !g_ctx.globals.weapon->can_fire(true))
	{
		if (m_pcmd->m_buttons & IN_ATTACK && !g_ctx.globals.weapon->is_non_aim() && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
			m_pcmd->m_buttons &= ~IN_ATTACK;
		else if ((m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2) && (g_ctx.globals.weapon->is_knife() || g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER) && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_HEALTHSHOT)
		{
			if (m_pcmd->m_buttons & IN_ATTACK)
				m_pcmd->m_buttons &= ~IN_ATTACK;

			if (m_pcmd->m_buttons & IN_ATTACK2)
				m_pcmd->m_buttons &= ~IN_ATTACK2;
		}
	}

	if (m_pcmd->m_buttons & IN_FORWARD && m_pcmd->m_buttons & IN_BACK)
	{
		m_pcmd->m_buttons &= ~IN_FORWARD;
		m_pcmd->m_buttons &= ~IN_BACK;
	}

	if (m_pcmd->m_buttons & IN_MOVELEFT && m_pcmd->m_buttons & IN_MOVERIGHT)
	{
		m_pcmd->m_buttons &= ~IN_MOVELEFT;
		m_pcmd->m_buttons &= ~IN_MOVERIGHT;
	}

	g_ctx.send_packet = true;
	g_ctx.globals.tickbase_shift = 0;
	g_ctx.globals.double_tap_fire = false;
	g_ctx.globals.force_send_packet = false;
	g_ctx.globals.exploits = exploit::get().double_tap_key || exploit::get().hide_shots_key;
	g_ctx.globals.current_weapon = g_ctx.globals.weapon->get_weapon_group(cfg::g_cfg.ragebot.enable);
	g_ctx.globals.slowwalking = false;
	g_ctx.globals.original_forwardmove = m_pcmd->m_forwardmove;
	g_ctx.globals.original_sidemove = m_pcmd->m_sidemove;

	antiaim::get().breaking_lby = false;

	auto wish_angle = m_pcmd->m_viewangles;

	misc::get().fast_stop(m_pcmd);

	if (cfg::g_cfg.misc.bunnyhop)
		bunnyhop::get().create_move();

	misc::get().slidewalk(m_pcmd);
	misc::get().infiniteduck_stamina(m_pcmd);
	misc::get().fakeduck(m_pcmd);

	GrenadePrediction::get().Tick(m_pcmd->m_buttons);

	if (cfg::g_cfg.misc.crouch_in_air && !(g_ctx.local()->m_fFlags() & FL_ONGROUND))
		m_pcmd->m_buttons |= IN_DUCK;

	engineprediction::get().prediction_data.reset();
	engineprediction::get().setup();
	engineprediction::get().predict(m_pcmd);

	g_ctx.globals.eye_pos = g_ctx.local()->get_shoot_position();

	if (cfg::g_cfg.misc.airstrafe)
		airstrafe::get().create_move(m_pcmd);

	if (key_binds::get().get_key_bind_state(19) && engineprediction::get().backup_data.flags & FL_ONGROUND && !(g_ctx.local()->m_fFlags() & FL_ONGROUND)) //-V807
		m_pcmd->m_buttons |= IN_JUMP;

	if (cfg::g_cfg.misc.slowwalk_type == 1 && key_binds::get().get_key_bind_state(21) && engineprediction::get().backup_data.flags & FL_ONGROUND && g_ctx.local()->m_fFlags() & FL_ONGROUND)
		slowwalk::get().create_move(m_pcmd, cfg::g_cfg.misc.slowwalk_speed);
	else if (cfg::g_cfg.misc.slowwalk_type == 0 && key_binds::get().get_key_bind_state(21) && engineprediction::get().backup_data.flags & FL_ONGROUND && g_ctx.local()->m_fFlags() & FL_ONGROUND)
		slowwalk::get().create_move(m_pcmd, 0.25f);

	if (!g_ctx.globals.startcharge)
		fakelag::get().Createmove();

	g_ctx.globals.aimbot_working = false;
	g_ctx.globals.revolver_working = false;

	auto backup_velocity = g_ctx.local()->m_vecVelocity();
	auto backup_abs_velocity = g_ctx.local()->m_vecAbsVelocity();

	g_ctx.local()->m_vecVelocity() = engineprediction::get().backup_data.velocity;
	g_ctx.local()->m_vecAbsVelocity() = engineprediction::get().backup_data.velocity;

	g_ctx.globals.weapon->update_accuracy_penality();

	g_ctx.local()->m_vecVelocity() = backup_velocity;
	g_ctx.local()->m_vecAbsVelocity() = backup_abs_velocity;

	g_ctx.globals.inaccuracy = g_ctx.globals.weapon->get_inaccuracy();
	g_ctx.globals.spread = g_ctx.globals.weapon->get_spread();


	Rbot::get().Run(m_pcmd);
	zeusbot::get().run(m_pcmd);
	knifebot::get().run(m_pcmd);

	legit_bot::get().createmove(m_pcmd);
	if (cfg::g_cfg.legitbot.backtrack)
		NewBacktrack::Get().LegitBacktrack(m_pcmd);

	misc::get().automatic_peek(m_pcmd, wish_angle.y);

	antiaim::get().desync_angle = 0.0f;

	antiaim::get().RunAntiaim(m_pcmd);

	//Extended backtrack
	if (g_ctx.local()->is_alive() && cfg::g_cfg.misc.extended_backtracking && !hooks::netchan_hook) {
		auto netchannel = m_clientstate()->pNetChannel;
		if (netchannel) {
			hooks::netchan_hook = new vmthook(reinterpret_cast<DWORD**>(netchannel));
			hooks::netchan_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkSendDataGram), 46);
		}
	}


	if (m_clientstate()->iChokedCommands >= (m_gamerules()->m_bIsValveDS() ? 6 : 14))
	{
		g_ctx.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	if (g_ctx.globals.should_send_packet)
	{
		g_ctx.globals.force_send_packet = true;
		g_ctx.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	if (g_ctx.globals.should_choke_packet)
	{
		g_ctx.globals.should_choke_packet = false;
		g_ctx.globals.should_send_packet = true;
		g_ctx.send_packet = false;
	}

	if (!g_ctx.globals.weapon->is_non_aim())
	{
		auto double_tap_aim_check = false;

		if (m_pcmd->m_buttons & IN_ATTACK && g_ctx.globals.double_tap_aim_check)
		{
			double_tap_aim_check = true;
			g_ctx.globals.double_tap_aim_check = false;
		}

		auto revolver_shoot = g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);

		if (m_pcmd->m_buttons & IN_ATTACK && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || revolver_shoot)
		{
			static auto weapon_recoil_scale = m_cvar()->FindVar(crypt_str("weapon_recoil_scale"));

			if (cfg::g_cfg.ragebot.enable)
				m_pcmd->m_viewangles -= g_ctx.local()->m_aimPunchAngle() * weapon_recoil_scale->GetFloat();

			if (!g_ctx.globals.fakeducking)
			{
				g_ctx.globals.force_send_packet = true;
				g_ctx.globals.should_choke_packet = true;
				g_ctx.send_packet = true;
				fakelag::get().started_peeking = false;
			}

			Rbot::get().last_shoot_position = g_ctx.globals.eye_pos;
			g_ctx.globals.m_ragebot_shot_nr = m_pcmd->m_command_number;

			if (!double_tap_aim_check)
				g_ctx.globals.double_tap_aim = false;
		}
	}
	else if (g_ctx.globals.weapon->is_knife() && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2))
	{
		if (!g_ctx.globals.fakeducking) {
			g_ctx.globals.force_send_packet = true;
			g_ctx.globals.should_choke_packet = true;
			g_ctx.send_packet = true;
			fakelag::get().started_peeking = false;
		}

		g_ctx.globals.m_ragebot_shot_nr = m_pcmd->m_command_number;
	}

	if (g_ctx.globals.fakeducking)
		g_ctx.globals.force_send_packet = g_ctx.send_packet;

	for (auto& backup : Rbot::get().backup)
		backup.adjust_player();

	auto backup_ticks_allowed = g_ctx.globals.ticks_allowed;

	if (g_ctx.globals.isshifting)
	{
		g_ctx.send_packet = g_ctx.globals.shift_ticks == 1;
		m_pcmd->m_buttons &= ~(IN_ATTACK | IN_ATTACK2);
		//return;
	}

	exploit::get().DoubleTap(m_pcmd);
	exploit::get().HideShots(m_pcmd);

	if (!g_ctx.globals.weapon->is_non_aim())
	{
		auto double_tap_aim_check = false;

		if (m_pcmd->m_buttons & IN_ATTACK && g_ctx.globals.double_tap_aim_check)
		{
			double_tap_aim_check = true;
			g_ctx.globals.double_tap_aim_check = false;
		}

		auto revolver_shoot = g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);

		if (!double_tap_aim_check && m_pcmd->m_buttons & IN_ATTACK && g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || revolver_shoot)
			g_ctx.globals.double_tap_aim = false;
	}

	if (m_globals()->m_tickcount - g_ctx.globals.last_aimbot_shot > 16) //-V807
	{
		g_ctx.globals.double_tap_aim = false;
		g_ctx.globals.double_tap_aim_check = false;
	}

	engineprediction::get().finish();

	BuyBot::get().Buy();
	misc::get().rank_reveal(m_pcmd);

	if (g_ctx.globals.loaded_script)
		for (auto current : c_lua::get().hooks.getHooks(crypt_str("on_createmove")))
			current.func();

	if (cfg::g_cfg.misc.anti_untrusted)
		math::normalize_angles(m_pcmd->m_viewangles);
	else
		m_pcmd->m_viewangles.y = math::normalize_yaw(m_pcmd->m_viewangles.y);

	util::movement_fix(wish_angle, m_pcmd);

	//if (g_ctx.globals.startcharge)
	//	g_ctx.send_packet = true;

	static auto previous_ticks_allowed = g_ctx.globals.ticks_allowed;


	if (g_ctx.send_packet && m_clientstate()->pNetChannel)
	{
		auto choked_packets = m_clientstate()->pNetChannel->m_nChokedPackets;

		if (choked_packets >= 0)
		{
			auto ticks_allowed = g_ctx.globals.ticks_allowed;
			auto command_number = m_pcmd->m_command_number - choked_packets;

			do
			{
				auto command = &m_input()->m_pCommands[m_pcmd->m_command_number - MULTIPLAYER_BACKUP * (command_number / MULTIPLAYER_BACKUP) - choked_packets];

				if (!command || command->m_tickcount > m_globals()->m_tickcount + int(1 / m_globals()->m_intervalpertick) + 8)
				{
					if (--ticks_allowed < 0)
						ticks_allowed = 0;

					g_ctx.globals.ticks_allowed = ticks_allowed;
				}

				++command_number;
				--choked_packets;
			} while (choked_packets >= 0);
		}
	}

	if (g_ctx.globals.ticks_allowed > 17)
		g_ctx.globals.ticks_allowed = math::clamp(g_ctx.globals.ticks_allowed - 1, 0, 17);

	if (previous_ticks_allowed && !g_ctx.globals.ticks_allowed)
		g_ctx.globals.ticks_choke = 16;

	previous_ticks_allowed = g_ctx.globals.ticks_allowed;

	if (g_ctx.globals.ticks_choke)
	{
		g_ctx.send_packet = g_ctx.globals.force_send_packet;
		--g_ctx.globals.ticks_choke;
	}

	auto& correct = g_ctx.globals.data.emplace_front();

	correct.command_number = m_pcmd->m_command_number;
	correct.choked_commands = m_clientstate()->iChokedCommands + 1;
	correct.tickcount = m_globals()->m_tickcount;

	if (g_ctx.send_packet)
		g_ctx.globals.choked_number.clear();
	else
		g_ctx.globals.choked_number.emplace_back(correct.command_number);

	while (g_ctx.globals.data.size() > (int)(2.0f / m_globals()->m_intervalpertick))
		g_ctx.globals.data.pop_back();

	auto& out = g_ctx.globals.commands.emplace_back();

	out.is_outgoing = g_ctx.send_packet;
	out.is_used = false;
	out.command_number = m_pcmd->m_command_number;
	out.previous_command_number = 0;

	while (g_ctx.globals.commands.size() > (int)(1.0f / m_globals()->m_intervalpertick))
		g_ctx.globals.commands.pop_front();

	if (!g_ctx.send_packet && !m_gamerules()->m_bIsValveDS())//
	{
		auto net_channel = m_clientstate()->pNetChannel;

		if (net_channel->m_nChokedPackets > 0 && !(net_channel->m_nChokedPackets % 4))
		{
			auto backup_choke = net_channel->m_nChokedPackets;
			net_channel->m_nChokedPackets = 0;

			net_channel->send_datagram();
			--net_channel->m_nOutSequenceNr;

			net_channel->m_nChokedPackets = backup_choke;
		}
	}

	/* Local animations, recoded */
	{
		local_animations::get().command = m_pcmd;
		if (g_ctx.send_packet && !g_ctx.globals.should_send_packet && (!g_ctx.globals.should_choke_packet || (!exploit::get().hide_shots_enabled && !g_ctx.globals.double_tap_fire)))
		{
			local_animations::get().local_data.fake_angles = m_pcmd->m_viewangles;
			local_animations::get().local_data.real_angles = local_animations::get().local_data.stored_real_angles;
		}

		if (!antiaim::get().breaking_lby)
			local_animations::get().local_data.stored_real_angles = m_pcmd->m_viewangles;

		const auto bk = g_ctx.local()->m_flThirdpersonRecoil();

		const auto movestate = g_ctx.local()->m_iMoveState();
		const auto iswalking = g_ctx.local()->m_bIsWalking();

		g_ctx.local()->m_iMoveState() = 0;
		g_ctx.local()->m_bIsWalking() = false;

		auto m_forward = m_pcmd->m_buttons & IN_FORWARD;
		auto m_back = m_pcmd->m_buttons & IN_BACK;
		auto m_right = m_pcmd->m_buttons & IN_MOVERIGHT;
		auto m_left = m_pcmd->m_buttons & IN_MOVELEFT;
		auto m_walking = m_pcmd->m_buttons & IN_SPEED;

		bool m_walk_state = m_walking ? true : false;

		if (m_pcmd->m_buttons & IN_DUCK || g_ctx.local()->m_bDucking() || g_ctx.local()->m_fFlags() & FL_DUCKING)
			m_walk_state = false;
		else if (m_walking)
		{
			float m_max_speed = g_ctx.local()->m_flMaxSpeed() * 0.52f;

			if (m_max_speed + 25.f > g_ctx.local()->m_vecVelocity().Length())
				g_ctx.local()->m_bIsWalking() = true;
		}

		auto move_buttons_pressed = m_pcmd->m_buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT | IN_RUN);

		bool holding_forward_and_back;
		bool holding_right_and_left;

		if (!m_forward)
			holding_forward_and_back = false;
		else
			holding_forward_and_back = m_back;

		if (!m_right)
			holding_right_and_left = false;
		else
			holding_right_and_left = m_left;

		if (move_buttons_pressed)
		{
			if (holding_forward_and_back)
			{
				if (holding_right_and_left)
					g_ctx.local()->m_iMoveState() = 0;
				else if (m_right || m_left)
					g_ctx.local()->m_iMoveState() = 2;
				else
					g_ctx.local()->m_iMoveState() = 0;
			}
			else
			{
				if (holding_forward_and_back)
					g_ctx.local()->m_iMoveState() = 0;
				else if (m_back || m_forward)
					g_ctx.local()->m_iMoveState() = 2;
				else
					g_ctx.local()->m_iMoveState() = 0;
			}
		}

		if (g_ctx.local()->m_iMoveState() == 2 && m_walk_state)
			g_ctx.local()->m_iMoveState() = 1;

		g_ctx.local()->m_iMoveState() = movestate;
		g_ctx.local()->m_bIsWalking() = iswalking;
		g_ctx.local()->m_flThirdpersonRecoil() = bk;
	}

	if (g_ctx.send_packet && g_ctx.globals.should_send_packet)
		g_ctx.globals.should_send_packet = false;


	g_ctx.globals.in_createmove = false;
	send_packet = g_ctx.send_packet;

	verified->m_cmd = *m_pcmd;
	verified->m_crc = m_pcmd->GetChecksum();
}

__declspec(naked) void __stdcall hooks::hkCreatemove_Proxy(int sequence_number, float input_sample_frametime, bool active)
{
	__asm
	{
		push ebx
		push esp
		push dword ptr[esp + 20]
		push dword ptr[esp + 0Ch + 8]
		push dword ptr[esp + 10h + 4]
		call hkCreateMove
		pop ebx
		retn 0Ch
	}
}