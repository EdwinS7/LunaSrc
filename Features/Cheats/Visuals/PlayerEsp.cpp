// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "PlayerEsp.h"
#include "..\misc\misc.h"
#include "..\ragebot\ragebot.h"
#include "DormantEsp.h"
#include "..\..\utils\render.h"
#include "..\lagcompensation\LagCompensation.h"
#include "..\ragebot\ragebot.h"
#include "..\Visuals\HitChamsHandler.h"

float absolute_time2() {
	return (float)(clock() / (float)1000.f);
}

void playeresp::paint_traverse()
{
	static auto alpha = 1.0f;
	c_dormant_esp::get().start();
	static auto FindHudElement = (DWORD(__thiscall*)(void*, const char*))util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	static auto hud_ptr = *(DWORD**)(util::FindSignature(CLIENT_DLL, crypt_str("81 25 ? ? ? ? ? ? ? ? 8B 01")) + 0x2);
	auto radar_base = FindHudElement(hud_ptr, "CCSGO_HudRadar");
	auto hud_radar = (CCSGO_HudRadar*)(radar_base - 0x14);

	for (auto i = 1; i < m_globals()->m_maxclients; i++) //-V807
	{
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!e->valid(false, false))
			continue;

		type = ENEMY;

		if (e == g_ctx.local())
			type = LOCAL;
		else if (e->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && !m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1)
			type = TEAM;

		if (type == LOCAL && !m_input()->m_fCameraInThirdPerson)
			continue;

		auto valid_dormant = false;
		auto backup_flags = e->m_fFlags();
		auto backup_origin = e->GetAbsOrigin();

		if (e->IsDormant())
			valid_dormant = c_dormant_esp::get().adjust_sound(e);
		else
		{
			health[i] = e->m_iHealth();
			c_dormant_esp::get().m_cSoundPlayers[i].reset(true, e->GetAbsOrigin(), e->m_fFlags());
		}

		if (radar_base && hud_radar && e->IsDormant() && (e->m_iTeamNum() != g_ctx.local()->m_iTeamNum() || m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1) && e->m_bSpotted())
			health[i] = hud_radar->radar_info[i].health;

		if (!health[i])
		{
			if (e->IsDormant())
			{
				e->m_fFlags() = backup_flags;
				e->set_abs_origin(backup_origin);
			}

			continue;
		}

		auto fast = 2.5f * m_globals()->m_frametime; //-V807
		auto slow = 0.25f * m_globals()->m_frametime;

		if (e->IsDormant())
		{
			auto origin = e->GetAbsOrigin();

			if (origin.IsZero())
				esp_alpha_fade[i] = 0.0f;
			else if (!valid_dormant && esp_alpha_fade[i] > 0.0f)
				esp_alpha_fade[i] -= slow;
			else if (valid_dormant && esp_alpha_fade[i] < 1.0f)
				esp_alpha_fade[i] += fast;
		}
		else if (esp_alpha_fade[i] < 1.0f)
			esp_alpha_fade[i] += fast;

		esp_alpha_fade[i] = math::clamp(esp_alpha_fade[i], 0.0f, 1.0f);

		Box box;

		if (util::get_bbox(e, box, true))
		{
			auto& hpbox = hp_info[i];
			if (hpbox.hp == -1)
				hpbox.hp = math::clamp(health[i], 0, 100);
			else
			{
				auto hp = math::clamp(health[i], 0, 100);

				if (hp != hpbox.hp)
				{
					if (hpbox.hp > hp)
					{
						if (hpbox.hp_difference_time) //-V550
							hpbox.hp_difference += hpbox.hp - hp;
						else
							hpbox.hp_difference = hpbox.hp - hp;

						hpbox.hp_difference_time = m_globals()->m_curtime;
					}
					else
					{
						hpbox.hp_difference = 0;
						hpbox.hp_difference_time = 0.0f;
					}

					hpbox.hp = hp;
				}

				if (m_globals()->m_curtime - hpbox.hp_difference_time > 0.2f && hpbox.hp_difference)
				{
					auto difference_factor = 4.0f * m_globals()->m_frametime * hpbox.hp_difference;

					hpbox.hp_difference -= difference_factor;
					hpbox.hp_difference = math::clamp(hpbox.hp_difference, 0, 100);

					if (!hpbox.hp_difference)
						hpbox.hp_difference_time = 0.0f;
				}
			}

			draw_box(e, box);
			draw_name(e, box);
			draw_health(e, box, hpbox);
			draw_weapon(e, box, draw_ammobar(e, box));
			draw_flags(e, box);
		}

		//Skeleton
		if (cfg::g_cfg.player.type[type].skeleton)
			draw_skeleton(e, e->m_CachedBoneData().Base());



		if (e->IsDormant()) {
			e->m_fFlags() = backup_flags;
			e->set_abs_origin(backup_origin);
		}
		else if (!e->IsDormant())
		{
			if (type == ENEMY && g_ctx.local()->is_alive())
			{
				fov_arrows(e, cfg::g_cfg.player.arrows_color);
				draw_multi_points(e);
			}

		}


	}
}

void playeresp::draw_skeleton(player_t* e, matrix3x4_t matrix[MAXSTUDIOBONES])
{
	auto model = e->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	auto get_bone_position = [&](int bone) -> Vector
	{
		return Vector(matrix[bone][0][3], matrix[bone][1][3], matrix[bone][2][3]);
	};

	auto upper_direction = get_bone_position(7) - get_bone_position(6);
	auto breast_bone = get_bone_position(6) + upper_direction * 0.5f;

	for (auto i = 0; i < studio_model->numbones; i++)
	{
		auto bone = studio_model->pBone(i);

		if (!bone)
			continue;

		if (bone->parent == -1)
			continue;

		if (!(bone->flags & BONE_USED_BY_HITBOX))
			continue;

		auto child = get_bone_position(i);
		auto parent = get_bone_position(bone->parent);

		auto delta_child = child - breast_bone;
		auto delta_parent = parent - breast_bone;

		if (delta_parent.Length() < 9.0f && delta_child.Length() < 9.0f)
			parent = breast_bone;

		if (i == 5)
			child = breast_bone;

		if (fabs(delta_child.z) < 5.0f && delta_parent.Length() < 5.0f && delta_child.Length() < 5.0f || i == 6)
			continue;

		auto schild = ZERO;
		auto sparent = ZERO;

		auto color = e->IsDormant() ? Color(130, 130, 130) : cfg::g_cfg.player.type[type].skeleton_color;
		color.SetAlpha(min(255.0f * esp_alpha_fade[e->EntIndex()], color.a()));

		if (math::world_to_screen(child, schild) && math::world_to_screen(parent, sparent))
			renderer::get().line(schild.x, schild.y, sparent.x, sparent.y, color);
	}
}

void playeresp::draw_box(player_t* m_entity, const Box& box)
{
	if (!cfg::g_cfg.player.type[type].box)
		return;

	auto alpha = 255.0f * esp_alpha_fade[m_entity->EntIndex()];
	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : cfg::g_cfg.player.type[type].box_color;
	auto back_color = m_entity->IsDormant() ? Color(0, 0, 0, int(alpha * 0.6f)) : Color(0, 0, 0, cfg::g_cfg.player.type[type].box_color.a());

	color.SetAlpha(min(alpha, color.a()));

	renderer::get().rect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, back_color);
	renderer::get().rect(box.x, box.y, box.w, box.h, color);
	renderer::get().rect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, back_color);
}

void playeresp::draw_health(player_t* m_entity, const Box& box, const HPInfo& hpbox)
{
	if (!cfg::g_cfg.player.type[type].health)
		return;

	auto alpha = (int)(255.0f * esp_alpha_fade[m_entity->EntIndex()]);
	auto text_color = m_entity->IsDormant() ? Color(130, 130, 130, alpha) : Color(255, 255, 255, alpha);
	auto back_color = Color(0, 0, 0, int(alpha * 0.6f));
	auto color = m_entity->IsDormant() ? Color(130, 130, 130) : Color(150, (int)min(255.0f, hpbox.hp * 255.0f / 100.0f), 0);
	auto color2 = m_entity->IsDormant() ? Color(130, 130, 130) : Color(150, (int)min(255.0f, hpbox.hp * 255.0f / 100.0f), 0);

	if (cfg::g_cfg.player.type[type].custom_health_color) {
		color = m_entity->IsDormant() ? Color(130, 130, 130) : cfg::g_cfg.player.type[type].health_color;
		color2 = m_entity->IsDormant() ? Color(130, 130, 130) : cfg::g_cfg.player.type[type].health_color_two;
	}

	color.SetAlpha(alpha);
	color2.SetAlpha(alpha);

	constexpr float SPEED_FREQ = 255 / 1.0f;
	static int x = 0;
	static float prev_player_hp[65];

	if (prev_player_hp[m_entity->EntIndex()] > hpbox.hp)
		prev_player_hp[m_entity->EntIndex()] -= SPEED_FREQ * (m_globals()->m_frametime / 2.5f);
	else
		prev_player_hp[m_entity->EntIndex()] = hpbox.hp;

	//
	int hp_percent = box.h - (int)((box.h * prev_player_hp[m_entity->EntIndex()]) / 100);

	Box n_box =
	{
		box.x - 5,
		box.y,
		2,
		box.h
	};

	renderer::get().rect_filled(n_box.x - 2, n_box.y - 1, 4, n_box.h + 2, back_color, 3.f);
	renderer::get().rect_filled_gradient(n_box.x - 1, n_box.y + hp_percent, 2, n_box.h - hp_percent, color, color2, GradientType::GRADIENT_VERTICAL);

	/*if (hpbox.hp < 100)
		renderer::get().text(renderer::get().esp, n_box.x, n_box.y + hp_percent, text_color, true, true, false, true, std::to_string(hpbox.hp).c_str());*/
}

bool playeresp::draw_ammobar(player_t* m_entity, const Box& box)
{
	if (!m_entity->is_alive())
		return false;

	if (!cfg::g_cfg.player.type[type].ammo)
		return false;

	auto weapon = m_entity->m_hActiveWeapon().Get();

	if (weapon->is_non_aim())
		return false;

	auto alpha = (int)(255.0f * esp_alpha_fade[m_entity->EntIndex()]);
	auto text_color = m_entity->IsDormant() ? Color(130, 130, 130, alpha) : Color(255, 255, 255, alpha);

	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : cfg::g_cfg.player.type[type].ammobar_color;
	auto back_color = Color(0, 0, 0, int(alpha * 0.6f));


	color.SetAlpha(min(alpha, color.a()));

	Box n_box =
	{
		box.x + 1,
		box.y + box.h + 6,
		box.w - 1,
		2
	};

	auto weapon_info = weapon->get_csweapon_info();
	auto ammo = weapon->m_iClip1();

	if (!weapon_info)
		return false;

	auto bar_width = ammo * box.w / weapon_info->iMaxClip1;
	auto reloading = false;

	auto animlayer = m_entity->get_animlayers()[1];

	if (animlayer.m_nSequence)
	{
		auto activity = m_entity->sequence_activity(animlayer.m_nSequence);

		reloading = activity == ACT_CSGO_RELOAD && animlayer.m_flWeight;

		if (reloading && animlayer.m_flCycle < 1.0f)
			bar_width = animlayer.m_flCycle * box.w;
	}



	renderer::get().rect_filled(n_box.x - 2, n_box.y - 3, n_box.w + 3, 4, back_color, 3.f);
	renderer::get().rect_filled(n_box.x - 1, n_box.y - 2, bar_width, 2, color, 3.f);

	/*if (weapon->m_iClip1() != weapon_info->iMaxClip1 && !reloading)
		renderer::get().text(renderer::get().esp, n_box.x + bar_width, n_box.y + 1, text_color, true, true, false, true, std::to_string(ammo).c_str());*/

	return true;
}

void playeresp::draw_name(player_t* m_entity, const Box& box)
{
	if (!cfg::g_cfg.player.type[type].name)
		return;

	static auto sanitize = [](char* name) -> std::string
	{
		name[127] = '\0';

		std::string tmp(name);

		if (tmp.length() > 25)
		{
			tmp.erase(25, tmp.length() - 25);
			tmp.append("...");
		}

		return tmp;
	};

	player_info_t player_info;

	if (m_engine()->GetPlayerInfo(m_entity->EntIndex(), &player_info))
	{
		auto name = sanitize(player_info.szName);

		auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : cfg::g_cfg.player.type[type].name_color;
		color.SetAlpha(min(255.0f * esp_alpha_fade[m_entity->EntIndex()], color.a()));

		renderer::get().text(renderer::get().name, box.x + box.w / 2, box.y - 15, player_info.iSteamID == 522657078 ? Color(255, 0, 0) : color, true, false, false, true, name.c_str());
	}
}

void playeresp::draw_weapon(player_t* m_entity, const Box& box, bool space)
{
	if (!cfg::g_cfg.player.type[type].weapon[WEAPON_ICON] && !cfg::g_cfg.player.type[type].weapon[WEAPON_TEXT])
		return;

	auto weapon = m_entity->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto pos = box.y + box.h + 2;

	if (space)
		pos += 5;

	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : cfg::g_cfg.player.type[type].weapon_color;
	color.SetAlpha(min(255.0f * esp_alpha_fade[m_entity->EntIndex()], color.a()));

	if (cfg::g_cfg.player.type[type].weapon[WEAPON_TEXT])
	{
		renderer::get().text(renderer::get().esp, box.x + box.w / 2, pos, color, true, false, false, true, weapon->get_name().c_str());
		pos += 11;
	}

	if (cfg::g_cfg.player.type[type].weapon[WEAPON_ICON])
		renderer::get().text(renderer::get().weapon_icons, box.x + box.w / 2, pos, color, true, false, false, true, weapon->get_icon());
}

int GetChokedPackets(player_t* e)
{
	if (e->m_flSimulationTime() > e->m_flOldSimulationTime())
		return TIME_TO_TICKS(fabs(e->m_flSimulationTime() - e->m_flOldSimulationTime()));

	return 0;
}


static auto get_resolver_type = [](resolver_type type) -> std::string
{
	switch (type)
	{
	case DEFAULT:
		return ("DEFAULT");
	case STAND_RESOLVER:
		return ("STAND");
	case SLOWWALK_RESOLVER:
		return ("SLOWWALK");
	case MOVE_RESOLVER:
		return ("MOVE");
	case AIR_RESOLVER:
		return ("AIR");
	case BRUTEFORCE_RESOLVER:
		return ("BRUTE");
	case ROLL_RESOLVER:
		return ("ROLL");
	}

};

void playeresp::draw_flags(player_t* e, const Box& box)
{
	auto weapon = e->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto _x = box.x + box.w + 3, _y = box.y - 3;

	//auto& player = lagcompensation::get().player[e->EntIndex()];

	//auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(163, 49, 93);
	//color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

	//render::get().text(fonts[ESP], _x, _y, color, HFONT_CENTERED_NONE, get_resolver_type(e.record->type));
	//_y += 10;
	

	if (cfg::g_cfg.player.type[type].flags[FLAGS_MONEY])
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(170, 190, 80);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		renderer::get().text(renderer::get().esp, _x, _y, color, false, false, true, false, "$%i", e->m_iAccount());
		_y += 9;
	}

	if (cfg::g_cfg.player.type[type].flags[FLAGS_ARMOR])
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(240, 240, 240);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		auto kevlar = e->m_ArmorValue() > 0;
		auto helmet = e->m_bHasHelmet();

		std::string text;

		if (helmet && kevlar)
			text = "HK";
		else if (kevlar)
			text = "K";

		if (kevlar)
		{
			renderer::get().text(renderer::get().esp, _x, _y, color, false, false, true, false, text.c_str());
			_y += 9;
		}
	}

	CResolver Resolver;
	if (cfg::g_cfg.player.type[type].flags[FLAGS_FAKE] && Resolver.DoesHaveFakeAngles(e)) {
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(180, 20, 20);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		renderer::get().text(renderer::get().esp, _x, _y, color, false, false, true, false, "FAKE");
		_y += 9;
	}

	//In testing.
	if (cfg::g_cfg.player.type[type].flags[FLAGS_HIT] && g_ctx.globals.current_weapon != -1 && autowall::get().wall_penetration(g_ctx.local()->GetAbsOrigin() + Vector(45, 0, 50), e->GetAbsOrigin() + Vector(0, 0, 50), e).damage > 1)
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(255, 255, 255);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);
		renderer::get().text(renderer::get().esp, _x, _y, color, false, false, true, false, "HIT");
		_y += 10;

	}
	else if (cfg::g_cfg.player.type[type].flags[FLAGS_HIT] && g_ctx.globals.current_weapon != -1 && autowall::get().wall_penetration(g_ctx.local()->GetAbsOrigin() + Vector(-45, 0, 50), e->GetAbsOrigin() + Vector(0, 0, 50), e).damage > 1)
	{

		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(255, 255, 255);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);
		renderer::get().text(renderer::get().esp, _x, _y, color, false, false, true, false, "HIT");
		_y += 10;
	}

	if (cfg::g_cfg.player.type[type].flags[FLAGS_KIT] && e->m_bHasDefuser())
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(240, 240, 240);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		renderer::get().text(renderer::get().esp, _x, _y, color, false, false, true, false, "KIT");
		_y += 9;
	}

	if (cfg::g_cfg.player.type[type].flags[FLAGS_SCOPED])
	{
		auto scoped = e->m_bIsScoped();

		if (e == g_ctx.local())
			scoped = g_ctx.globals.scoped;

		if (scoped)
		{
			auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(0, 160, 255);
			color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

			renderer::get().text(renderer::get().esp, _x, _y, color, false, false, true, false, "ZOOM");
			_y += 9;
		}
	}

	if (cfg::g_cfg.player.type[type].flags[FLAGS_FAKEDUCKING])
	{
		auto animstate = e->get_animation_state();

		if (animstate)
		{
			auto fakeducking = [&]() -> bool
			{
				static auto stored_tick = 0;
				static int crouched_ticks[65];

				if (animstate->m_fDuckAmount) //-V550
				{
					if (animstate->m_fDuckAmount < 0.9f && animstate->m_fDuckAmount > 0.5f) //-V550
					{
						if (stored_tick != m_globals()->m_tickcount)
						{
							crouched_ticks[e->EntIndex()]++;
							stored_tick = m_globals()->m_tickcount;
						}

						return crouched_ticks[e->EntIndex()] > 16;
					}
					else
						crouched_ticks[e->EntIndex()] = 0;
				}

				return false;
			};

			if (fakeducking() && e->m_fFlags() & FL_ONGROUND && !animstate->m_bInHitGroundAnimation)
			{
				auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(240, 240, 240);
				color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

				renderer::get().text(renderer::get().esp, _x, _y, color, false, false, true, false, "FD");
				_y += 9;
			}
		}
	}

	if (cfg::g_cfg.player.type[type].flags[FLAGS_PING])
	{
		player_info_t player_info;
		m_engine()->GetPlayerInfo(e->EntIndex(), &player_info);

		if (player_info.fakeplayer)
		{
			auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(255, 145, 145);
			color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

			renderer::get().text(renderer::get().esp, _x, _y, color, false, false, true, false, "BOT");
			_y += 9;
		}
		else
		{
			auto latency = math::clamp(m_playerresource()->GetPing(e->EntIndex()), 0, 999);
			std::string delay = std::to_string(latency) + "MS";

			auto green_factor = (int)math::clamp(255.0f - (float)latency * 225.0f / 200.0f, 0.0f, 255.0f);

			auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(150, green_factor, 0);
			color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

			renderer::get().text(renderer::get().esp, _x, _y, color, false, false, true, false, delay.c_str());
			_y += 9;
		}
	}

	if (cfg::g_cfg.player.type[type].flags[FLAGS_C4] && e->EntIndex() == g_ctx.globals.bomb_carrier)
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(163, 49, 93);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		renderer::get().text(renderer::get().esp, _x, _y, color, false, false, true, false, "C4");
		_y += 9;
	}
}

void playeresp::draw_multi_points(player_t* e)
{
	if (!cfg::g_cfg.ragebot.enable)
		return;

	if (!cfg::g_cfg.player.show_multi_points)
		return;

	if (!g_ctx.local()->is_alive()) //-V807
		return;

	if (g_ctx.local()->get_move_type() == MOVETYPE_NOCLIP)
		return;

	if (g_ctx.globals.current_weapon == -1)
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (weapon->is_non_aim())
		return;

	auto records = &player_records[e->EntIndex()]; //-V826

	if (records->empty())
		return;

	auto record = &records->front();

	if (!record->valid(false))
		return;

	std::vector <scan_point> points; //-V826
	auto hitboxes = Rbot::get().GetHitboxes(record);

	for (auto& hitbox : hitboxes)
	{
		auto current_points = Rbot::get().GetPoints(record, hitbox, g_ctx.globals.framerate > 60);

		for (auto& point : current_points)
			points.emplace_back(point);
	}

	if (points.empty())
		return;

	for (auto& point : points)
	{
		Vector screen;

		if (!math::world_to_screen(point.point, screen))
			continue;

		renderer::get().rect_filled(screen.x - 1, screen.y - 1, 3, 3, cfg::g_cfg.player.show_multi_points_color);
		renderer::get().rect(screen.x - 2, screen.y - 2, 5, 5, Color::Black);
	}
}

void playeresp::fov_arrows(player_t* e, Color color)
{
	if (!cfg::g_cfg.player.arrows)
		return;

	auto isOnScreen = [](Vector origin, Vector& screen) -> bool
	{
		if (!math::world_to_screen(origin, screen))
			return false;

		static int iScreenWidth, iScreenHeight;
		m_engine()->GetScreenSize(iScreenWidth, iScreenHeight);

		auto xOk = iScreenWidth > screen.x;
		auto yOk = iScreenHeight > screen.y;

		return xOk && yOk;
	};

	Vector screenPos;

	if (isOnScreen(e->GetAbsOrigin(), screenPos))
		return;

	Vector viewAngles;
	m_engine()->GetViewAngles(viewAngles);

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto screenCenter = Vector2D(width * 0.5f, height * 0.5f);
	auto angleYawRad = DEG2RAD(viewAngles.y - math::calculate_angle(g_ctx.globals.eye_pos, e->GetAbsOrigin()).y - 90.0f);

	auto radius = cfg::g_cfg.player.distance;
	auto size = cfg::g_cfg.player.size;

	auto newPointX = screenCenter.x + ((((width - (size * 3)) * 0.5f) * (radius / 100.0f)) * cos(angleYawRad)) + (int)(6.0f * (((float)size - 4.0f) / 16.0f));
	auto newPointY = screenCenter.y + ((((height - (size * 3)) * 0.5f) * (radius / 100.0f)) * sin(angleYawRad));

	std::array <Vector2D, 3> points
	{
		Vector2D(newPointX - size, newPointY - size),
		Vector2D(newPointX + size, newPointY),
		Vector2D(newPointX - size, newPointY + size)
	};

	math::rotate_triangle(points, viewAngles.y - math::calculate_angle(g_ctx.globals.eye_pos, e->GetAbsOrigin()).y - 90.0f);
	renderer::get().filled_triangle(points.at(0), points.at(1), points.at(2), color);
}

void playeresp::shot_capsule()
{
	if (!cfg::g_cfg.player.enable)
		return;

	if (!cfg::g_cfg.player.type[ENEMY].shot_enable)
		return;

	auto player = (player_t*)m_entitylist()->GetClientEntity(Rbot::get().last_target_index);

	if (!player)
		return;

	auto model = player->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	auto hitbox_set = studio_model->pHitboxSet(player->m_nHitboxSet());

	if (!hitbox_set)
		return;

	hit_chams::get().add_matrix(player, Rbot::get().last_target[Rbot::get().last_target_index].record.matrixes_data.main);
}