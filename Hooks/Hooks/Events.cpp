// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\Features\cheats\ragebot\ragebot.h"
#include "..\..\Features\cheats\misc\logs.h"
#include "..\..\Features\cheats\visuals\OtherEsp.h"
#include "..\..\Features\cheats\visuals\BulletTracers.h"
#include "..\..\Features\cheats\visuals\PlayerEsp.h"
#include "..\..\Features\cheats\ragebot\antiaim.h"
#include "..\..\Features\cheats\visuals\nightmode.h"
#include "..\..\Features\cheats\misc\misc.h"
#include "..\..\Features\cheats\visuals\DormantEsp.h"
#include "..\..\Features\cheats\lagcompensation\LagCompensation.h"
#include "..\..\Features\cheats\visuals\hitmarker.h"


bool weapon_is_aim(const std::string& weapon)
{
	return weapon.find(crypt_str("decoy")) == std::string::npos && weapon.find(crypt_str("flashbang")) == std::string::npos &&
		weapon.find(crypt_str("hegrenade")) == std::string::npos && weapon.find(crypt_str("inferno")) == std::string::npos &&
		weapon.find(crypt_str("molotov")) == std::string::npos && weapon.find(crypt_str("smokegrenade")) == std::string::npos;
}

void C_HookedEvents::FireGameEvent(IGameEvent* event)
{
	auto event_name = event->GetName();

	if (g_ctx.globals.loaded_script)
	{
		for (auto& script : c_lua::get().scripts)
		{
			auto script_id = c_lua::get().get_script_id(script);

			if (c_lua::get().events.find(script_id) == c_lua::get().events.end())
				continue;

			if (c_lua::get().events[script_id].find(event_name) == c_lua::get().events[script_id].end())
				continue;

			c_lua::get().events[script_id][event_name](event);
		}
	}

	if (!strcmp(event->GetName(), crypt_str("player_footstep")))
	{
		auto userid = m_engine()->GetPlayerForUserID(event->GetInt(crypt_str("userid")));
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid));

		if (e->valid(false, false))
		{
			auto type = ENEMY;

			if (e == g_ctx.local())
				type = LOCAL;
			else if (e->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && !m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1)
				type = TEAM;

			if (cfg::g_cfg.player.type[type].footsteps)
			{
				static auto model_index = m_modelinfo()->GetModelIndex(crypt_str("sprites/white.vmt"));

				if (g_ctx.globals.should_update_beam_index)
					model_index = m_modelinfo()->GetModelIndex(crypt_str("sprites/white.vmt"));

				BeamInfo_t info;

				info.m_nType = TE_BEAMRINGPOINT;
				info.m_pszModelName = crypt_str("sprites/white.vmt");
				info.m_nModelIndex = model_index;
				info.m_nHaloIndex = -1;
				info.m_flHaloScale = 3.0f;
				info.m_flLife = 2.0f;
				info.m_flWidth = (float)cfg::g_cfg.player.type[type].thickness / 8;
				info.m_flFadeLength = 1.0f;
				info.m_flAmplitude = 0.0f;
				info.m_flRed = (float)cfg::g_cfg.player.type[type].footsteps_color.r();
				info.m_flGreen = (float)cfg::g_cfg.player.type[type].footsteps_color.g();
				info.m_flBlue = (float)cfg::g_cfg.player.type[type].footsteps_color.b();
				info.m_flBrightness = (float)cfg::g_cfg.player.type[type].footsteps_color.a();
				info.m_flSpeed = 0.0f;
				info.m_nStartFrame = 0.0f;
				info.m_flFrameRate = 60.0f;
				info.m_nSegments = -1;
				info.m_nFlags = FBEAM_FADEOUT;
				info.m_vecCenter = e->GetAbsOrigin() + Vector(0.0f, 0.0f, 5.0f);
				info.m_flStartRadius = 5.0f;
				info.m_flEndRadius = (float)cfg::g_cfg.player.type[type].radius;
				info.m_bRenderable = true;

				auto beam_draw = m_viewrenderbeams()->CreateBeamRingPoint(info);

				if (beam_draw)
					m_viewrenderbeams()->DrawBeam(beam_draw);
			}
		}

	}
	else if (!strcmp(event_name, crypt_str("weapon_fire")))
	{
		auto user_id = event->GetInt(crypt_str("userid"));
		auto user = m_engine()->GetPlayerForUserID(user_id);

		if (user == m_engine()->GetLocalPlayer())
		{
			aim_shot* current_shot = nullptr;

			for (auto& shot : g_ctx.shots)
			{
				if (shot.start)
				{
					shot.end = true;
					continue;
				}

				if (shot.end)
					continue;

				current_shot = &shot;
				break;
			}

			if (current_shot)
			{
				current_shot->start = true;
				current_shot->event_fire_tick = m_globals()->m_tickcount;

				++g_ctx.globals.fired_shots[Rbot::get().last_target[current_shot->last_target].record.i];
			}
		}
	}
	else if (!strcmp(event_name, crypt_str("bullet_impact")))
	{
		auto user_id = event->GetInt(crypt_str("userid"));
		auto user = m_engine()->GetPlayerForUserID(user_id); //-V807

		if (user == m_engine()->GetLocalPlayer())
		{
			Vector position(event->GetFloat(crypt_str("x")), event->GetFloat(crypt_str("y")), event->GetFloat(crypt_str("z")));
			Vector HeadPosition = g_ctx.local()->hitbox_position(CSGOHitboxID::Head);
			if (cfg::g_cfg.visuals.enable && cfg::g_cfg.visuals.server_bullet_impacts)
				m_debugoverlay()->BoxOverlay(position, Vector(-cfg::g_cfg.visuals.bullet_impacts_size, -cfg::g_cfg.visuals.bullet_impacts_size, -cfg::g_cfg.visuals.bullet_impacts_size), Vector(cfg::g_cfg.visuals.bullet_impacts_size, cfg::g_cfg.visuals.bullet_impacts_size, cfg::g_cfg.visuals.bullet_impacts_size), QAngle(0.0f, 0.0f, 0.0f), cfg::g_cfg.visuals.server_bullet_impacts_color.r(), cfg::g_cfg.visuals.server_bullet_impacts_color.g(), cfg::g_cfg.visuals.server_bullet_impacts_color.b(), cfg::g_cfg.visuals.server_bullet_impacts_color.a(), 4.0f);

			if (cfg::g_cfg.visuals.enable && cfg::g_cfg.visuals.bullet_tracer)
				bullettracers::get().RenderBeam(false, HeadPosition, position, cfg::g_cfg.visuals.bullet_tracer_color, false);

			aim_shot* current_shot = nullptr;

			for (auto& shot : g_ctx.shots)
			{
				if (!shot.start)
					continue;

				if (shot.end)
					continue;

				current_shot = &shot;
				break;
			}

			if (current_shot && Rbot::get().last_target[current_shot->last_target].record.player->valid(true, false)) //-V807
			{
				auto backup_data = adjust_data(Rbot::get().last_target[current_shot->last_target].record.player);
				Rbot::get().last_target[current_shot->last_target].record.adjust_player();

				trace_t trace, trace_zero, trace_first, trace_second;
				Ray_t ray;

				ray.Init(Rbot::get().last_shoot_position, position);
				m_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, Rbot::get().last_target[current_shot->last_target].record.player, &trace);

				if (Rbot::get().last_target[current_shot->last_target].data.point.safe)
				{
					memcpy(Rbot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Base(), Rbot::get().last_target[current_shot->last_target].record.matrixes_data.zero, Rbot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Count() * sizeof(matrix3x4_t)); //-V807
					m_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, Rbot::get().last_target[current_shot->last_target].record.player, &trace_zero);

					memcpy(Rbot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Base(), Rbot::get().last_target[current_shot->last_target].record.matrixes_data.first, Rbot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Count() * sizeof(matrix3x4_t)); //-V807
					m_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, Rbot::get().last_target[current_shot->last_target].record.player, &trace_first);

					memcpy(Rbot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Base(), Rbot::get().last_target[current_shot->last_target].record.matrixes_data.second, Rbot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Count() * sizeof(matrix3x4_t)); //-V807
					m_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, Rbot::get().last_target[current_shot->last_target].record.player, &trace_second);
				}

				auto hit = trace.hit_entity == Rbot::get().last_target[current_shot->last_target].record.player;

				if (Rbot::get().last_target[current_shot->last_target].data.point.safe)
					hit = hit && trace_zero.hit_entity == Rbot::get().last_target[current_shot->last_target].record.player && trace_first.hit_entity == Rbot::get().last_target[current_shot->last_target].record.player && trace_second.hit_entity == Rbot::get().last_target[current_shot->last_target].record.player;

				if (hit)
					current_shot->impact_hit_player = true;
				else if (Rbot::get().last_shoot_position.DistTo(position) < Rbot::get().last_target[current_shot->last_target].distance)
					current_shot->occlusion = true;
				else
					current_shot->occlusion = false;

				current_shot->impacts = true;
				backup_data.adjust_player();
			}
		}
	}
	else if (!strcmp(event_name, crypt_str("player_death")))
	{
		auto attacker = event->GetInt(crypt_str("attacker"));
		auto user = event->GetInt(crypt_str("userid"));

		auto attacker_id = m_engine()->GetPlayerForUserID(attacker); //-V807
		auto user_id = m_engine()->GetPlayerForUserID(user);

		if (g_ctx.local()->is_alive() && attacker_id == m_engine()->GetLocalPlayer() && user_id != m_engine()->GetLocalPlayer())
		{
			auto entity = static_cast<player_t*>(m_entitylist()->GetClientEntity(user_id));

			g_ctx.globals.kills++;

			std::string weapon_name = event->GetString(crypt_str("weapon"));
			auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

			if (weapon && weapon_name.find(crypt_str("knife")) != std::string::npos)
				SkinChanger::overrideHudIcon(event);
		}
	}
	else if (!strcmp(event_name, crypt_str("player_hurt")))
	{
		auto attacker = event->GetInt(crypt_str("attacker"));
		auto user = event->GetInt(crypt_str("userid"));

		auto attacker_id = m_engine()->GetPlayerForUserID(attacker);
		auto user_id = m_engine()->GetPlayerForUserID(user);

		static auto get_hitbox_by_hitgroup = [](int hitgroup) -> int
		{
			switch (hitgroup)
			{
			case HITGROUP_HEAD:
				return CSGOHitboxID::Head;
			case HITGROUP_CHEST:
				return CSGOHitboxID::Chest;
			case HITGROUP_STOMACH:
				return CSGOHitboxID::Stomach;
			case HITGROUP_LEFTARM:
				return CSGOHitboxID::LeftHand;
			case HITGROUP_RIGHTARM:
				return CSGOHitboxID::RightHand;
			case HITGROUP_LEFTLEG:
				return CSGOHitboxID::RightCalf;
			case HITGROUP_RIGHTLEG:
				return CSGOHitboxID::LeftCalf;
			default:
				return CSGOHitboxID::Pelvis;
			}
		};


		player_t* hurt = (player_t*)m_entitylist()->GetClientEntity(m_engine()->GetPlayerForUserID(event->GetInt("userid")));
		player_t* attackerE = (player_t*)m_entitylist()->GetClientEntity(m_engine()->GetPlayerForUserID(event->GetInt("attacker")));

		if (hurt && attackerE) {
			bool hurt_dead = event->GetInt("health") == 0;
			auto headshot = event->GetInt("hitgroup") == HITGROUP_HEAD;
			if (cfg::g_cfg.visuals.damage_marker && cfg::g_cfg.visuals.enable) {
				if (hurt != g_ctx.local() && attackerE == g_ctx.local()) {
					hitmarker::get().damage = event->GetInt("dmg_health");
					hitmarker::get().player = hurt;
					hitmarker::get().col = cfg::g_cfg.visuals.damage_marker_color;
					hitmarker::get().hit_box = event->GetInt("hitgroup");
					hitmarker::get().erase_time = m_globals()->m_curtime + 3.0;
					hitmarker::get().initialized = false;
					auto entity = static_cast<player_t*>(m_entitylist()->GetClientEntity(user_id));
					
					if (event->GetInt("hitgroup") != HITGROUP_GENERIC)
						hitmarker::get().add_hit(hitmarker_t(m_globals()->m_curtime, user_id, event->GetInt("dmg_health"), event->GetInt("hitgroup"), hurt->hitbox_position_matrix(get_hitbox_by_hitgroup(event->GetInt("hitgroup")), entity->m_CachedBoneData().Base())));
				}
			}
		}

		if (attacker_id == m_engine()->GetLocalPlayer() && user_id != m_engine()->GetLocalPlayer())
		{
			if (cfg::g_cfg.visuals.hitsound)
			{
				switch (cfg::g_cfg.visuals.hitsound)
				{
				case 1:
					m_surface()->PlaySound_(crypt_str("metallic.wav"));
					break;
				case 2:
					m_surface()->PlaySound_(crypt_str("flick.wav"));
					break;
				case 3:
					m_surface()->PlaySound_(crypt_str("ding.wav"));
					break;
				case 4:
					m_surface()->PlaySound_(crypt_str("primordial.wav"));
					break;
				case 5:
					m_surface()->PlaySound_(crypt_str("magic.wav"));
					break;
				case 6:
					m_surface()->PlaySound_(crypt_str("bell.wav"));
					break;
				}
			}

			auto entity = static_cast<player_t*>(m_entitylist()->GetClientEntity(user_id));

			std::string weapon = event->GetString(crypt_str("weapon"));
			auto damage = event->GetInt(crypt_str("dmg_health"));
			auto hitgroup = event->GetInt(crypt_str("hitgroup"));

			static auto get_hitbox_by_hitgroup = [](int hitgroup) -> int
			{
				switch (hitgroup)
				{
				case HITGROUP_HEAD:
					return CSGOHitboxID::Head;
				case HITGROUP_CHEST:
					return CSGOHitboxID::Chest;
				case HITGROUP_STOMACH:
					return CSGOHitboxID::Stomach;
				case HITGROUP_LEFTARM:
					return CSGOHitboxID::LeftHand;
				case HITGROUP_RIGHTARM:
					return CSGOHitboxID::RightHand;
				case HITGROUP_LEFTLEG:
					return CSGOHitboxID::RightCalf;
				case HITGROUP_RIGHTLEG:
					return CSGOHitboxID::LeftCalf;
				default:
					return CSGOHitboxID::Pelvis;
				}
			};

			static auto get_hitbox_name = [](int hitbox, bool shot_info = false) -> std::string
			{
				switch (hitbox)
				{
				case CSGOHitboxID::Head:
					return shot_info ? crypt_str("Head") : crypt_str("head");
				case CSGOHitboxID::LowerChest:
					return shot_info ? crypt_str("Lower chest") : crypt_str("lower chest");
				case CSGOHitboxID::Chest:
					return shot_info ? crypt_str("Chest") : crypt_str("chest");
				case CSGOHitboxID::UpperChest:
					return shot_info ? crypt_str("Upper chest") : crypt_str("upper chest");
				case CSGOHitboxID::Stomach:
					return shot_info ? crypt_str("Stomach") : crypt_str("stomach");
				case CSGOHitboxID::Pelvis:
					return shot_info ? crypt_str("Pelvis") : crypt_str("pelvis");
				case CSGOHitboxID::RightUpperArm:
					return shot_info ? crypt_str("Right upperarm") : crypt_str("left arm");
				case CSGOHitboxID::RightLowerArm:
					return shot_info ? crypt_str("Right forearm") : crypt_str("left arm");
				case CSGOHitboxID::RightHand:
					return shot_info ? crypt_str("Right hand") : crypt_str("left arm");
				case CSGOHitboxID::LeftUpperArm:
					return shot_info ? crypt_str("Left upperarm") : crypt_str("right arm");
				case CSGOHitboxID::LeftLowerArm:
					return shot_info ? crypt_str("Left forearm") : crypt_str("right arm");
				case CSGOHitboxID::LeftHand:
					return shot_info ? crypt_str("Left hand") : crypt_str("right arm");
				case CSGOHitboxID::RightThigh:
					return shot_info ? crypt_str("Right thigh") : crypt_str("left leg");
				case CSGOHitboxID::RightCalf:
					return shot_info ? crypt_str("Right calf") : crypt_str("left leg");
				case CSGOHitboxID::LeftThigh:
					return shot_info ? crypt_str("Left thigh") : crypt_str("right leg");
				case CSGOHitboxID::LeftCalf:
					return shot_info ? crypt_str("Left calf") : crypt_str("right leg");
				case CSGOHitboxID::RightFoot:
					return shot_info ? crypt_str("Right foot") : crypt_str("left foot");
				case CSGOHitboxID::LeftFoot:
					return shot_info ? crypt_str("Left foot") : crypt_str("right foot");
				}
			};

			aim_shot* current_shot = nullptr;

			for (auto& shot : g_ctx.shots)
			{
				if (!shot.start)
					continue;

				if (shot.end)
					continue;

				current_shot = &shot;
				break;
			}

			if (weapon_is_aim(weapon))
			{
				otheresp::get().hitmarker.hurt_time = m_globals()->m_realtime + .5f;
				otheresp::get().hitmarker.hurt_time_world = m_globals()->m_realtime + 3.5f;
				otheresp::get().hitmarker.point = entity->hitbox_position_matrix(get_hitbox_by_hitgroup(hitgroup), current_shot && entity == Rbot::get().last_target[current_shot->last_target].record.player ? Rbot::get().last_target[current_shot->last_target].record.matrixes_data.main : entity->m_CachedBoneData().Base());
				otheresp::get().damage_marker[user_id] = otheresp::Damage_marker
				{
					entity->hitbox_position_matrix(get_hitbox_by_hitgroup(hitgroup), current_shot && entity == Rbot::get().last_target[current_shot->last_target].record.player ? Rbot::get().last_target[current_shot->last_target].record.matrixes_data.main : entity->m_CachedBoneData().Base()),
					m_globals()->m_curtime,
					Color::White,
					damage,
					hitgroup
				};

				playeresp::get().shot_capsule();
			}
			
			auto headshot = hitgroup == HITGROUP_HEAD;

			otheresp::get().hitmarker.hurt_color = Color::White;
			otheresp::get().damage_marker[user_id].hurt_color = headshot ? Color::Red : Color::White;

			if (current_shot)
			{
				current_shot->shot_info.result = crypt_str("Hit");
				current_shot->shot_info.server_hitbox = get_hitbox_name(get_hitbox_by_hitgroup(hitgroup));
				current_shot->shot_info.server_damage = damage;

				current_shot->end = true;
				current_shot->hurt_player = true;
			}
		}
	}
	else if (!strcmp(event_name, crypt_str("round_start")))
	{
		if (!cfg::g_cfg.misc.console_filter && eventlogs::get().last_log && cfg::g_cfg.misc.log_output)
		{
			eventlogs::get().last_log = false;
		}

		for (auto i = 1; i < m_globals()->m_maxclients; i++)
		{
			g_ctx.globals.fired_shots[i] = 0;
			g_ctx.globals.missed_shots[i] = 0;
			lagcompensation::get().is_dormant[i] = false;
			lagcompensation::get().CPlayerResolver[i].Reset();
			playeresp::get().esp_alpha_fade[i] = 0.0f;
			playeresp::get().health[i] = 100;
			c_dormant_esp::get().m_cSoundPlayers[i].reset();
		}
		
		antiaim::get().freeze_check = true;
		g_ctx.globals.bomb_timer_enable = true;
		g_ctx.globals.should_buy = 2;
		g_ctx.globals.should_clear_death_notices = true;
		g_ctx.globals.should_update_playerresource = true;
		g_ctx.globals.should_update_gamerules = true;
		g_ctx.globals.kills = 0;
		g_ctx.shots.clear();
	}
	else if (!strcmp(event_name, crypt_str("round_freeze_end")))
		antiaim::get().freeze_check = false;
	else if (!strcmp(event_name, crypt_str("bomb_defused")))
		g_ctx.globals.bomb_timer_enable = false;

	/*if ((cfg::g_cfg.visuals.bullet_tracer || cfg::g_cfg.visuals.enemy_bullet_tracer) && cfg::g_cfg.visuals.enable)
		bullettracers::get().events(event);*/

	eventlogs::get().events(event);
}

int C_HookedEvents::GetEventDebugID(void)
{
	return EVENT_DEBUG_ID_INIT;
}

void C_HookedEvents::RegisterSelf()
{
	m_iDebugId = EVENT_DEBUG_ID_INIT;
	auto eventmanager = m_eventmanager();

	eventmanager->AddListener(this, crypt_str("player_footstep"), false);
	eventmanager->AddListener(this, crypt_str("player_hurt"), false);
	eventmanager->AddListener(this, crypt_str("player_death"), false);
	eventmanager->AddListener(this, crypt_str("weapon_fire"), false);
	eventmanager->AddListener(this, crypt_str("item_purchase"), false);
	eventmanager->AddListener(this, crypt_str("bullet_impact"), false);
	eventmanager->AddListener(this, crypt_str("round_start"), false);
	eventmanager->AddListener(this, crypt_str("round_freeze_end"), false);
	eventmanager->AddListener(this, crypt_str("bomb_defused"), false);
	eventmanager->AddListener(this, crypt_str("bomb_begindefuse"), false);
	eventmanager->AddListener(this, crypt_str("bomb_beginplant"), false);

	g_ctx.globals.events.emplace_back(crypt_str("player_footstep"));
	g_ctx.globals.events.emplace_back(crypt_str("player_hurt"));
	g_ctx.globals.events.emplace_back(crypt_str("player_death"));
	g_ctx.globals.events.emplace_back(crypt_str("weapon_fire"));
	g_ctx.globals.events.emplace_back(crypt_str("item_purchase"));
	g_ctx.globals.events.emplace_back(crypt_str("bullet_impact"));
	g_ctx.globals.events.emplace_back(crypt_str("round_start"));
	g_ctx.globals.events.emplace_back(crypt_str("round_freeze_end"));
	g_ctx.globals.events.emplace_back(crypt_str("bomb_defused"));
	g_ctx.globals.events.emplace_back(crypt_str("bomb_begindefuse"));
	g_ctx.globals.events.emplace_back(crypt_str("bomb_beginplant"));
}

void C_HookedEvents::RemoveSelf()
{
	m_eventmanager()->RemoveListener(this);
}