// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\Features\cheats\ui.h"
#include "..\..\Features\cheats\lagcompensation\LagCompensation.h"
#include "..\..\Features\cheats\visuals\PlayerEsp.h"
#include "..\..\Features\cheats\visuals\OtherEsp.h"
#include "..\..\Features\cheats\misc\logs.h"
#include "..\..\Features\cheats\visuals\WorldEsp.h"
#include "..\..\Features\cheats\misc\misc.h"
#include "..\..\Features\cheats\visuals\GrenadePrediction.h"
#include "..\..\Features\cheats\visuals\DormantEsp.h"
#include "..\..\Features\cheats\lagcompensation\LocalAnimations.h"
#include "..\..\Features\cheats\exploits\TickBase.h"
#include "..\..\Features\cheats\visuals\nightmode.h"
#include "..\..\Features\utils\render.h"

using PaintTraverse_t = void(__thiscall*)(void*, vgui::VPANEL, bool, bool);


void __fastcall hooks::hkPaintTraverse(void* ecx, void* edx, vgui::VPANEL panel, bool force_repaint, bool allow_force)
{
	static auto original_fn = panel_hook->get_func_address <PaintTraverse_t> (41);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true); //-V807

	if (!m_engine()->IsInGame() || !m_engine()->IsConnected())
		LoadPlayerMdlOnce = false;

	static auto set_console = true;

	if (set_console)
	{
		set_console = false;

		m_cvar()->FindVar(crypt_str("developer"))->SetValue(FALSE); //-V807
		m_cvar()->FindVar(crypt_str("con_filter_enable"))->SetValue(TRUE);
		m_cvar()->FindVar(crypt_str("con_filter_text"))->SetValue(crypt_str(""));
		m_engine()->ExecuteClientCmd(crypt_str("clear"));

		m_cvar()->ConsoleColorPrintf(Color(0, 153, 255), "[Luna] Successfully injected!");
		m_cvar()->ConsolePrintf(crypt_str("\n"));
		//m_cvar()->ConsoleColorPrintf(Color::White, "Build: developer");
	}

	if (!cfg::g_cfg.menu.menu_color)
		cfg::g_cfg.menu.menu_color_col = Color(0, 153, 255);

	static auto log_value = true;

	if (log_value != cfg::g_cfg.misc.console_filter)
	{
		log_value = cfg::g_cfg.misc.console_filter;

		if (log_value)
			m_cvar()->FindVar(crypt_str("con_filter_text"))->SetValue(crypt_str("IrWL5106TZZKNFPz4P4Gl3pSN?J370f5hi373ZjPg%VOVh6lN"));
		else
			m_cvar()->FindVar(crypt_str("con_filter_text"))->SetValue(crypt_str(""));
	}

	static vgui::VPANEL panel_id = 0;
	static auto in_game = false;

	if (!in_game && m_engine()->IsInGame()) //-V807
	{
		in_game = true;

		for (auto i = 1; i < 65; i++)
		{
			g_ctx.globals.fired_shots[i] = 0;
			g_ctx.globals.missed_shots[i] = 0;
			player_records[i].clear();
			lagcompensation::get().is_dormant[i] = false;
			playeresp::get().esp_alpha_fade[i] = 0.0f;
			playeresp::get().health[i] = 100;
			c_dormant_esp::get().m_cSoundPlayers[i].reset();
			otheresp::get().damage_marker[i].reset();
		}

		antiaim::get().freeze_check = false;
		g_ctx.globals.next_lby_update = FLT_MIN;
		g_ctx.globals.last_lby_move = FLT_MIN;
		g_ctx.globals.last_aimbot_shot = 0;
		g_ctx.globals.bomb_timer_enable = true;
		g_ctx.globals.backup_model = false;
		g_ctx.globals.should_remove_smoke = false;
		g_ctx.globals.should_update_beam_index = true;
		g_ctx.globals.should_update_playerresource = true;
		g_ctx.globals.should_update_gamerules = true;
		g_ctx.globals.kills = 0;
		g_ctx.shots.clear();
		otheresp::get().hitmarker.hurt_time = FLT_MIN;
		otheresp::get().hitmarker.point = ZERO;
		g_ctx.globals.commands.clear();
		SkinChanger::model_indexes.clear();
		SkinChanger::player_model_indexes.clear();
	}
	else if (in_game && !m_engine()->IsInGame())
	{
		in_game = false;

		g_ctx.globals.m_networkable = nullptr;

		cfg::g_cfg.player_list.players.clear();

		exploit::get().double_tap_enabled = false;
		exploit::get().double_tap_key = false;
		exploit::get().hide_shots_enabled = false;
		exploit::get().hide_shots_key = false;
	}

	static uint32_t HudZoomPanel = 0;
	
	if (!HudZoomPanel)
		if (!strcmp(crypt_str("HudZoom"), m_panel()->GetName(panel)))
			HudZoomPanel = panel;

	if (HudZoomPanel == panel && cfg::g_cfg.visuals.enable && cfg::g_cfg.visuals.removals[REMOVALS_SCOPE])
		return;

	original_fn(ecx, panel, force_repaint, allow_force);

	if (!panel_id)
	{
		auto panelName = m_panel()->GetName(panel);

		if (!strcmp(panelName, crypt_str("MatSystemTopPanel")))
			panel_id = panel;
	}

	if (panel_id == panel)
	{
		if (g_ctx.available())
		{
			static auto alive = false;

			if (!alive && g_ctx.local()->is_alive())
			{
				alive = true;
				g_ctx.globals.should_clear_death_notices = true;
			}
			else if (alive && !g_ctx.local()->is_alive())
			{
				alive = false;

				for (auto i = 1; i < m_globals()->m_maxclients; i++)
				{
					g_ctx.globals.fired_shots[i] = 0;
					g_ctx.globals.missed_shots[i] = 0;
				}

				local_animations::get().local_data.prediction_animstate = nullptr;
				local_animations::get().local_data.animstate = nullptr;

				g_ctx.globals.weapon = nullptr;
				g_ctx.globals.should_choke_packet = false;
				g_ctx.globals.should_send_packet = false;
				g_ctx.globals.kills = 0;
				g_ctx.globals.should_buy = 3;
			}

			auto weapon_debug_spread_show = m_cvar()->FindVar(crypt_str("weapon_debug_spread_show"));
			weapon_debug_spread_show->SetValue(g_ctx.local()->is_alive() && cfg::g_cfg.visuals.forcecrosshair && !g_ctx.local()->m_bIsScoped() ? 3 : 0);
	
			auto recoil_cross = m_cvar()->FindVar(crypt_str("cl_crosshair_recoil"));
			recoil_cross->SetValue(!cfg::g_cfg.visuals.removals[REMOVALS_RECOIL] && cfg::g_cfg.visuals.recoil_crosshair ? 1 : 0);

		}
		renderer::get().scene_render();
	}
}