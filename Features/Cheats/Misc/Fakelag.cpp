// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\ragebot\ragebot.h"
#include "fakelag.h"
#include "misc.h"
#include "../Prediction/EnginePrediction.h"
#include "logs.h"
#include "..\exploits\TickBase.h"

void fakelag::Fakelag(CUserCmd* m_pcmd)
{
	if (cfg::g_cfg.antiaim.fakelag && !g_ctx.globals.exploits)
	{
		static auto force_choke = false;

		if (force_choke)
		{
			force_choke = false;
			g_ctx.send_packet = false;
			return;
		}

		if (g_ctx.local()->m_fFlags() & FL_ONGROUND && !(engineprediction::get().backup_data.flags & FL_ONGROUND))
		{
			force_choke = true;
			g_ctx.send_packet = false;
			return;
		}
	}

	static auto fluctuate_ticks = 0;
	static auto switch_ticks = false;
	static auto random_factor = min(rand() % 16 + 1, cfg::g_cfg.antiaim.triggers_fakelag_amount);

	auto choked = m_clientstate()->iChokedCommands; //-V807
	auto flags = engineprediction::get().backup_data.flags; //-V807
	auto velocity = engineprediction::get().backup_data.velocity.Length(); //-V807
	auto velocity2d = engineprediction::get().backup_data.velocity.Length2D();

	auto max_speed = 260.0f;
	auto weapon_info = g_ctx.globals.weapon->get_csweapon_info();

	if (weapon_info)
		max_speed = g_ctx.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;

	switch (cfg::g_cfg.antiaim.fakelag_type)
	{
	case 0:
		max_choke = cfg::g_cfg.antiaim.triggers_fakelag_amount;
		break;
	case 1:
		max_choke = random_factor;
		break;
	case 2:
		if (velocity2d >= 5.0f)
		{
			auto dynamic_factor = std::ceilf(64.0f / (velocity2d * m_globals()->m_intervalpertick));

			if (dynamic_factor > 16)
				dynamic_factor = cfg::g_cfg.antiaim.triggers_fakelag_amount;

			max_choke = dynamic_factor;
		}
		else
			max_choke = cfg::g_cfg.antiaim.triggers_fakelag_amount;
		break;
	case 3:
		max_choke = fluctuate_ticks;
		break;
	}

	if (m_gamerules()->m_bIsValveDS())
		max_choke = m_engine()->IsVoiceRecording() ? 1 : min(max_choke, 6);

	//if (g_ctx.globals.startcharge)
	//	max_choke = 14;

	if (g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND && !m_gamerules()->m_bIsValveDS() && key_binds::get().get_key_bind_state(20)) //-V807
	{
		max_choke = 14;

		if (choked < max_choke)
			g_ctx.send_packet = false;
		else
			g_ctx.send_packet = true;
	}
	else
	{
		if (cfg::g_cfg.ragebot.enable && g_ctx.globals.current_weapon != -1 && !g_ctx.globals.exploits && cfg::g_cfg.antiaim.fakelag && cfg::g_cfg.antiaim.fakelag_enablers[FAKELAG_PEEK] && cfg::g_cfg.antiaim.triggers_fakelag_amount > 6 && !started_peeking && velocity >= 5.0f)
		{
			auto predicted_eye_pos = g_ctx.globals.eye_pos + engineprediction::get().backup_data.velocity * m_globals()->m_intervalpertick * (float)cfg::g_cfg.antiaim.triggers_fakelag_amount * 0.5f;

			for (auto i = 1; i < m_globals()->m_maxclients; i++)
			{
				auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

				if (!e->valid(true))
					continue;

				auto records = &player_records[i]; //-V826

				if (records->empty())
					continue;

				auto record = &records->front();

				if (!record->valid())
					continue;

				scan_data predicted_data;
				Rbot::get().Scan(record, predicted_data, predicted_eye_pos);

				if (predicted_data.valid())
				{
					scan_data data;
					Rbot::get().Scan(record, data, g_ctx.globals.eye_pos);

					if (!data.valid())
					{
						random_factor = min(rand() % 16 + 1, cfg::g_cfg.antiaim.triggers_fakelag_amount);
						switch_ticks = !switch_ticks;
						fluctuate_ticks = switch_ticks ? cfg::g_cfg.antiaim.triggers_fakelag_amount : max(cfg::g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

						g_ctx.send_packet = true;
						started_peeking = true;

						return;
					}
				}
			}
		}

		if (!g_ctx.globals.exploits && cfg::g_cfg.antiaim.fakelag && cfg::g_cfg.antiaim.fakelag_enablers[FAKELAG_PEEK] && started_peeking)
		{
			if (choked < max_choke)
				g_ctx.send_packet = false;
			else
			{
				started_peeking = false;

				random_factor = min(rand() % 16 + 1, cfg::g_cfg.antiaim.triggers_fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? cfg::g_cfg.antiaim.triggers_fakelag_amount : max(cfg::g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

				g_ctx.send_packet = true;
			}
		}
		else if (!g_ctx.globals.exploits && cfg::g_cfg.antiaim.fakelag && velocity >= 5.0f && g_ctx.globals.slowwalking && cfg::g_cfg.antiaim.fakelag_enablers[FAKELAG_SLOW_WALK])
		{
			if (choked < max_choke)
				g_ctx.send_packet = false;
			else
			{
				started_peeking = false;

				random_factor = min(rand() % 16 + 1, cfg::g_cfg.antiaim.triggers_fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? cfg::g_cfg.antiaim.triggers_fakelag_amount : max(cfg::g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

				g_ctx.send_packet = true;
			}
		}
		else if (!g_ctx.globals.exploits && cfg::g_cfg.antiaim.fakelag && velocity >= 5.0f && !g_ctx.globals.slowwalking && g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND && cfg::g_cfg.antiaim.fakelag_enablers[FAKELAG_MOVE])
		{
			if (choked < max_choke)
				g_ctx.send_packet = false;
			else
			{
				started_peeking = false;

				random_factor = min(rand() % 16 + 1, cfg::g_cfg.antiaim.triggers_fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? cfg::g_cfg.antiaim.triggers_fakelag_amount : max(cfg::g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

				g_ctx.send_packet = true;
			}
		}
		else if (!g_ctx.globals.exploits && cfg::g_cfg.antiaim.fakelag && !g_ctx.globals.slowwalking && !(g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND) && cfg::g_cfg.antiaim.fakelag_enablers[FAKELAG_AIR])
		{
			if (choked < max_choke)
				g_ctx.send_packet = false;
			else
			{
				started_peeking = false;

				random_factor = min(rand() % 16 + 1, cfg::g_cfg.antiaim.triggers_fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? cfg::g_cfg.antiaim.triggers_fakelag_amount : max(cfg::g_cfg.antiaim.triggers_fakelag_amount - 2, 1);

				g_ctx.send_packet = true;
			}
		}
		else if (!g_ctx.globals.exploits && cfg::g_cfg.antiaim.fakelag)
		{
			max_choke = cfg::g_cfg.antiaim.fakelag_amount;

			if (m_gamerules()->m_bIsValveDS())
				max_choke = min(max_choke, 6);

			if (choked < max_choke)
				g_ctx.send_packet = false;
			else
			{
				started_peeking = false;

				random_factor = min(rand() % 16 + 1, cfg::g_cfg.antiaim.fakelag_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? cfg::g_cfg.antiaim.fakelag_amount : max(cfg::g_cfg.antiaim.fakelag_amount - 2, 1);

				g_ctx.send_packet = true;
			}
		}
		else if (g_ctx.globals.exploits || antiaim::get().CanAntiAim(m_pcmd, false)) //-V648
		{
			condition = true;
			started_peeking = false;

			if (choked < 1)
				g_ctx.send_packet = false;
			else
				g_ctx.send_packet = true;
		}
		else
			condition = true;
	}
}

void fakelag::Createmove()
{
	if (FakelagCondition(g_ctx.get_command()))
		return;

	Fakelag(g_ctx.get_command());

	if (!m_gamerules()->m_bIsValveDS() && m_clientstate()->iChokedCommands <= 16)
	{
		static auto Fn = util::FindSignature(ENGINE_DLL, crypt_str("B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC")) + 0x1;
		DWORD old = 0;

		VirtualProtect((void*)Fn, sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &old);
		*(uint32_t*)Fn = 17;
		VirtualProtect((void*)Fn, sizeof(uint32_t), old, &old);
	}
}

bool fakelag::FakelagCondition(CUserCmd* m_pcmd) {
	condition = false;

	if (g_ctx.local()->m_bGunGameImmunity() || g_ctx.local()->m_fFlags() & FL_FROZEN)
		condition = true;

	if (antiaim::get().freeze_check && !exploit::get().double_tap_enabled && !exploit::get().hide_shots_enabled)
		condition = true;

	if (exploit::get().uncharged_this_tick)
		condition = true;

	return condition;
}