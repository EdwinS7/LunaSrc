// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "configs.h"
#include "base64.h"
#include "..\includes.hpp"
#include "..\utils\util.hpp"
#include <shlobj.h>

std::unordered_map <std::string, float[4]> colors;

C_ConfigManager* cfg_manager = new C_ConfigManager();
cfg::Config cfg::g_cfg;

item_setting* get_by_definition_index(const int definition_index)
{
	auto it = std::find_if(std::begin(cfg::g_cfg.skins.skinChanger), std::end(cfg::g_cfg.skins.skinChanger), [definition_index](const item_setting& e)
		{
			return e.itemId == definition_index;
		});

	return it == std::end(cfg::g_cfg.skins.skinChanger) ? nullptr : &*it;
}

//Default config
void C_ConfigManager::setup()
{
	//Legitbot
	{
		setup_item(&cfg::g_cfg.legitbot.enabled, false, ("Legitbot.enabled"));
		setup_item(&cfg::g_cfg.legitbot.friendly_fire, false, ("Legitbot.friendly_fire"));
		setup_item(&cfg::g_cfg.legitbot.autopistol, false, ("Legitbot.autopistol"));
		setup_item(&cfg::g_cfg.legitbot.disasblers, 5, ("Legitbot.disablers"));
		setup_item(&cfg::g_cfg.legitbot.autofire_delay, 0, ("Legitbot.autofire_delay"));
		setup_item(&cfg::g_cfg.legitbot.autofire_key, key_bind(HOLD), ("Legitbot.autofire_key"));
		setup_item(&cfg::g_cfg.legitbot.key, key_bind(HOLD), ("Legitbot.key"));
		setup_item(&cfg::g_cfg.legitbot.backtrack, false, ("Legitbot.backtrack"));
		setup_item(&cfg::g_cfg.legitbot.backtrack_ticks, 1, ("Legitbot.backtrack_ticks"));


		for (auto i = 0; i < 9; i++)
		{
			setup_item(&cfg::g_cfg.legitbot.weapon[i].priority, 0, std::to_string(i) + ("Legitbot.rcs_priority"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].auto_stop, false, std::to_string(i) + ("Legitbot.auto_stop"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].fov_type, 0, std::to_string(i) + ("Legitbot.fov_type"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].fov, 0.0f, std::to_string(i) + ("Legitbot.fov"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].smooth_type, 0, std::to_string(i) + ("Legitbot.smooth_type"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].smooth, 1.0f, std::to_string(i) + ("Legitbot.smooth"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].silent_fov, 0.0f, std::to_string(i) + ("Legitbot.silent_fov"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].rcs_type, 0, std::to_string(i) + ("Legitbot.rcs_type"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].rcs, 0.0f, std::to_string(i) + ("Legitbot.rcs"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].custom_rcs_smooth, 0.0f, std::to_string(i) + ("Legitbot.custom_rcs_smooth"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].custom_rcs_fov, 0.0f, std::to_string(i) + ("Legitbot.custom_rcs_fov"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].awall_dmg, 0, std::to_string(i) + ("Legitbot.awall_dmg"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].target_switch_delay, 0.0f, std::to_string(i) + ("Legitbot.target_switch_delay"));
			setup_item(&cfg::g_cfg.legitbot.weapon[i].autofire_hitchance, 0, std::to_string(i) + ("Legitbot.autofire_hitchance"));
		}
	}

	//Ragebot
	{
		setup_item(&cfg::g_cfg.ragebot.enable, false, ("Ragebot.enable"));
		setup_item(&cfg::g_cfg.ragebot.extrapolation, false, ("Ragebot.extrapolation"));
		setup_item(&cfg::g_cfg.ragebot.prediction, false, ("Ragebot.prediction"));
		setup_item(&cfg::g_cfg.ragebot.prediction_ticks, 1, ("Ragebot.hitchance_amount"));
		setup_item(&cfg::g_cfg.ragebot.accurate_fd, true, ("Ragebot.accurate_fakeduck"));
		setup_item(&cfg::g_cfg.ragebot.dormant_aimbot, false, ("Ragebot.dormant_aimbot"));
		setup_item(&cfg::g_cfg.ragebot.zeus_bot, true, ("Ragebot.zeus_bot"));
		setup_item(&cfg::g_cfg.ragebot.zeus_bot_hitchance, 55, ("Ragebot.zeus_bot_hitchance"));
		setup_item(&cfg::g_cfg.ragebot.knife_bot, true, ("Ragebot.knife_bot"));
		setup_item(&cfg::g_cfg.ragebot.autoshoot, false, ("Ragebot.autoshoot"));
		setup_item(&cfg::g_cfg.ragebot.double_tap, false, ("Ragebot.double_tap"));
		setup_item(&cfg::g_cfg.ragebot.double_tap_key, key_bind(TOGGLE), ("Ragebot.double_tap_key"));
		setup_item(&cfg::g_cfg.ragebot.exploit_modifiers, 2, ("Ragebot.exploit_modifiers"));
		setup_item(&cfg::g_cfg.ragebot.body_aim_key, key_bind(TOGGLE), ("Ragebot.body_aim_key"));
		setup_item(&cfg::g_cfg.ragebot.resolver, true, ("Ragebot.roll_correction"));
		setup_item(&cfg::g_cfg.ragebot.headshot_only, false, ("Ragebot.headshot_only"));
		setup_item(&cfg::g_cfg.ragebot.roll_correction_bind, key_bind(TOGGLE), ("Ragebot.roll_correction_bind"));
		setup_item(&cfg::g_cfg.ragebot.damage_override_key, key_bind(HOLD), ("Ragebot.damage_override_key"));

		for (auto i = 0; i < 9; i++)
		{
			setup_item(&cfg::g_cfg.ragebot.weapon[i].selection_type, 0, std::to_string(i) + ("Ragebot.selection_type"));

			setup_item(&cfg::g_cfg.ragebot.weapon[i].hitboxes, 6, std::to_string(i) + ("Ragebot.hitboxes"));

			setup_item(&cfg::g_cfg.ragebot.weapon[i].autostop, false, std::to_string(i) + ("Ragebot_automaticstop"));
			setup_item(&cfg::g_cfg.ragebot.weapon[i].autostop_modifiers, 3, std::to_string(i) + ("Ragebot.autostop_modifiers"));
			setup_item(&cfg::g_cfg.ragebot.weapon[i].autostop_conditions, 3, std::to_string(i) + ("Ragebot.autostop_conditions"));

			setup_item(&cfg::g_cfg.ragebot.weapon[i].min_dmg, 1, std::to_string(i) + ("Ragebot.minimum_damage"));
			setup_item(&cfg::g_cfg.ragebot.weapon[i].min_override_dmg, 1, std::to_string(i) + ("Ragebot.minimum_override_damage"));

			setup_item(&cfg::g_cfg.ragebot.weapon[i].head_scale, 0.0f, std::to_string(i) + ("Ragebot.head_scale"));
			setup_item(&cfg::g_cfg.ragebot.weapon[i].body_scale, 0.0f, std::to_string(i) + ("Ragebot.body_scale"));

			setup_item(&cfg::g_cfg.ragebot.weapon[i].multipoints, 6, std::to_string(i) + ("Ragebot.multipoints"));

			setup_item(&cfg::g_cfg.ragebot.weapon[i].autoscope, false, std::to_string(i) + ("Ragebot.autoscope"));
			setup_item(&cfg::g_cfg.ragebot.weapon[i].autoscope_mode, 0, std::to_string(i) + ("Ragebot.autoscope_mode"));

			setup_item(&cfg::g_cfg.ragebot.weapon[i].hitchance_amount, 0, std::to_string(i) + ("Ragebot.hitchance_amount"));
			setup_item(&cfg::g_cfg.ragebot.weapon[i].double_tap_hc, false, std::to_string(i) + ("Ragebot.double_tap_hc"));
			setup_item(&cfg::g_cfg.ragebot.weapon[i].double_tap_hitchance_amount, 0, std::to_string(i) + ("Ragebot.doubletaphc_amount"));
			setup_item(&cfg::g_cfg.ragebot.weapon[i].noscope_hitchance_amount, 0, std::to_string(i) + ("Ragebot.noscopehitchance_amount"));

			setup_item(&cfg::g_cfg.ragebot.weapon[i].prefer_body_aim, false, std::to_string(i) + ("Ragebot.prefer_body_aim"));
			setup_item(&cfg::g_cfg.ragebot.weapon[i].prefer_body_aim_mode, 1, std::to_string(i) + ("Ragebot.prefer_body_aim_mode"));
		}
	}

	//Antiaim
	{
		setup_item(&cfg::g_cfg.antiaim.enable, false, crypt_str("Antiaim.enable"));
		setup_item(&cfg::g_cfg.antiaim.hide_shots, false, crypt_str("Ragebot.hide_shots"));
		setup_item(&cfg::g_cfg.antiaim.hide_shots_key, key_bind(TOGGLE), crypt_str("Ragebot.hide_shots_key"));
		setup_item(&cfg::g_cfg.antiaim.desync, 0, crypt_str("Antiaim.desync"));
		setup_item(&cfg::g_cfg.antiaim.freestand, false, crypt_str("Antiaim.freestand"));
		setup_item(&cfg::g_cfg.antiaim.freestand_key, key_bind(TOGGLE), crypt_str("Antiaim.freestand_key"));
		setup_item(&cfg::g_cfg.antiaim.manual_back, key_bind(TOGGLE), crypt_str("Antiaim.manual_back"));
		setup_item(&cfg::g_cfg.antiaim.manual_left, key_bind(TOGGLE), crypt_str("Antiaim.manual_left"));
		setup_item(&cfg::g_cfg.antiaim.manual_right, key_bind(TOGGLE), crypt_str("Antiaim.manual_right"));
		setup_item(&cfg::g_cfg.antiaim.flip_desync, key_bind(TOGGLE), crypt_str("Antiaim.flip_desync"));
		setup_item(&cfg::g_cfg.antiaim.fakelag, false, crypt_str("Antiaim.fake_lag"));
		setup_item(&cfg::g_cfg.antiaim.fakelag_type, 0, crypt_str("Antiaim.fake_lag_type"));
		setup_item(&cfg::g_cfg.antiaim.fakelag_enablers, 4, crypt_str("Antiaim.fake_lag_enablers"));
		setup_item(&cfg::g_cfg.antiaim.fakelag_amount, 1, crypt_str("Antiaim.fake_lag_limit"));
		setup_item(&cfg::g_cfg.antiaim.triggers_fakelag_amount, 1, crypt_str("Antiaim.triggers_fakelag_limit"));
		setup_item(&cfg::g_cfg.antiaim.pitch, 0, crypt_str("Antiaim.pitch"));
		setup_item(&cfg::g_cfg.antiaim.custom_pitch, 0, crypt_str("Antiaim.custom_pitch"));
		setup_item(&cfg::g_cfg.antiaim.base_angle, 0, crypt_str("Antiaim.base_angle"));
		setup_item(&cfg::g_cfg.antiaim.yaw_direction, 0, crypt_str("Antiaim.direction"));
		setup_item(&cfg::g_cfg.antiaim.yaw, 0, crypt_str("Antiaim.yaw"));
		setup_item(&cfg::g_cfg.antiaim.yaw_add_left, 0, crypt_str("Antiaim.yaw_left"));
		setup_item(&cfg::g_cfg.antiaim.yaw_add_right, 0, crypt_str("Antiaim.yaw_right"));
		setup_item(&cfg::g_cfg.antiaim.range, 1, crypt_str("Antiaim.range"));
		setup_item(&cfg::g_cfg.antiaim.speed, 1, crypt_str("Antiaim.speed"));
		setup_item(&cfg::g_cfg.antiaim.desync, 0, crypt_str("Antiaim.desync"));
		setup_item(&cfg::g_cfg.antiaim.desync_range, 60, crypt_str("Antiaim.desync_range"));
		setup_item(&cfg::g_cfg.antiaim.roll, 0, crypt_str("Antiaim.roll"));
	}


	//Players
	{
		setup_item(&cfg::g_cfg.player.enable, false, ("Player.enable"));
		setup_item(&cfg::g_cfg.player.arrows_color, Color(255, 255, 255), ("Player.arrows_color"));
		setup_item(&cfg::g_cfg.player.arrows, false, ("Player.arrows"));
		setup_item(&cfg::g_cfg.player.distance, 5, ("Player.arrows_distance"));
		setup_item(&cfg::g_cfg.player.size, 5, ("Player.arrows_size"));
		setup_item(&cfg::g_cfg.player.show_multi_points, false, ("Player.show_multi_points"));
		setup_item(&cfg::g_cfg.player.show_multi_points_color, Color(255, 255, 255), ("Player.show_multi_points_color"));
		setup_item(&cfg::g_cfg.player.lag_hitbox, false, ("Player.lag_hitbox"));
		setup_item(&cfg::g_cfg.player.lag_hitbox_color, Color(255, 255, 255), ("Player.lag_hitbox_color"));
		setup_item(&cfg::g_cfg.player.player_model_t, 0, ("Esp.player_model_t"));
		setup_item(&cfg::g_cfg.player.player_model_ct, 0, ("Esp.player_model_ct"));

		setup_item(&cfg::g_cfg.player.local_chams_type, 0, ("Player.local_chams_type"));
		setup_item(&cfg::g_cfg.player.fake_chams_enable, false, ("Player.fake_chams_enable"));
		setup_item(&cfg::g_cfg.player.visualize_lag, false, ("Player.visualize_lag"));
		setup_item(&cfg::g_cfg.player.layered, false, ("Player.layered"));

		setup_item(&cfg::g_cfg.player.fake_chams_color, Color(255, 255, 255), ("Player.fake_chams_color"));
		setup_item(&cfg::g_cfg.player.fake_chams_type, 0, ("Player.fake_chams_type"));

		setup_item(&cfg::g_cfg.player.fake_double_material, false, ("Player.fake_double_material"));
		setup_item(&cfg::g_cfg.player.fake_double_material_material, 0, ("Player.fake_double_material_material"));
		setup_item(&cfg::g_cfg.player.fake_double_material_color, Color(255, 255, 255), ("Player.fake_double_material_color"));

		setup_item(&cfg::g_cfg.player.backtrack_chams, false, ("Player.backtrack_chams"));
		setup_item(&cfg::g_cfg.player.backtrack_chams_material, 0, ("Player.backtrack_chams_material"));
		setup_item(&cfg::g_cfg.player.backtrack_chams_color, Color(255, 255, 255), ("Player.backtrack_chams_color"));


		setup_item(&cfg::g_cfg.player.transparency_in_scope, false, ("Player.transparency_in_scope"));
		setup_item(&cfg::g_cfg.player.transparency_in_scope_amount, 100, ("Player.transparency_in_scope_amount"));

		for (auto i = 0; i < 3; i++)
		{
			setup_item(&cfg::g_cfg.player.type[i].dormant_time, 10.f, std::to_string(i) + ("Player.dormant_time"));
			setup_item(&cfg::g_cfg.player.type[i].box, false, std::to_string(i) + ("Player.box"));
			setup_item(&cfg::g_cfg.player.type[i].box_color, Color(255, 255, 255), std::to_string(i) + ("Player.box_color"));
			setup_item(&cfg::g_cfg.player.type[i].name, false, std::to_string(i) + ("Player.name"));
			setup_item(&cfg::g_cfg.player.type[i].name_color, Color(255, 255, 255), std::to_string(i) + ("Player.name_color"));
			setup_item(&cfg::g_cfg.player.type[i].health, false, std::to_string(i) + ("Player.health"));
			setup_item(&cfg::g_cfg.player.type[i].custom_health_color, false, std::to_string(i) + ("Player.custom_health_color"));
			setup_item(&cfg::g_cfg.player.type[i].health_color, Color(100, 94, 255), std::to_string(i) + ("Player.health_color"));
			setup_item(&cfg::g_cfg.player.type[i].health_color_two, Color(255, 0, 76), std::to_string(i) + ("Player.health_color_two"));
			setup_item(&cfg::g_cfg.player.type[i].weapons, false, std::to_string(i) + ("Player.weapons"));
			setup_item(&cfg::g_cfg.player.type[i].weapon, 2, std::to_string(i) + ("Player.weapon"));
			setup_item(&cfg::g_cfg.player.type[i].weapon_color, Color(255, 255, 255), std::to_string(i) + ("Player.weapon_bar_color"));
			setup_item(&cfg::g_cfg.player.type[i].skeleton, false, std::to_string(i) + ("Player.skeleton"));
			setup_item(&cfg::g_cfg.player.type[i].skeleton_color, Color(255, 255, 255), std::to_string(i) + ("Player.skeleton_color"));
			setup_item(&cfg::g_cfg.player.type[i].ammo, false, std::to_string(i) + ("Player.ammo"));
			setup_item(&cfg::g_cfg.player.type[i].ammobar_color, Color(255, 255, 255), std::to_string(i) + ("Player.ammobar_color"));
			setup_item(&cfg::g_cfg.player.type[i].footsteps, false, std::to_string(i) + ("Player.footsteps"));
			setup_item(&cfg::g_cfg.player.type[i].footsteps_color, Color(255, 255, 255), std::to_string(i) + ("Player.footsteps_color"));
			setup_item(&cfg::g_cfg.player.type[i].thickness, 0, std::to_string(i) + ("Player.thickness"));
			setup_item(&cfg::g_cfg.player.type[i].radius, 0, std::to_string(i) + ("Player.radius"));
			setup_item(&cfg::g_cfg.player.type[i].glow, false, std::to_string(i) + ("Player.glow"));
			setup_item(&cfg::g_cfg.player.type[i].glow_color, Color(0, 153, 255, 255 / 2), std::to_string(i) + ("Player.glow_color"));

			// Default chams
			setup_item(&cfg::g_cfg.player.type[i].enable_chams, false, std::to_string(i) + ("Player.enable_chams"));
			setup_item(&cfg::g_cfg.player.type[i].xqz_enable, false, std::to_string(i) + ("Player.xqz_enable"));
			setup_item(&cfg::g_cfg.player.type[i].shot_enable, false, std::to_string(i) + ("Player.shot_enable"));
			setup_item(&cfg::g_cfg.player.type[i].chams_type, 1, std::to_string(i) + ("Player.chams_type"));
			setup_item(&cfg::g_cfg.player.type[i].chams_xqz, 1, std::to_string(i) + ("Player.chams_xqz"));
			setup_item(&cfg::g_cfg.player.type[i].chams_shot, 1, std::to_string(i) + ("Player.chams_shot"));
			setup_item(&cfg::g_cfg.player.type[i].chams_color, Color(0, 0, 0), std::to_string(i) + ("Player.chams_color"));
			setup_item(&cfg::g_cfg.player.type[i].xqz_color, Color(0, 0, 0), std::to_string(i) + ("Player.xqz_color"));
			setup_item(&cfg::g_cfg.player.type[i].shot_color, Color(0, 0, 0), std::to_string(i) + ("Player.shot_color"));

			// Double normal chams
			setup_item(&cfg::g_cfg.player.type[i].double_material, false, std::to_string(i) + ("Player.double_material"));
			setup_item(&cfg::g_cfg.player.type[i].double_material_material, 0, std::to_string(i) + ("Player.double_material_material"));
			setup_item(&cfg::g_cfg.player.type[i].double_material_color, Color(255, 255, 255), std::to_string(i) + ("Player.double_material_color"));

			// Double xqz chams
			setup_item(&cfg::g_cfg.player.type[i].double_material_xqz, false, std::to_string(i) + ("Player.double_material_xqz"));
			setup_item(&cfg::g_cfg.player.type[i].double_material_material_xqz, 0, std::to_string(i) + ("Player.double_material_material_xqz"));
			setup_item(&cfg::g_cfg.player.type[i].double_material_color_xqz, Color(255, 255, 255), std::to_string(i) + ("Player.double_material_color_xqz"));

			// Shot chams
			setup_item(&cfg::g_cfg.player.type[i].double_material_shot, false, std::to_string(i) + ("Player.double_material_shot"));
			setup_item(&cfg::g_cfg.player.type[i].double_material_material_shot, 0, std::to_string(i) + ("Player.double_material_material_shot"));
			setup_item(&cfg::g_cfg.player.type[i].double_material_color_shot, Color(255, 255, 255), std::to_string(i) + ("Player.double_material_color_shot"));
			setup_item(&cfg::g_cfg.player.type[i].shot_time, 0.5f, std::to_string(i) + ("Player.shot_chams_time"));

			// Ragdoll chams
			setup_item(&cfg::g_cfg.player.type[i].ragdoll_chams, false, std::to_string(i) + ("Player.ragdoll_chams"));
			setup_item(&cfg::g_cfg.player.type[i].ragdoll_chams_material, 0, std::to_string(i) + ("Player.ragdoll_chams_material"));
			setup_item(&cfg::g_cfg.player.type[i].ragdoll_chams_color, Color(255, 255, 255), std::to_string(i) + ("Player.ragdoll_chams_color"));
			setup_item(&cfg::g_cfg.player.type[i].flag, false, std::to_string(i) + ("Player.flag"));
			setup_item(&cfg::g_cfg.player.type[i].flags, 9, std::to_string(i) + ("Player.esp_flags"));
		}
	}

	//Visuals
	{
		setup_item(&cfg::g_cfg.visuals.enable, false, ("Visuals.enable"));
		setup_item(&cfg::g_cfg.visuals.removals, 11, ("Esp.world_removals"));
		setup_item(&cfg::g_cfg.misc.thirdperson_toggle, key_bind(TOGGLE), ("Misc.thirdperson_toggle"));
		setup_item(&cfg::g_cfg.misc.thirdperson_when_spectating, false, ("Misc.thirdperson_when_spectating"));
		setup_item(&cfg::g_cfg.misc.thirdperson_distance, 50, ("Misc.thirdperson_distance"));
		setup_item(&cfg::g_cfg.visuals.grenade_prediction, false, ("Esp.grenade_prediction"));
		setup_item(&cfg::g_cfg.visuals.on_click, false, ("Esp.on_click"));
		setup_item(&cfg::g_cfg.visuals.grenade_prediction_tracer_color, Color(255, 255, 255), ("Esp.grenade_prediction_tracer_color"));
		setup_item(&cfg::g_cfg.visuals.grenade_proximity_warning, false, ("Esp.grenade_proximity_warning"));
		setup_item(&cfg::g_cfg.visuals.grenade_proximity_warning_progress_color, Color(255, 255, 255), ("Esp.grenade_proximity_warning_progress_color"));
		setup_item(&cfg::g_cfg.visuals.projectiles, false, ("Esp.projectiles"));
		setup_item(&cfg::g_cfg.visuals.projectiles_color, Color(255, 255, 255), ("Esp.projectiles_color"));
		setup_item(&cfg::g_cfg.visuals.molotov_timer, false, ("Esp.molotov_timer"));
		setup_item(&cfg::g_cfg.visuals.molotov_timer_color, Color(255, 255, 255), ("Esp.molotov_timer_color"));
		setup_item(&cfg::g_cfg.visuals.smoke_timer, false, ("Esp.smoke_timer"));
		setup_item(&cfg::g_cfg.visuals.smoke_timer_color, Color(255, 255, 255), ("Esp.smoke_timer_color"));
		setup_item(&cfg::g_cfg.visuals.bomb_timer, false, ("Esp.bomb_timer"));
		setup_item(&cfg::g_cfg.visuals.nightmode, false, ("Esp.nightmode"));
		setup_item(&cfg::g_cfg.visuals.nightmode_amount, 100, ("Esp.nightmode_amount"));
		setup_item(&cfg::g_cfg.visuals.ambient_lighting, false, ("Esp.ambient_lighting"));
		setup_item(&cfg::g_cfg.visuals.world_color, Color(255, 255, 255), ("Esp.world_color"));
		setup_item(&cfg::g_cfg.visuals.prop_alpha, 255, ("Esp.props_alpha"));
		setup_item(&cfg::g_cfg.visuals.sunset_mode, false, ("Esp.sunset_mode"));
		setup_item(&cfg::g_cfg.visuals.skybox, 0, ("Esp.skybox"));
		setup_item(&cfg::g_cfg.visuals.custom_skybox, (""), ("Esp.custom_skybox"));
		setup_item(&cfg::g_cfg.visuals.skybox_color, Color(255, 255, 255), ("Esp.skybox_color"));
		setup_item(&cfg::g_cfg.visuals.client_bullet_impacts, false, ("Esp.client_bullet_impacts"));
		setup_item(&cfg::g_cfg.visuals.client_bullet_impacts_color, Color(255, 255, 255), ("Esp.client_bullet_impacts_color"));
		setup_item(&cfg::g_cfg.visuals.server_bullet_impacts, false, ("Esp.server_bullet_impacts"));
		setup_item(&cfg::g_cfg.visuals.server_bullet_impacts_color, Color(255, 255, 255), ("Esp.server_bullet_impacts_color"));
		setup_item(&cfg::g_cfg.visuals.bullet_impacts_size, 1.5f, ("Esp.bullet_impacts_size"));
		setup_item(&cfg::g_cfg.visuals.bullet_tracer, false, ("Esp.bullet_tracer"));
		setup_item(&cfg::g_cfg.visuals.bullet_tracer_color, Color(255, 255, 255), ("Esp.bullet_tracer_color"));
		setup_item(&cfg::g_cfg.visuals.enemy_bullet_tracer, false, ("Esp.enemy_bullet_tracer"));
		setup_item(&cfg::g_cfg.visuals.enemy_bullet_tracer_color, Color(255, 255, 255), ("Esp.enemy_bullet_tracer_color"));
		setup_item(&cfg::g_cfg.visuals.bullet_tracers_type, 3, ("Esp.bullet_tracer_type"));
		setup_item(&cfg::g_cfg.visuals.bullet_tracers_width, 4.f, ("Esp.bullet_tracer_width"));
		setup_item(&cfg::g_cfg.visuals.preserve_killfeed, false, ("Esp.preserve_killfeed"));
		setup_item(&cfg::g_cfg.visuals.hitmarker, false, ("Esp.hit_marker"));
		setup_item(&cfg::g_cfg.visuals.hiteffect, false, ("Esp.hit_effect"));
		setup_item(&cfg::g_cfg.visuals.hitsound, 0, ("Esp.hitsound"));
		setup_item(&cfg::g_cfg.visuals.damage_marker, false, ("Esp.damage_marker"));
		setup_item(&cfg::g_cfg.visuals.damage_marker_color, Color(0, 153, 255), ("Esp.damage_marker_color"));
		setup_item(&cfg::g_cfg.visuals.fov, 0, ("Esp.fov"));
		setup_item(&cfg::g_cfg.visuals.viewmodel_enable, false, ("Esp.viewmodel_enable"));
		setup_item(&cfg::g_cfg.visuals.viewmodel_fov, 0, ("Esp.viewmodel_fov"));
		setup_item(&cfg::g_cfg.visuals.viewmodel_x, 0, ("Esp.viewmodel_x"));
		setup_item(&cfg::g_cfg.visuals.viewmodel_y, 0, ("Esp.viewmodel_y"));
		setup_item(&cfg::g_cfg.visuals.viewmodel_z, 0, ("Esp.viewmodel_z"));
		setup_item(&cfg::g_cfg.visuals.arms_chams, false, ("Esp.arms_chams"));
		setup_item(&cfg::g_cfg.visuals.arms_chams_type, 0, ("Esp.arms_chams_type"));
		setup_item(&cfg::g_cfg.visuals.arms_chams_color, Color(255, 255, 255), ("Esp.arms_chams_color"));
		setup_item(&cfg::g_cfg.visuals.arms_double_material, false, ("Esp.arms_double_material"));
		setup_item(&cfg::g_cfg.visuals.arms_double_material_material, 0, ("Esp.arms_double_material_material"));
		setup_item(&cfg::g_cfg.visuals.arms_double_material_color, Color(255, 255, 255), ("Esp.arms_double_material_color"));
		setup_item(&cfg::g_cfg.visuals.weapon_chams, false, ("Esp.weapon_chams"));
		setup_item(&cfg::g_cfg.visuals.weapon_chams_type, 0, ("Esp.weapon_chams_type"));
		setup_item(&cfg::g_cfg.visuals.weapon_chams_color, Color(255, 255, 255), ("Esp.weapon_chams_color"));
		setup_item(&cfg::g_cfg.visuals.weapon_double_material, false, ("Esp.weapon_double_material"));
		setup_item(&cfg::g_cfg.visuals.weapon_double_material_material, 0, ("Esp.weapon_double_material_material"));
		setup_item(&cfg::g_cfg.visuals.weapon_double_material_color, Color(255, 255, 255), ("Esp.weapon_double_material_color"));
		setup_item(&cfg::g_cfg.visuals.show_spread, false, ("Esp.show_spread"));
		setup_item(&cfg::g_cfg.visuals.show_spread_color, Color(255, 255, 255), ("Esp.show_spread_color"));
		setup_item(&cfg::g_cfg.visuals.forcecrosshair, false, ("Esp.force_crosshair"));
		setup_item(&cfg::g_cfg.visuals.recoil_crosshair, false, ("Esp.recoil_crosshair"));
		setup_item(&cfg::g_cfg.visuals.penetration_crosshair, false, ("Esp.penetration_crosshair"));
		setup_item(&cfg::g_cfg.visuals.world_modulation, false, ("Esp.world_modulation"));
		setup_item(&cfg::g_cfg.visuals.bloom, 0.0f, ("Esp.bloom"));
		setup_item(&cfg::g_cfg.visuals.exposure, 0.0f, ("Esp.exposure"));
		setup_item(&cfg::g_cfg.visuals.ambient, 0.0f, ("Esp.ambient"));
		setup_item(&cfg::g_cfg.visuals.fog, false, ("Esp.fog"));
		setup_item(&cfg::g_cfg.visuals.fog_distance, 0, ("Esp.fog_distance"));
		setup_item(&cfg::g_cfg.visuals.fog_density, 0, ("Esp.fog_density"));
		setup_item(&cfg::g_cfg.visuals.fog_color, Color(255, 255, 255), ("Esp.fog_color"));
		setup_item(&cfg::g_cfg.visuals.precipitation, false, ("Esp.precipitation"));
		setup_item(&cfg::g_cfg.visuals.precipitation_mode, 0, ("Esp.precipitation_type"));
		setup_item(&cfg::g_cfg.visuals.weapon, 6, ("Esp.dropped_weapon"));
		setup_item(&cfg::g_cfg.visuals.box_color, Color(255, 255, 255), ("Esp.dropped_weapon_box_color"));
		setup_item(&cfg::g_cfg.visuals.weapon_color, Color(255, 255, 255), ("Esp.dropped_weapon_color"));
		setup_item(&cfg::g_cfg.visuals.weapon_glow_color, Color(255, 255, 255), ("Esp.dropped_weapon_glow_color"));
		setup_item(&cfg::g_cfg.visuals.weapon_ammo_color, Color(255, 255, 255), ("Esp.dropped_weapon_ammo_color"));
		setup_item(&cfg::g_cfg.visuals.grenade_esp, 4, ("Esp.grenade_esp"));
		setup_item(&cfg::g_cfg.visuals.grenade_glow_color, Color(255, 255, 255), ("Esp.grenade_glow_color"));
		setup_item(&cfg::g_cfg.visuals.grenade_box_color, Color(255, 255, 255), ("Esp.grenade_box_color"));
	}
	
	//Misc, skins & menu
	{
		setup_item(&cfg::g_cfg.menu.spectators_list, false, ("Misc.spectators_list"));
		setup_item(&cfg::g_cfg.misc.ragdolls, false, ("Misc.ragdolls"));
		setup_item(&cfg::g_cfg.misc.bunnyhop, false, ("Misc.autohop"));
		setup_item(&cfg::g_cfg.misc.airstrafe, 0, ("Misc.airstrafe"));
		setup_item(&cfg::g_cfg.misc.crouch_in_air, false, ("Misc.crouch_in_air"));
		setup_item(&cfg::g_cfg.misc.automatic_peek, key_bind(TOGGLE), ("Misc.automatic_peek"));
		setup_item(&cfg::g_cfg.misc.edge_jump, key_bind(HOLD), ("Misc.edge_jump"));
		setup_item(&cfg::g_cfg.misc.noduck, false, ("Misc.noduck"));
		setup_item(&cfg::g_cfg.misc.fakeduck_key, key_bind(HOLD), ("Misc.fakeduck_key"));
		setup_item(&cfg::g_cfg.misc.fast_stop, false, ("Misc.fast_stop"));
		setup_item(&cfg::g_cfg.misc.slidewalk, false, ("Misc.slidewalk"));
		setup_item(&cfg::g_cfg.misc.slowwalk_key, key_bind(HOLD), ("Misc.slowwalk_key"));
		setup_item(&cfg::g_cfg.misc.slowwalk_speed, 0.25, crypt_str("antiaim.slow_walk:speed"));
		setup_item(&cfg::g_cfg.misc.slowwalk_type, 0, crypt_str("misc.slow_walk:type"));
		setup_item(&cfg::g_cfg.misc.log_output, false, ("Misc.log_output"));
		setup_item(&cfg::g_cfg.misc.debug_log, false, ("Misc.debug_log"));
		setup_item(&cfg::g_cfg.misc.events_to_log, 3, ("Misc.events_to_log"));
		setup_item(&cfg::g_cfg.misc.console_filter, false, ("Misc.show_default_log"));
		setup_item(&cfg::g_cfg.misc.log_color, Color(255, 255, 255), ("Misc.log_color"));
		setup_item(&cfg::g_cfg.misc.inventory_access, false, ("Misc.inventory_access"));
		setup_item(&cfg::g_cfg.misc.rank_reveal, false, ("Misc.rank_reveal"));
		setup_item(&cfg::g_cfg.misc.clantag_spammer, false, ("Misc.clantag_spammer"));
		setup_item(&cfg::g_cfg.misc.aspect_ratio, false, ("Misc.aspect_ratio"));
		setup_item(&cfg::g_cfg.misc.aspect_ratio_amount, 1.7f, ("Misc.aspect_ratio_amount"));
		setup_item(&cfg::g_cfg.misc.anti_untrusted, true, ("Misc.antiuntrusted"));
		setup_item(&cfg::g_cfg.misc.buybot_enable, false, ("Misc.buybot"));
		setup_item(&cfg::g_cfg.misc.buybot1, 0, ("Player.buybot1"));
		setup_item(&cfg::g_cfg.misc.buybot2, 0, ("Player.buybot2"));
		setup_item(&cfg::g_cfg.misc.buybot3, 4, ("Misc.buybot3"));
		setup_item(&cfg::g_cfg.misc.extended_backtracking, false, ("Misc.extended_backtrack"));
		setup_item(&cfg::g_cfg.misc.extended_backtracking_value, 1, ("Misc.extended_backtrack_value"));
		setup_item(&cfg::g_cfg.skins.rare_animations, false, ("Skins.rare_animations"));
		setup_item(&cfg::g_cfg.skins.show_names, false, ("Skins.show_weapon_names"));

		for (auto i = 0; i < cfg::g_cfg.skins.skinChanger.size(); i++)
		{
			setup_item(&cfg::g_cfg.skins.skinChanger.at(i).definition_override_index, 0, std::to_string(i) + ("Skins.definition_override_index")); //-V807
			setup_item(&cfg::g_cfg.skins.skinChanger.at(i).definition_override_vector_index, 0, std::to_string(i) + ("Skins.definition_override_vector_index"));
			setup_item(&cfg::g_cfg.skins.skinChanger.at(i).entity_quality_vector_index, 0, std::to_string(i) + ("Skins.entity_quality_vector_index"));
			setup_item(&cfg::g_cfg.skins.skinChanger.at(i).itemId, 0, std::to_string(i) + ("Skins.itemId"));
			setup_item(&cfg::g_cfg.skins.skinChanger.at(i).itemIdIndex, 0, std::to_string(i) + ("Skins.itemIdIndex"));
			setup_item(&cfg::g_cfg.skins.skinChanger.at(i).paintKit, 0, std::to_string(i) + ("Skins.paintKit"));
			setup_item(&cfg::g_cfg.skins.skinChanger.at(i).paint_kit_vector_index, 0, std::to_string(i) + ("Skins.paint_kit_vector_index"));
			setup_item(&cfg::g_cfg.skins.skinChanger.at(i).quality, 0, std::to_string(i) + ("Skins.quality"));
			setup_item(&cfg::g_cfg.skins.skinChanger.at(i).seed, 0, std::to_string(i) + ("Skins.seed"));
			setup_item(&cfg::g_cfg.skins.skinChanger.at(i).stat_trak, 0, std::to_string(i) + ("Skins.stat_trak"));
			setup_item(&cfg::g_cfg.skins.skinChanger.at(i).wear, 0.0f, std::to_string(i) + ("Skins.wear"));
			setup_item(&cfg::g_cfg.skins.custom_name_tag[i], (""), std::to_string(i) + ("Skins.custom_name_tag"));
		}

		setup_item(&cfg::g_cfg.menu.menu_color, false, ("Menu.menu_color"));
		setup_item(&cfg::g_cfg.menu.menu_color_col, Color(0, 153, 255), ("Menu.menu_color_col"));
		setup_item(&cfg::g_cfg.menu.watermark, false, ("Menu.watermark"));
		setup_item(&cfg::g_cfg.menu.keybind_list, false, ("Menu.keybinds"));
		setup_item(&cfg::g_cfg.scripts.scripts, ("Scripts.loaded"));
	}
}

void C_ConfigManager::add_item(void* pointer, const char* name, const std::string& type) {
	items.push_back(new C_ConfigItem(std::string(name), pointer, type));
}

void C_ConfigManager::setup_item(int* pointer, int value, const std::string& name)
{
	add_item(pointer, name.c_str(), ("int"));
	*pointer = value;
}

void C_ConfigManager::setup_item(bool* pointer, bool value, const std::string& name)
{
	add_item(pointer, name.c_str(), ("bool"));
	*pointer = value;
}

void C_ConfigManager::setup_item(float* pointer, float value, const std::string& name)
{
	add_item(pointer, name.c_str(), ("float"));
	*pointer = value;
}

void C_ConfigManager::setup_item(key_bind* pointer, key_bind value, const std::string& name) //-V813
{
	add_item(pointer, name.c_str(), ("KeyBind"));
	*pointer = value;
}

void C_ConfigManager::setup_item(Color* pointer, Color value, const std::string& name)
{
	colors[name][0] = (float)value.r() / 255.0f;
	colors[name][1] = (float)value.g() / 255.0f;
	colors[name][2] = (float)value.b() / 255.0f;
	colors[name][3] = (float)value.a() / 255.0f;

	add_item(pointer, name.c_str(), ("Color"));
	*pointer = value;
}

void C_ConfigManager::setup_item(std::vector< int >* pointer, int size, const std::string& name)
{
	add_item(pointer, name.c_str(), ("vector<int>"));
	pointer->clear();

	for (int i = 0; i < size; i++)
		pointer->push_back(FALSE);
}

void C_ConfigManager::setup_item(std::vector< std::string >* pointer, const std::string& name)
{
	add_item(pointer, name.c_str(), ("vector<string>"));
}

void C_ConfigManager::setup_item(std::string* pointer, const std::string& value, const std::string& name)
{
	add_item(pointer, name.c_str(), ("string"));
	*pointer = value; //-V820
}

void C_ConfigManager::save(std::string config)
{
	std::string folder, file;

	auto get_dir = [&folder, &file, &config]() -> void
	{
		static TCHAR path[MAX_PATH];

		folder = std::string(path) + ("luna\\configs\\");

		file = std::string(path) + ("luna\\configs\\") + config;

		CreateDirectory(folder.c_str(), nullptr);
	};

	get_dir();
	json allJson;

	for (auto it : items)
	{
		json j;

		j[("name")] = it->name;
		j[("type")] = it->type;

		if (!it->type.compare(("int")))
			j[("value")] = (int)*(int*)it->pointer; //-V206
		else if (!it->type.compare(("float")))
			j[("value")] = (float)*(float*)it->pointer;
		else if (!it->type.compare(("bool")))
			j[("value")] = (bool)*(bool*)it->pointer;
		else if (!it->type.compare(("KeyBind")))
		{
			auto k = *(key_bind*)(it->pointer);

			std::vector <int> a = { k.key, k.mode };
			json ja;

			for (auto& i : a)
				ja.push_back(i);

			j[("value")] = ja.dump();
		}
		else if (!it->type.compare(("Color")))
		{
			auto c = *(Color*)(it->pointer);

			std::vector<int> a = { c.r(), c.g(), c.b(), c.a() };
			json ja;

			for (auto& i : a)
				ja.push_back(i);

			j[("value")] = ja.dump();
		}
		else if (!it->type.compare(("vector<int>")))
		{
			auto& ptr = *(std::vector<int>*)(it->pointer);
			json ja;

			for (auto& i : ptr)
				ja.push_back(i);

			j[("value")] = ja.dump();
		}
		else if (!it->type.compare(("vector<string>")))
		{
			auto& ptr = *(std::vector<std::string>*)(it->pointer);
			json ja;

			for (auto& i : ptr)
				ja.push_back(i);

			j[("value")] = ja.dump();
		}
		else if (!it->type.compare(("string")))
			j[("value")] = (std::string) * (std::string*)it->pointer;

		allJson.push_back(j);
	}

	auto get_type = [](menu_item_type type)
	{
		switch (type) //-V719
		{
		case CHECK_BOX:
			return "bool";
		case COMBO_BOX:
		case SLIDER_INT:
			return "int";
		case SLIDER_FLOAT:
			return "float";
		case COLOR_PICKER:
			return "Color";
		}
	};

	for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
	{
		auto& script = c_lua::get().scripts.at(i);

		for (auto& item : c_lua::get().items.at(i))
		{
			if (item.second.type == NEXT_LINE)
				continue;

			json j;
			auto type = (std::string)get_type(item.second.type);

			j[("name")] = item.first;
			j[("type")] = type;

			if (!type.compare(("bool")))
				j[("value")] = item.second.check_box_value;
			else if (!type.compare(("int")))
				j[("value")] = item.second.type == COMBO_BOX ? item.second.combo_box_value : item.second.slider_int_value;
			else if (!type.compare(("float")))
				j[("value")] = item.second.slider_float_value;
			else if (!type.compare(("Color")))
			{
				std::vector <int> color =
				{
					item.second.color_picker_value.r(),
					item.second.color_picker_value.g(),
					item.second.color_picker_value.b(),
					item.second.color_picker_value.a()
				};

				json j_color;

				for (auto& i : color)
					j_color.push_back(i);

				j[("value")] = j_color.dump();
			}

			allJson.push_back(j);
		}
	}


	auto str = allJson.dump();
	std::vector<uint8_t> vec_enc = std::vector<uint8_t>(str.begin(), str.end());
	(vec_enc);
	auto str_enc = std::string(vec_enc.begin(), vec_enc.end());

	std::ofstream file_out(file, std::ios::binary | std::ios::trunc);
	if (file_out.good())
		file_out.write(str_enc.data(), str_enc.size());

	file_out.close();

	/*
	std::string data;

	Base64 base64;
	base64.encode(allJson.dump(), &data);

	std::ofstream ofs;
	ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	ofs << std::setw(4) << data << std::endl;
	ofs.close();*/
}
std::vector<uint8_t> ReadAllBytes2(char const* filename)
{
	std::vector<uint8_t> result;

	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	if (ifs)
	{
		std::ifstream::pos_type pos = ifs.tellg();
		result.resize(pos);

		ifs.seekg(0, std::ios::beg);
		ifs.read((char*)&result[0], pos);
	}

	return result;
}
void C_ConfigManager::load(std::string config, bool load_script_items)
{
	static auto find_item = [](std::vector< C_ConfigItem* > items, std::string name) -> C_ConfigItem*
	{
		for (int i = 0; i < (int)items.size(); i++) //-V202
			if (!items[i]->name.compare(name))
				return items[i];

		return nullptr;
	};

	std::string folder, file;

	auto get_dir = [&folder, &file, &config]() ->void
	{
		static TCHAR path[MAX_PATH];


		folder = std::string(path) + ("luna\\configs\\");
		file = std::string(path) + ("luna\\configs\\") + config;


		CreateDirectory(folder.c_str(), nullptr);
	};

	get_dir();

	std::vector<uint8_t> vec_enc;
	json allJson;

	vec_enc = ReadAllBytes2(file.c_str());



	std::vector<uint8_t> vec_dec = vec_enc;
	(vec_dec);
	std::stringstream stream_dec;
	auto str_dec = std::string(vec_dec.begin(), vec_dec.end());
	stream_dec << str_dec;
	stream_dec >> allJson;

	/*
	std::string data;

	std::ifstream ifs;
	ifs.open(file + '\0');

	ifs >> data;
	ifs.close();

	if (data.empty())
		return;

	Base64 base64;

	std::string decoded_data;
	base64.decode(data, &decoded_data);

	std::ofstream ofs;
	ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	ofs << decoded_data;
	ofs.close();



	std::ifstream ifs_final;
	ifs_final.open(file + '\0');

	ifs_final >> allJson;
	ifs_final.close();

	base64.encode(allJson.dump(), &data);

	std::ofstream final_ofs;
	final_ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	final_ofs << data;
	final_ofs.close();
	*/
	for (auto it = allJson.begin(); it != allJson.end(); ++it)
	{
		json j = *it;

		std::string name = j[("name")];
		std::string type = j[("type")];

		auto script_item = std::count_if(name.begin(), name.end(),
			[](char& c)
			{
				return c == '.';
			}
		) >= 2;

		if (load_script_items && script_item)
		{
			std::string script_name;
			auto first_point = false;

			for (auto& c : name)
			{
				if (c == '.')
				{
					if (first_point)
						break;
					else
						first_point = true;
				}

				script_name.push_back(c);
			}

			auto script_id = c_lua::get().get_script_id(script_name);

			if (script_id == -1)
				continue;

			for (auto& current_item : c_lua::get().items.at(script_id))
			{
				if (current_item.first == name)
				{
					if (!type.compare(("bool")))
					{
						current_item.second.type = CHECK_BOX;
						current_item.second.check_box_value = j[("value")].get<bool>();
					}
					else if (!type.compare(("int")))
					{
						if (current_item.second.type == COMBO_BOX)
							current_item.second.combo_box_value = j[("value")].get<int>();
						else
							current_item.second.slider_int_value = j[("value")].get<int>();
					}
					else if (!type.compare(("float")))
						current_item.second.slider_float_value = j[("value")].get<float>();
					else if (!type.compare(("Color")))
					{
						std::vector<int> a;
						json ja = json::parse(j[("value")].get<std::string>().c_str());

						for (json::iterator it = ja.begin(); it != ja.end(); ++it)
							a.push_back(*it);

						colors.erase(name);
						current_item.second.color_picker_value = Color(a[0], a[1], a[2], a[3]);
					}
				}
			}
		}
		else if (!load_script_items && !script_item)
		{
			auto item = find_item(items, name);

			if (item)
			{
				if (!type.compare(("int")))
					*(int*)item->pointer = j[("value")].get<int>(); //-V206
				else if (!type.compare(("float")))
					*(float*)item->pointer = j[("value")].get<float>();
				else if (!type.compare(("bool")))
					*(bool*)item->pointer = j[("value")].get<bool>();
				else if (!type.compare(("KeyBind")))
				{
					std::vector<int> a;
					json ja = json::parse(j[("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						a.push_back(*it);

					*(key_bind*)item->pointer = key_bind((ButtonCode_t)a[0], (key_bind_mode)a[1]);
				}
				else if (!type.compare(("Color")))
				{
					std::vector<int> a;
					json ja = json::parse(j[("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						a.push_back(*it);

					colors.erase(item->name);
					*(Color*)item->pointer = Color(a[0], a[1], a[2], a[3]);
				}
				else if (!type.compare(("vector<int>")))
				{
					auto ptr = static_cast<std::vector <int>*> (item->pointer);
					ptr->clear();

					json ja = json::parse(j[("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						ptr->push_back(*it);
				}
				else if (!type.compare(("vector<string>")))
				{
					auto ptr = static_cast<std::vector <std::string>*> (item->pointer);
					ptr->clear();

					json ja = json::parse(j[("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						ptr->push_back(*it);
				}
				else if (!type.compare(("string")))
					*(std::string*)item->pointer = j[("value")].get<std::string>();
			}
		}
	}
}

void C_ConfigManager::remove(std::string config)
{
	std::string folder, file;

	auto get_dir = [&folder, &file, &config]() -> void
	{
		static TCHAR path[MAX_PATH];

		folder = std::string(path) + ("luna\\configs\\");

		file = std::string(path) + ("luna\\configs\\") + config;

		CreateDirectory(folder.c_str(), nullptr);
	};

	get_dir();

	std::string path = file + '\0';
	std::remove(path.c_str());
}


void C_ConfigManager::config_files()
{
	std::string folder;

	auto get_dir = [&folder]() -> void
	{
		static TCHAR path[MAX_PATH];


		folder = std::string(path) + ("luna\\configs\\");

		CreateDirectory(folder.c_str(), nullptr);
	};

	get_dir();
	files.clear();

	std::string path = folder + ("/*.cfg");
	WIN32_FIND_DATA fd;

	HANDLE hFind = FindFirstFile(path.c_str(), &fd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				files.push_back(fd.cFileName);
		} while (FindNextFile(hFind, &fd));

		FindClose(hFind);
	}
}