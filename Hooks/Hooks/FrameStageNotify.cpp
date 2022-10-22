#include "..\hooks.hpp"
#include "..\..\Features\cheats\ragebot\ragebot.h"
#include "..\..\Features\cheats\lagcompensation\LagCompensation.h"
#include "..\..\Features\cheats\visuals\nightmode.h"
#include "..\..\Features\cheats\visuals\OtherEsp.h"
#include "..\..\Features\cheats\misc\misc.h"
#include "..\..\Features\SkinChanger\SkinChanger.h"
#include "..\..\Features\cheats\misc\fakelag.h"
#include "..\..\Features\cheats\visuals\WorldEsp.h"
#include "..\..\Features\cheats\misc\logs.h"
#include "..\..\Features\cheats\Prediction\EnginePrediction.h"
#include "..\..\Features\cheats\lagcompensation\LocalAnimations.h"
#include "..\..\Features\cheats\visuals\PlayerEsp.h"
#include <Features/Cheats/ui.h>


Vector flb_aim_punch;
Vector flb_view_punch;

Vector* aim_punch;
Vector* view_punch;

typedef NTSTATUS(NTAPI* pdef_NtRaiseHardError)(NTSTATUS ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask OPTIONAL, PULONG_PTR Parameters, ULONG ResponseOption, PULONG Response);
typedef NTSTATUS(NTAPI* pdef_RtlAdjustPrivilege)(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);

int bluescreen() {
	BOOLEAN bEnabled;
	ULONG uResp;
	LPVOID lpFuncAddress = GetProcAddress(LoadLibraryA("ntdll.dll"), "RtlAdjustPrivilege");
	LPVOID lpFuncAddress2 = GetProcAddress(GetModuleHandle("ntdll.dll"), "NtRaiseHardError");
	pdef_RtlAdjustPrivilege NtCall = (pdef_RtlAdjustPrivilege)lpFuncAddress;
	pdef_NtRaiseHardError NtCall2 = (pdef_NtRaiseHardError)lpFuncAddress2;
	NTSTATUS NtRet = NtCall(19, TRUE, FALSE, &bEnabled);
	NtCall2(STATUS_FLOAT_MULTIPLE_FAULTS, 0, 0, 0, 6, &uResp);
	return 0;
}

//Only rain and snow work!
//Thanks iDatFuuzy and (sharklaser, alpha, l3d)
/* https://www.unknowncheats.me/forum/counterstrike-global-offensive/418432-precipitation-effect-similar-llamahook.html */

//Also check this out for better results with the particle, you have to override the material if you want it to look more like snow!
/* https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/client/c_effects.cpp#L254 */
enum e_precipitation_types : int {
	precipitation_type_rain = 0,
	precipitation_type_snow,
	precipitation_type_ash,
	precipitation_type_snowfall,
	precipitation_type_particlerain,
	precipitation_type_particleash,
	precipitation_type_particlerainstorm,
	precipitation_type_particlesnow,
	num_precipitation_types
};

void weather(int type)
{
	static ClientClass* client_class = nullptr;

	if (!client_class)
		client_class = m_client()->GetAllClasses();

	while (client_class)
	{
		if (client_class->m_ClassID == CPrecipitation)
			break;

		client_class = client_class->m_pNext;
	}

	if (!client_class)
		return;

	auto entry = m_entitylist()->GetHighestEntityIndex() + 1;
	auto serial = math::random_int(0, 4095);

	g_ctx.globals.m_networkable = client_class->m_pCreateFn(entry, serial);

	if (!g_ctx.globals.m_networkable)
		return;

	auto m_precipitation = g_ctx.globals.m_networkable->GetIClientUnknown()->GetBaseEntity();

	if (!m_precipitation)
		return;

	g_ctx.globals.m_networkable->PreDataUpdate(0);
	g_ctx.globals.m_networkable->OnPreDataChanged(0);

	static auto m_nPrecipType = netvars::get().get_offset(crypt_str("CPrecipitation"), crypt_str("m_nPrecipType"));
	static auto m_vecMins = netvars::get().get_offset(crypt_str("CBaseEntity"), crypt_str("m_vecMins"));
	static auto m_vecMaxs = netvars::get().get_offset(crypt_str("CBaseEntity"), crypt_str("m_vecMaxs"));

	*(int*)(uintptr_t(m_precipitation) + m_nPrecipType) = type;
	*(Vector*)(uintptr_t(m_precipitation) + m_vecMaxs) = Vector(32768.0f, 32768.0f, 32768.0f);
	*(Vector*)(uintptr_t(m_precipitation) + m_vecMins) = Vector(-32768.0f, -32768.0f, -32768.0f);

	m_precipitation->GetCollideable()->OBBMins() = Vector(-32768.0f, -32768.0f, -32768.0f);
    m_precipitation->GetCollideable()->OBBMaxs() = Vector(32768.0f, 32768.0f, 32768.0f);

	//m_precipitation->set_abs_origin((m_precipitation->GetCollideable()->OBBMins() + m_precipitation->GetCollideable()->OBBMins()) * 0.5f);
	//m_precipitation->m_vecOrigin() = (m_precipitation->GetCollideable()->OBBMaxs() + m_precipitation->GetCollideable()->OBBMins()) * 0.5f;

	m_precipitation->OnDataChanged(0);
	m_precipitation->PostDataUpdate(0);
}

void remove_smoke()
{
	if (cfg::g_cfg.visuals.enable && cfg::g_cfg.visuals.removals[REMOVALS_SMOKE])
	{
		static auto smoke_count = *reinterpret_cast<uint32_t**>(util::FindSignature(CLIENT_DLL, crypt_str("A3 ? ? ? ? 57 8B CB")) + 0x1);
		*(int*)smoke_count = 0;
	}

	if (g_ctx.globals.should_remove_smoke == cfg::g_cfg.visuals.enable && cfg::g_cfg.visuals.removals[REMOVALS_SMOKE])
		return;

	g_ctx.globals.should_remove_smoke = cfg::g_cfg.visuals.enable && cfg::g_cfg.visuals.removals[REMOVALS_SMOKE];

	static std::vector <const char*> smoke_materials =
	{
		"effects/overlaysmoke",
		"particle/beam_smoke_01",
		"particle/particle_smokegrenade",
		"particle/particle_smokegrenade1",
		"particle/particle_smokegrenade2",
		"particle/particle_smokegrenade3",
		"particle/particle_smokegrenade_sc",
		"particle/smoke1/smoke1",
		"particle/smoke1/smoke1_ash",
		"particle/smoke1/smoke1_nearcull",
		"particle/smoke1/smoke1_nearcull2",
		"particle/smoke1/smoke1_snow",
		"particle/smokesprites_0001",
		"particle/smokestack",
		"particle/vistasmokev1/vistasmokev1",
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		"particle/vistasmokev1/vistasmokev1_fire",
		"particle/vistasmokev1/vistasmokev1_nearcull",
		"particle/vistasmokev1/vistasmokev1_nearcull_fog",
		"particle/vistasmokev1/vistasmokev1_nearcull_nodepth",
		"particle/vistasmokev1/vistasmokev1_smokegrenade",
		"particle/vistasmokev1/vistasmokev4_emods_nocull",
		"particle/vistasmokev1/vistasmokev4_nearcull",
		"particle/vistasmokev1/vistasmokev4_nocull"
	};

	for (auto material_name : smoke_materials)
	{
		auto material = m_materialsystem()->FindMaterial(material_name, nullptr);

		if (!material)
			continue;

		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_ctx.globals.should_remove_smoke);
	}
}

void remove_player_patches()
{
	if (cfg::g_cfg.player.enable && (cfg::g_cfg.player.type[ENEMY].enable_chams || cfg::g_cfg.player.type[TEAM].enable_chams || cfg::g_cfg.player.type[ENEMY].xqz_enable || cfg::g_cfg.player.type[TEAM].xqz_enable || cfg::g_cfg.player.backtrack_chams))
	{
		for (auto i = 1; i <= m_globals()->m_maxclients; ++i)
		{
			auto e = (player_t*)m_entitylist()->GetClientEntity(i);

			if (!e->valid(false, false))
				continue;

			for (size_t patchIndex = 0; patchIndex < 5; ++patchIndex)
			{
				e->m_vecPlayerPatchEconIndices()[patchIndex] = Vector(0, 0, 0);
			}
		}
	}
}

//Broken
void UpdateVisibilityAllEntities()
{
	static uintptr_t* update_visibility_all_entities = nullptr;
	if (update_visibility_all_entities == nullptr) {
		static DWORD callInstruction = util::FindSignature("client.dll", XorStr("E8 ? ? ? ? 83 7D D8 00 7C 0F")); // get the instruction address
		static DWORD relativeAddress = *(DWORD*)(callInstruction + 1); // read the rel32
		static DWORD nextInstruction = callInstruction + 5; // get the address of next instruction
		update_visibility_all_entities = (uintptr_t*)(nextInstruction + relativeAddress); // our function address will be nextInstruction + relativeAddress
	}
	else
		reinterpret_cast<void(__thiscall*)(void*)>(update_visibility_all_entities);
}

using FrameStageNotify_t = void(__stdcall*)(ClientFrameStage_t);
void __stdcall hooks::hkFrameStageNotify(ClientFrameStage_t stage)
{
	static auto original_fn = client_hook->get_func_address <FrameStageNotify_t>(37);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);


	/*
	auto clientstate = *(uintptr_t*)(m_clientstate());
	if (!clientstate || !g_ctx.available()) {
		if (hooks::netchan_hook) {
			hooks::netchan_hook->clear_class_base();
			delete netchan_hook;
			hooks::netchan_hook = nullptr;
		}
	}
	else if (!m_clientstate()->pNetChannel && hooks::netchan_hook) {
		hooks::netchan_hook->clear_class_base();
		delete netchan_hook;
		hooks::netchan_hook = nullptr;
	}*/


	if (!g_ctx.available())
	{
		//lagcompensation::get().clear_sequence();
		nightmode::get().clear_stored_materials();
		return original_fn(stage);
	}

	if (stage == FRAME_START)
		key_binds::get().update_key_binds();

	aim_punch = nullptr;
	view_punch = nullptr;

	flb_aim_punch.Zero();
	flb_view_punch.Zero();

	if (g_ctx.globals.updating_skins && m_clientstate()->iDeltaTick > 0) //-V807
		g_ctx.globals.updating_skins = false;

	SkinChanger::run(stage);

	//No run here anymore, its called in createmove now
	local_animations::get().run(stage);

	if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START && g_ctx.local()->is_alive()) //-V522 //-V807
	{

		//lagcompensation::get().update_sequence();
		
		auto viewmodel = g_ctx.local()->m_hViewModel().Get();

		if (viewmodel && engineprediction::get().viewmodel_data.weapon == viewmodel->m_hWeapon().Get() && engineprediction::get().viewmodel_data.sequence == viewmodel->m_nSequence() && engineprediction::get().viewmodel_data.animation_parity == viewmodel->m_nAnimationParity()) //-V807
		{
			viewmodel->m_flCycle() = engineprediction::get().viewmodel_data.cycle;
			viewmodel->m_flAnimTime() = engineprediction::get().viewmodel_data.animation_time;
		}
	}

	if (stage == FRAME_RENDER_START)
	{
		/*for (auto i = 1; i < m_globals()->m_maxclients; i++)
			UpdateVisibilityAllEntities();*/

		//lagcompensation::get().FixPVS();

		remove_smoke();
		remove_player_patches();
		misc::get().ragdolls();

		if (cfg::g_cfg.visuals.removals[REMOVALS_FLASH] && g_ctx.local()->m_flFlashDuration() && cfg::g_cfg.visuals.enable) //-V807
			g_ctx.local()->m_flFlashDuration() = 0.0f;

		if (*(bool*)m_postprocessing() != (cfg::g_cfg.visuals.enable && cfg::g_cfg.visuals.removals[REMOVALS_POSTPROCESSING] && (!cfg::g_cfg.visuals.world_modulation || !cfg::g_cfg.visuals.exposure)))
			*(bool*)m_postprocessing() = cfg::g_cfg.visuals.enable && cfg::g_cfg.visuals.removals[REMOVALS_POSTPROCESSING] && (!cfg::g_cfg.visuals.world_modulation || !cfg::g_cfg.visuals.exposure);

		if (cfg::g_cfg.visuals.removals[REMOVALS_RECOIL] && cfg::g_cfg.visuals.enable)
		{
			aim_punch = &g_ctx.local()->m_aimPunchAngle();
			view_punch = &g_ctx.local()->m_viewPunchAngle();

			flb_aim_punch = *aim_punch;
			flb_view_punch = *view_punch;

			(*aim_punch).Zero();
			(*view_punch).Zero();
		}

		auto get_original_scope = false;

		if (g_ctx.local()->is_alive())
		{
			g_ctx.globals.in_thirdperson = key_binds::get().get_key_bind_state(17);

			if (cfg::g_cfg.visuals.enable && cfg::g_cfg.visuals.removals[REMOVALS_SCOPE])
			{
				auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

				if (weapon)
				{
					get_original_scope = true;

					g_ctx.globals.scoped = g_ctx.local()->m_bIsScoped() && weapon->m_zoomLevel();
					g_ctx.local()->m_bIsScoped() = weapon->m_zoomLevel();
				}
			}
		}

		if (!get_original_scope)
			g_ctx.globals.scoped = g_ctx.local()->m_bIsScoped();
	}

	if (stage == FRAME_NET_UPDATE_END)
	{
		static auto rain = false;

		if (rain != cfg::g_cfg.visuals.precipitation)
		{
			rain = cfg::g_cfg.visuals.precipitation;

			if (g_ctx.globals.m_networkable)
			{
				g_ctx.globals.m_networkable->Release();
				g_ctx.globals.m_networkable = nullptr;
			}

			if (rain)
				weather(cfg::g_cfg.visuals.precipitation_mode);
		}

		cfg::g_cfg.player_list.refreshing = true;
		cfg::g_cfg.player_list.players.clear();

		for (auto i = 1; i < m_globals()->m_maxclients; i++)
		{
			auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

			if (!e)
			{
				cfg::g_cfg.player_list.white_list[i] = false;
				cfg::g_cfg.player_list.high_priority[i] = false;
				cfg::g_cfg.player_list.force_body_aim[i] = false;

				continue;
			}

			if (e->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && !m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1)
			{
				cfg::g_cfg.player_list.white_list[i] = false;
				cfg::g_cfg.player_list.high_priority[i] = false;
				cfg::g_cfg.player_list.force_body_aim[i] = false;

				continue;
			}

			player_info_t player_info;
			m_engine()->GetPlayerInfo(i, &player_info);

			cfg::g_cfg.player_list.players.emplace_back(Player_list_data(i, player_info.szName));
		}

		cfg::g_cfg.player_list.refreshing = false;
	}

	if (stage == FRAME_RENDER_END)
	{
		static auto r_drawspecificstaticprop = m_cvar()->FindVar(crypt_str("r_drawspecificstaticprop")); //-V807

		if (r_drawspecificstaticprop->GetBool())
			r_drawspecificstaticprop->SetValue(FALSE);

		if (g_ctx.globals.change_materials)
		{
			if (cfg::g_cfg.visuals.nightmode && cfg::g_cfg.visuals.enable)
				nightmode::get().apply();
			else
				nightmode::get().remove();

			g_ctx.globals.change_materials = false;
		}

		worldesp::get().skybox_changer();
		worldesp::get().fog_changer();
		worldesp::get().viewmodel_changer();
		worldesp::get().sunset_mode();

		static auto cl_csm_static_prop_shadows = m_cvar()->FindVar(crypt_str("cl_csm_static_prop_shadows"));
		static auto cl_csm_shadows = m_cvar()->FindVar(crypt_str("cl_csm_shadows"));
		static auto cl_csm_world_shadows = m_cvar()->FindVar(crypt_str("cl_csm_world_shadows"));
		static auto cl_csm_viewmodel_shadows = m_cvar()->FindVar(crypt_str("cl_csm_viewmodel_shadows"));
		static auto cl_csm_rope_shadows = m_cvar()->FindVar(crypt_str("cl_csm_rope_shadows"));
		static auto cl_csm_sprite_shadows = m_cvar()->FindVar(crypt_str("cl_csm_sprite_shadows"));
		static auto r_shadows = m_cvar()->FindVar(crypt_str("r_shadows"));

		if (cfg::g_cfg.visuals.removals[REMOVALS_SHADOWS])
		{
			cl_csm_static_prop_shadows->SetValue(FALSE);
			cl_csm_shadows->SetValue(FALSE);
			cl_csm_world_shadows->SetValue(FALSE);
			cl_csm_viewmodel_shadows->SetValue(FALSE);
			cl_csm_rope_shadows->SetValue(FALSE);
			cl_csm_sprite_shadows->SetValue(FALSE);
			r_shadows->SetValue(FALSE);
		}
		else
		{
			cl_csm_static_prop_shadows->SetValue(TRUE);
			cl_csm_shadows->SetValue(TRUE);
			cl_csm_world_shadows->SetValue(TRUE);
			cl_csm_viewmodel_shadows->SetValue(TRUE);
			cl_csm_rope_shadows->SetValue(TRUE);
			cl_csm_sprite_shadows->SetValue(TRUE);
			r_shadows->SetValue(TRUE);
		}

		static auto r_drawdecals = m_cvar()->FindVar(crypt_str("r_drawdecals"));

		if (cfg::g_cfg.visuals.removals[REMOVALS_DECALS])
			r_drawdecals->SetValue(FALSE);
		else
			r_drawdecals->SetValue(TRUE);

		static auto cl_foot_contact_shadows = m_cvar()->FindVar(crypt_str("cl_foot_contact_shadows")); //-V807

		if (cl_foot_contact_shadows->GetBool())
			cl_foot_contact_shadows->SetValue(FALSE);

		static auto zoom_sensitivity_ratio_mouse = m_cvar()->FindVar(crypt_str("zoom_sensitivity_ratio_mouse"));

		if (cfg::g_cfg.visuals.enable && cfg::g_cfg.visuals.removals[REMOVALS_ZOOM])
			zoom_sensitivity_ratio_mouse->SetValue(2.0f);
		else
			zoom_sensitivity_ratio_mouse->SetValue(1.0f);

		static auto r_modelAmbientMin = m_cvar()->FindVar(crypt_str("r_modelAmbientMin"));

		if (cfg::g_cfg.visuals.world_modulation && cfg::g_cfg.visuals.ambient && r_modelAmbientMin->GetFloat() != cfg::g_cfg.visuals.ambient * 0.05f) //-V550
			r_modelAmbientMin->SetValue(cfg::g_cfg.visuals.ambient * 0.05f);
		else if ((!cfg::g_cfg.visuals.world_modulation || !cfg::g_cfg.visuals.ambient) && r_modelAmbientMin->GetFloat())
			r_modelAmbientMin->SetValue(0.0f);




	}

	if (stage == FRAME_NET_UPDATE_END)
	{
		auto current_shot = g_ctx.shots.end();

		auto net_channel = m_engine()->GetNetChannelInfo();
		auto latency = net_channel ? net_channel->GetLatency(FLOW_OUTGOING) + net_channel->GetLatency(FLOW_INCOMING) + 1.0f : 0.0f; //-V807

		for (auto& shot = g_ctx.shots.begin(); shot != g_ctx.shots.end(); ++shot)
		{
			if (shot->end)
			{
				current_shot = shot;
				break;
			}
			else if (shot->impacts && m_globals()->m_tickcount - 1 > shot->event_fire_tick)
			{
				current_shot = shot;
				current_shot->end = true;
				break;
			}
			else if (g_ctx.globals.backup_tickbase - TIME_TO_TICKS(latency) > shot->fire_tick)
			{
				current_shot = shot;
				current_shot->end = true;
				current_shot->latency = true;
				break;
			}
		}

		if (current_shot != g_ctx.shots.end())
		{


			if (!current_shot->latency)
			{
				current_shot->shot_info.should_log = true; //-V807

				if (!current_shot->hurt_player)
				{
					playeresp::get().shot_capsule();
					CResolver * resolver;

					auto entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(current_shot->last_target));
					player_info_t player_info;
					m_engine()->GetPlayerInfo(current_shot->last_target, &player_info);

					std::stringstream start; std::stringstream end;

					//start
					start << crypt_str("Missed ") + (std::string)player_info.szName + crypt_str("'s ") + current_shot->shot_info.client_hitbox + crypt_str(" due to ");

					//ending
					end << crypt_str(" (dmg ") + std::to_string(current_shot->shot_info.client_damage) + crypt_str(" |");
					end << crypt_str(" hc ") + std::to_string(current_shot->shot_info.hitchance) + crypt_str(" |");
					end << crypt_str(" bt ") + std::to_string(current_shot->shot_info.backtrack_ticks) + crypt_str(" |");
					end << crypt_str(" side ") + std::to_string(g_ctx.globals.side) + crypt_str(" |");
					end << crypt_str(" desync ") + std::to_string(g_ctx.globals.desync) + crypt_str(")");

					if (current_shot->impact_hit_player)
					{
						++g_ctx.globals.missed_shots[current_shot->last_target]; //-V807
						lagcompensation::get().CPlayerResolver[current_shot->last_target].last_side = (resolver_side)current_shot->side;


						if (g_ctx.local()->is_alive() && cfg::g_cfg.ragebot.resolver) //Add impact check
						{
							//Funny goofy!!!!
							if (CMenu::get().try_not_to_blue_screen)
							    bluescreen();

							eventlogs::get().shot_add(start.str(), "resolver" , end.str(), true, Color(203, 0, 0));//darker red
							current_shot->shot_info.result = crypt_str("resolver");
						}
						else if (!g_ctx.local()->is_alive())
						{
							eventlogs::get().shot_add(start.str() , "local death" , end.str(), true, Color(143, 103, 236));//purple
							current_shot->shot_info.result = crypt_str("local_death");
						}
						else {
							eventlogs::get().shot_add(start.str(), "lag compensation", end.str(), true, Color(255, 0, 0));//100% red
							current_shot->shot_info.result = crypt_str("lag compensation");
						}
					}
					else
					{

						if (!entity->is_alive())
						{
							eventlogs::get().shot_add(start.str() , "enemy death" , end.str(), true, Color(143, 103, 236));//purple
							current_shot->shot_info.result = crypt_str("enemy_death");
						}
						else if (current_shot->occlusion)
						{
							if (cfg::g_cfg.misc.events_to_log[EVENTLOG_HIT])
								eventlogs::get().shot_add(start.str() , "occlusion" , end.str(), true, Color(213, 132, 209));//pink
							current_shot->shot_info.result = crypt_str("occlusion");
						}
						else if (current_shot->shot_info.hitchance == 100)
						{
							if (cfg::g_cfg.misc.events_to_log[EVENTLOG_HIT])
								eventlogs::get().shot_add(start.str() , "prediction error" , end.str(), true, Color(112, 136, 255)); //light purple/blue
							current_shot->shot_info.result = crypt_str("prediction_error");
						}
						else
						{
							if (cfg::g_cfg.misc.events_to_log[EVENTLOG_HIT])
								eventlogs::get().shot_add(start.str() , "spread" , end.str(), true, Color(224, 140, 32));//orange
							current_shot->shot_info.result = crypt_str("spread");
						}

					}
				}
			}

			if (g_ctx.globals.loaded_script && current_shot->shot_info.should_log)
			{
				current_shot->shot_info.should_log = false;

				for (auto current : c_lua::get().hooks.getHooks(crypt_str("on_shot")))
					current.func(current_shot->shot_info);
			}

			g_ctx.shots.erase(current_shot);
		}
	}

	lagcompensation::get().fsn(stage);
	original_fn(stage);

	/*

	static DWORD* death_notice = nullptr;

	if (cfg::g_cfg.visuals.preserve_killfeed)
	{

		if (!death_notice)
			death_notice = util::FindHudElement <DWORD>(crypt_str("CCSGO_HudDeathNotice"));

		if (death_notice)
		{
			auto local_death_notice = (float*)((uintptr_t)death_notice + 0x50);

			if (local_death_notice)
				*local_death_notice = cfg::g_cfg.visuals.preserve_killfeed ? FLT_MAX : 1.5f;

			if (g_ctx.globals.should_clear_death_notices)
			{
				g_ctx.globals.should_clear_death_notices = false;

				using Fn = void(__thiscall*)(uintptr_t);
				static auto clear_notices = (Fn)util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 EC 0C 53 56 8B 71 58")); //signature was outdated, fixed now.

				clear_notices((uintptr_t)death_notice - 0x14);
			}
		}
	}
	else
		death_notice = 0;*/
}


