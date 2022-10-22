// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "LocalAnimations.h"

__forceinline float approach(float target, float value, float speed)
{
	float delta = target - value;

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}

void local_animations::run(ClientFrameStage_t stage)
{
	if (m_clientstate()->iDeltaTick == -1) return;

	if (!fakelag::get().condition && key_binds::get().get_key_bind_state(20))
	{
		if (stage == FRAME_NET_UPDATE_END)
		{
			fake_server_update = false;

			if (g_ctx.local()->m_flSimulationTime() != fake_simulation_time) //-V550 //-V807
			{
				fake_server_update = true;
				fake_simulation_time = g_ctx.local()->m_flSimulationTime();
			}

			g_ctx.local()->get_animlayers()[3].m_flWeight = 0.0f;
			g_ctx.local()->get_animlayers()[3].m_flCycle = 0.0f;
			g_ctx.local()->get_animlayers()[12].m_flWeight = 0.0f;

			update_fake_animations();
		}
		else if (stage == FRAME_RENDER_START)
		{
			real_server_update = false;

			if (g_ctx.local()->m_flSimulationTime() != real_simulation_time) //-V550
			{
				real_server_update = true;
				real_simulation_time = g_ctx.local()->m_flSimulationTime();
			}

			g_ctx.local()->get_animlayers()[3].m_flWeight = 0.0f;
			g_ctx.local()->get_animlayers()[3].m_flCycle = 0.0f;
			g_ctx.local()->get_animlayers()[12].m_flWeight = 0.0f;

			update_local_animations();
		}
	}
	else if (stage == FRAME_RENDER_START)
	{
		real_server_update = false;
		fake_server_update = false;

		if (g_ctx.local()->m_flSimulationTime() != real_simulation_time || g_ctx.local()->m_flSimulationTime() != fake_simulation_time) //-V550
		{
			real_server_update = fake_server_update = true;
			real_simulation_time = fake_simulation_time = g_ctx.local()->m_flSimulationTime();
		}

		g_ctx.local()->get_animlayers()[3].m_flWeight = 0.0f;
		g_ctx.local()->get_animlayers()[3].m_flCycle = 0.0f;
		g_ctx.local()->get_animlayers()[12].m_flWeight = 0.0f;

		update_fake_animations();
		update_local_animations();
	}
}

void local_animations::update_fake_animations()
{
	auto alloc = !local_data.animstate;
	auto change = !alloc && handle != &g_ctx.local()->GetRefEHandle(); //-V807
	auto reset = !alloc && !change && g_ctx.local()->m_flSpawnTime() != spawntime; //-V550

	if (change)
		m_memalloc()->Free(local_data.animstate);

	if (reset)
	{
		util::reset_state(local_data.animstate);
		spawntime = g_ctx.local()->m_flSpawnTime();
	}

	if (alloc || change)
	{
		local_data.animstate = (c_baseplayeranimationstate*)m_memalloc()->Alloc(sizeof(c_baseplayeranimationstate));

		if (local_data.animstate)
			util::create_state(local_data.animstate, g_ctx.local());

		handle = (CBaseHandle*)&g_ctx.local()->GetRefEHandle();
		spawntime = g_ctx.local()->m_flSpawnTime();
	}

	if (!alloc && !change && !reset && fake_server_update)
	{
		float pose_parameter[24]; //-V688
		memcpy(pose_parameter, &g_ctx.local()->m_flPoseParameter(), 24 * sizeof(float));

		AnimationLayer layers[13]; //-V688
		memcpy(layers, g_ctx.local()->get_animlayers(), g_ctx.local()->animlayer_count() * sizeof(AnimationLayer));

		auto backup_frametime = m_globals()->m_frametime; //-V807
		auto backup_curtime = m_globals()->m_curtime;

		m_globals()->m_frametime = m_globals()->m_intervalpertick;
		m_globals()->m_curtime = g_ctx.local()->m_flSimulationTime();

		local_data.animstate->m_pBaseEntity = g_ctx.local();
		util::update_state(local_data.animstate, local_animations::get().local_data.fake_angles);

		local_data.animstate->m_bInHitGroundAnimation = false;
		local_data.animstate->m_fLandingDuckAdditiveSomething = 0.0f;
		local_data.animstate->m_flHeadHeightOrOffsetFromHittingGroundAnimation = 1.0f;

		g_ctx.local()->invalidate_bone_cache();

		g_ctx.local()->setup_bones_rebuilt(g_ctx.globals.fake_matrix, BONE_USED_BY_ANYTHING);

		for (auto& i : g_ctx.globals.fake_matrix)
		{
			i[0][3] -= g_ctx.local()->GetRenderOrigin().x; //-V807
			i[1][3] -= g_ctx.local()->GetRenderOrigin().y;
			i[2][3] -= g_ctx.local()->GetRenderOrigin().z;
		}

		m_globals()->m_frametime = backup_frametime;
		m_globals()->m_curtime = backup_curtime;

		memcpy(&g_ctx.local()->m_flPoseParameter(), pose_parameter, 24 * sizeof(float));
		memcpy(g_ctx.local()->get_animlayers(), layers, g_ctx.local()->animlayer_count() * sizeof(AnimationLayer));
	}
}

void local_animations::update_local_animations()
{
	auto animstate = g_ctx.local()->get_animation_state(); //-V807
	if (!animstate)
		return;

	if (tickcount != m_globals()->m_tickcount) //-V550
	{
		tickcount = m_globals()->m_tickcount;
		memcpy(layers, g_ctx.local()->get_animlayers(), g_ctx.local()->animlayer_count() * sizeof(AnimationLayer)); //-V807

		float flNewDuckAmount;
		if (local_data.animstate)
		{
			flNewDuckAmount = math::clamp(g_ctx.local()->m_flDuckAmount() + animstate->m_fLandingDuckAdditiveSomething, 0.0f, 1.0f);
			flNewDuckAmount = approach(flNewDuckAmount, animstate->m_fDuckAmount, animstate->m_flUpdateTimeDelta * 6.0f);
			flNewDuckAmount = math::clamp(flNewDuckAmount, 0.0f, 1.0f);
		}
		animstate->m_fDuckAmount = flNewDuckAmount;

		animstate->m_iLastClientSideAnimationUpdateFramecount = 0;
		util::update_state(animstate, local_animations::get().local_data.fake_angles);

		if (real_server_update)
		{
			abs_angles = animstate->m_flGoalFeetYaw;
			memcpy(pose_parameter, &g_ctx.local()->m_flPoseParameter(), 24 * sizeof(float));
		}
	}
	else
		animstate->m_iLastClientSideAnimationUpdateFramecount = m_globals()->m_framecount;


	animstate->m_flGoalFeetYaw = antiaim::get().CanAntiAim(g_ctx.get_command()) ? local_animations::get().local_data.real_angles.y : abs_angles;

	player_t* local = g_ctx.local();

	//local->force_bone_rebuild();
	memcpy(g_ctx.local()->get_animlayers(), layers, g_ctx.local()->animlayer_count() * sizeof(AnimationLayer));
	memcpy(&g_ctx.local()->m_flPoseParameter(), pose_parameter, 24 * sizeof(float));
	g_ctx.local()->set_abs_angles(Vector(0.0f, abs_angles, 0.0f));
	g_ctx.local()->set_abs_origin(g_ctx.local()->m_vecOrigin());
	g_ctx.local()->set_abs_velocity(g_ctx.local()->m_vecAbsVelocity());
	//local->setup_local_bones();

	if (g_ctx.local()->is_alive())
	{
		if (g_ctx.local()->m_fFlags() & FL_ONGROUND)
			animstate->m_bOnGround = true;
	}
	
	//Static legs in air
	g_ctx.local()->m_flPoseParameter()[6] = 1;
}