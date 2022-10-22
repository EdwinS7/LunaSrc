// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "WorldEsp.h"
#include "GrenadeWarning.h"
#include "..\..\utils\render.h"

void worldesp::paint_traverse()
{
	skybox_changer();
	draw_client_impacts();

	for (int i = 1; i <= m_entitylist()->GetHighestEntityIndex(); i++)  //-V807
	{
		auto e = static_cast<entity_t*>(m_entitylist()->GetClientEntity(i));

		if (!e)
			continue;

		if (e->is_player())
			continue;

		if (e->IsDormant())
			continue;

		auto client_class = e->GetClientClass();

		if (!client_class)
			continue;

		switch (client_class->m_ClassID)
		{
		case CEnvTonemapController:
			world_modulation(e);
			break;
		case CInferno:
			molotov_timer(e);
			break;
		case CSmokeGrenadeProjectile:
			smoke_timer(e);
			break;
		case CPlantedC4:
			bomb_timer(e);
			break;
		case CC4:
			if (cfg::g_cfg.player.type[ENEMY].flags[FLAGS_C4] || cfg::g_cfg.player.type[TEAM].flags[FLAGS_C4] || cfg::g_cfg.player.type[LOCAL].flags[FLAGS_C4] || cfg::g_cfg.visuals.bomb_timer)
			{
				auto owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(e->m_hOwnerEntity());

				if ((cfg::g_cfg.player.type[ENEMY].flags[FLAGS_C4] || cfg::g_cfg.player.type[TEAM].flags[FLAGS_C4] || cfg::g_cfg.player.type[LOCAL].flags[FLAGS_C4]) && owner->valid(false, false))
					g_ctx.globals.bomb_carrier = owner->EntIndex();
				else if (cfg::g_cfg.visuals.bomb_timer && !owner->is_player())
				{
					auto screen = ZERO;

					if (math::world_to_screen(e->GetAbsOrigin(), screen))
						renderer::get().text(renderer::get().world, screen.x, screen.y, Color(215, 20, 20), true, true, false, true, "C4");
				}
			}

			break;
		default:
			grenade_projectiles(e);
			c_grenade_warning::get().grenade_warning(e);

			if (client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponZoneRepulsor)
				dropped_weapons(e);

			break;
		}
	}
}

void worldesp::draw_client_impacts()
{
	if (!cfg::g_cfg.visuals.client_bullet_impacts)
		return;

	auto& aClientImpactList = *(CUtlVector< client_hit_verify_t >*)((uintptr_t)(g_ctx.local()) + 0x11C50);
	for (auto Impact = aClientImpactList.Count(); Impact > m_iLastProcessedImpact; --Impact)
		m_debugoverlay()->BoxOverlay(
			aClientImpactList[Impact - 1].position,
			Vector(-cfg::g_cfg.visuals.bullet_impacts_size, -cfg::g_cfg.visuals.bullet_impacts_size, -cfg::g_cfg.visuals.bullet_impacts_size),
			Vector(cfg::g_cfg.visuals.bullet_impacts_size, cfg::g_cfg.visuals.bullet_impacts_size, cfg::g_cfg.visuals.bullet_impacts_size),
			QAngle(0.0f, 0.0f, 0.0f),
			cfg::g_cfg.visuals.client_bullet_impacts_color.r(),
			cfg::g_cfg.visuals.client_bullet_impacts_color.g(),
			cfg::g_cfg.visuals.client_bullet_impacts_color.b(),
			cfg::g_cfg.visuals.client_bullet_impacts_color.a(),
			4.0f);

	if (aClientImpactList.Count() != m_iLastProcessedImpact)
		m_iLastProcessedImpact = aClientImpactList.Count();
}

void worldesp::skybox_changer()
{
	static auto load_skybox = reinterpret_cast<void(__fastcall*)(const char*)>(util::FindSignature(ENGINE_DLL, crypt_str("55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45")));
	auto skybox_name = backup_skybox;

	switch (cfg::g_cfg.visuals.skybox)
	{
	case 1:
		skybox_name = "cs_tibet";
		break;
	case 2:
		skybox_name = "cs_baggage_skybox_";
		break;
	case 3:
		skybox_name = "italy";
		break;
	case 4:
		skybox_name = "jungle";
		break;
	case 5:
		skybox_name = "office";
		break;
	case 6:
		skybox_name = "sky_cs15_daylight01_hdr";
		break;
	case 7:
		skybox_name = "sky_cs15_daylight02_hdr";
		break;
	case 8:
		skybox_name = "vertigoblue_hdr";
		break;
	case 9:
		skybox_name = "vertigo";
		break;
	case 10:
		skybox_name = "sky_day02_05_hdr";
		break;
	case 11:
		skybox_name = "nukeblank";
		break;
	case 12:
		skybox_name = "sky_venice";
		break;
	case 13:
		skybox_name = "sky_cs15_daylight03_hdr";
		break;
	case 14:
		skybox_name = "sky_cs15_daylight04_hdr";
		break;
	case 15:
		skybox_name = "sky_csgo_cloudy01";
		break;
	case 16:
		skybox_name = "sky_csgo_night02";
		break;
	case 17:
		skybox_name = "sky_csgo_night02b";
		break;
	case 18:
		skybox_name = "sky_csgo_night_flat";
		break;
	case 19:
		skybox_name = "sky_dust";
		break;
	case 20:
		skybox_name = "vietnam";
		break;
	case 21:
		skybox_name = cfg::g_cfg.visuals.custom_skybox;
		break;
	}

	static auto skybox_number = 0;
	static auto old_skybox_name = skybox_name;

	static auto color_r = (unsigned char)255;
	static auto color_g = (unsigned char)255;
	static auto color_b = (unsigned char)255;

	if (skybox_number != cfg::g_cfg.visuals.skybox)
	{
		changed = true;
		skybox_number = cfg::g_cfg.visuals.skybox;
	}
	else if (old_skybox_name != skybox_name)
	{
		changed = true;
		old_skybox_name = skybox_name;
	}
	else if (color_r != cfg::g_cfg.visuals.skybox_color[0])
	{
		changed = true;
		color_r = cfg::g_cfg.visuals.skybox_color[0];
	}
	else if (color_g != cfg::g_cfg.visuals.skybox_color[1])
	{
		changed = true;
		color_g = cfg::g_cfg.visuals.skybox_color[1];
	}
	else if (color_b != cfg::g_cfg.visuals.skybox_color[2])
	{
		changed = true;
		color_b = cfg::g_cfg.visuals.skybox_color[2];
	}

	if (changed)
	{
		changed = false;
		load_skybox(skybox_name.c_str());

		auto materialsystem = m_materialsystem();

		for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
		{
			auto material = materialsystem->GetMaterial(i);

			if (!material)
				continue;

			if (strstr(material->GetTextureGroupName(), crypt_str("SkyBox")))
				material->ColorModulate(cfg::g_cfg.visuals.skybox_color[0] / 255.0f, cfg::g_cfg.visuals.skybox_color[1] / 255.0f, cfg::g_cfg.visuals.skybox_color[2] / 255.0f);
		}
	}
}

void worldesp::fog_changer()
{
	static auto fog_override = m_cvar()->FindVar(crypt_str("fog_override")); //-V807

	if (!cfg::g_cfg.visuals.fog)
	{
		if (fog_override->GetBool())
			fog_override->SetValue(FALSE);

		return;
	}

	if (!fog_override->GetBool())
		fog_override->SetValue(TRUE);

	static auto fog_start = m_cvar()->FindVar(crypt_str("fog_start"));

	if (fog_start->GetInt())
		fog_start->SetValue(0);

	static auto fog_end = m_cvar()->FindVar(crypt_str("fog_end"));

	if (fog_end->GetInt() != cfg::g_cfg.visuals.fog_distance)
		fog_end->SetValue(cfg::g_cfg.visuals.fog_distance);

	static auto fog_maxdensity = m_cvar()->FindVar(crypt_str("fog_maxdensity"));

	if (fog_maxdensity->GetFloat() != (float)cfg::g_cfg.visuals.fog_density * 0.01f) //-V550
		fog_maxdensity->SetValue((float)cfg::g_cfg.visuals.fog_density * 0.01f);

	char buffer_color[12];
	sprintf_s(buffer_color, 12, "%i %i %i", cfg::g_cfg.visuals.fog_color.r(), cfg::g_cfg.visuals.fog_color.g(), cfg::g_cfg.visuals.fog_color.b());

	static auto fog_color = m_cvar()->FindVar(crypt_str("fog_color"));

	if (strcmp(fog_color->GetString(), buffer_color)) //-V526
		fog_color->SetValue(buffer_color);
}

void worldesp::world_modulation(entity_t* entity)
{
	if (!cfg::g_cfg.visuals.world_modulation)
		return;

	entity->set_m_bUseCustomBloomScale(TRUE);
	entity->set_m_flCustomBloomScale(cfg::g_cfg.visuals.bloom * 0.01f);

	entity->set_m_bUseCustomAutoExposureMin(TRUE);
	entity->set_m_flCustomAutoExposureMin(cfg::g_cfg.visuals.exposure * 0.001f);

	entity->set_m_bUseCustomAutoExposureMax(TRUE);
	entity->set_m_flCustomAutoExposureMax(cfg::g_cfg.visuals.exposure * 0.001f);
}

void worldesp::molotov_timer(entity_t* entity)
{
	if (!cfg::g_cfg.visuals.molotov_timer)
		return;

	auto inferno = reinterpret_cast<inferno_t*>(entity);

	auto origin = inferno->GetAbsOrigin();

	Vector screen_origin;

	if (!math::world_to_screen(origin, screen_origin))
		return;

	auto spawn_time = inferno->get_spawn_time();

	auto factor = (spawn_time + inferno_t::get_expiry_time() - m_globals()->m_curtime) / inferno_t::get_expiry_time();

	static const auto global_size = Vector2D(35.0f, 5.0f);

	auto color = cfg::g_cfg.visuals.molotov_timer_color;

	auto distance = g_ctx.local()->m_vecOrigin().DistTo(origin) / 12;

	auto alpha_damage = 0.f;

	if (distance <= 20)
		alpha_damage = 255 - 255 * (distance / 20);

	renderer::get().ring3d(origin.x, origin.y, origin.z, 90, 256, Color(color.r(), color.g(), color.b(), 255), Color(140, 140, 140, 35), 2, factor);

	renderer::get().circle_filled(screen_origin.x, screen_origin.y - global_size.y * 0.5f - 12, 21, 64, Color(25, 25, 25, color.a()));

	renderer::get().circle_filled(screen_origin.x, screen_origin.y - global_size.y * 0.5f - 12, 21, 64, Color(200, 25, 25, (int)alpha_damage));

	renderer::get().sided_arc(screen_origin.x, screen_origin.y - global_size.y * 0.5f - 12, 20, factor, Color(color.r(), color.g(), color.b(), 255), 3);



	renderer::get().text(renderer::get().grenades, screen_origin.x, screen_origin.y - global_size.y * 0.5f - 12, Color::White, true, true, false, true, "l");
}

void worldesp::smoke_timer(entity_t* entity)
{
	if (!cfg::g_cfg.visuals.smoke_timer)
		return;

	auto smoke = reinterpret_cast<smoke_t*>(entity);

	auto origin = smoke->GetAbsOrigin();

	if (!smoke->m_nSmokeEffectTickBegin() || !smoke->m_bDidSmokeEffect())
		return;

	Vector screen_origin;

	if (!math::world_to_screen(origin, screen_origin))
		return;

	auto spawn_time = TICKS_TO_TIME(smoke->m_nSmokeEffectTickBegin());

	auto factor = (spawn_time + smoke_t::get_expiry_time() - m_globals()->m_curtime) / smoke_t::get_expiry_time();

	static const auto global_size = Vector2D(35.0f, 5.0f);

	auto color = cfg::g_cfg.visuals.smoke_timer_color;

	renderer::get().ring3d(origin.x, origin.y, origin.z, 144, 256, Color(color.r(), color.g(), color.b(), 255), Color(140, 140, 140, 35), 2, factor);

	renderer::get().circle_filled(screen_origin.x, screen_origin.y - global_size.y * 0.5f - 12, 21, 64, Color(25, 25, 25, color.a()));
	renderer::get().sided_arc(screen_origin.x, screen_origin.y - global_size.y * 0.5f - 12, 20, factor, Color(color.r(), color.g(), color.b(), 255), 3);
	renderer::get().text(renderer::get().grenades, screen_origin.x, screen_origin.y - global_size.y * 0.5f - 12, Color::White, true, true, false, true, "k");
}

void worldesp::grenade_projectiles(entity_t* entity)
{
	if (!cfg::g_cfg.visuals.projectiles)
		return;

	auto client_class = entity->GetClientClass();

	if (!client_class)
		return;

	auto model = entity->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	auto name = studio_model->szName;

	if (strstr(name, "thrown") != NULL ||
		client_class->m_ClassID == CBaseCSGrenadeProjectile || client_class->m_ClassID == CDecoyProjectile || client_class->m_ClassID == CMolotovProjectile || client_class->m_ClassID == CSmokeGrenadeProjectile)
	{
		auto grenade_origin = entity->GetAbsOrigin();
		auto grenade_position = ZERO;

		if (!math::world_to_screen(grenade_origin, grenade_position))
			return;

		std::string grenade_name, grenade_icon;

		if (strstr(name, "flashbang") != NULL)
		{
			grenade_name = "Flashbang";
			grenade_icon = "i";
		}
		else if (strstr(name, "smokegrenade") != NULL)
		{
			grenade_name = "Smoke";
			grenade_icon = "k";
		}
		else if (strstr(name, "incendiarygrenade") != NULL)
		{
			grenade_name = "Incendiary";
			grenade_icon = "n";
		}
		else if (strstr(name, "molotov") != NULL)
		{
			grenade_name = "Molotov";
			grenade_icon = "l";
		}
		else if (strstr(name, "fraggrenade") != NULL)
		{
			grenade_name = "He grenade";
			grenade_icon = "j";
		}
		else if (strstr(name, "decoy") != NULL)
		{
			grenade_name = "Decoy";
			grenade_icon = "m";
		}
		else
			return;

		Box box;

		if (util::get_bbox(entity, box, false))
		{
			if (cfg::g_cfg.visuals.grenade_esp[GRENADE_BOX])
			{
				renderer::get().rect(box.x, box.y, box.w, box.h, cfg::g_cfg.visuals.grenade_box_color);

				if (cfg::g_cfg.visuals.grenade_esp[GRENADE_ICON])
					renderer::get().text(renderer::get().grenades, box.x + box.w / 2, box.y - 21, cfg::g_cfg.visuals.projectiles_color, true, false, false, true, grenade_icon.c_str());

				if (cfg::g_cfg.visuals.grenade_esp[GRENADE_TEXT])
					renderer::get().text(renderer::get().world, box.x + box.w / 2, box.y + box.h + 2, cfg::g_cfg.visuals.projectiles_color, true, false, false, true, grenade_name.c_str());
			}
			else
			{
				if (cfg::g_cfg.visuals.grenade_esp[GRENADE_ICON] && cfg::g_cfg.visuals.grenade_esp[GRENADE_TEXT])
				{
					renderer::get().text(renderer::get().grenades, box.x + box.w / 2, box.y + box.h / 2 - 10, cfg::g_cfg.visuals.projectiles_color, true, false, false, true, grenade_icon.c_str());
					renderer::get().text(renderer::get().world, box.x + box.w / 2, box.y + box.h / 2 + 7, cfg::g_cfg.visuals.projectiles_color, true, false, false, true, grenade_name.c_str());
				}
				else
				{
					if (cfg::g_cfg.visuals.grenade_esp[GRENADE_ICON])
						renderer::get().text(renderer::get().grenades, box.x + box.w / 2, box.y + box.h / 2, cfg::g_cfg.visuals.projectiles_color, true, true, false, true, grenade_icon.c_str());

					if (cfg::g_cfg.visuals.grenade_esp[GRENADE_TEXT])
						renderer::get().text(renderer::get().world, box.x + box.w / 2, box.y + box.h / 2, cfg::g_cfg.visuals.projectiles_color, true, true, false, true, grenade_name.c_str());
				}
			}
		}
	}
	else if (strstr(name, "dropped") != NULL)
	{

		if (strstr(name, "flashbang") != NULL ||
			strstr(name, "smokegrenade") != NULL ||
			strstr(name, "incendiarygrenade") != NULL ||
			strstr(name, "molotov") != NULL ||
			strstr(name, "fraggrenade") != NULL ||
			strstr(name, "decoy") != NULL)
		{

		}
		else
			return;

		auto weapon = (weapon_t*)entity; //-V1027
		Box box;

		if (util::get_bbox(weapon, box, false))
		{
			auto offset = 0;

			if (cfg::g_cfg.visuals.weapon[WEAPON_BOX])
			{
				renderer::get().rect(box.x, box.y, box.w, box.h, cfg::g_cfg.visuals.box_color);

				if (cfg::g_cfg.visuals.weapon[WEAPON_ICON])
				{
					renderer::get().text(renderer::get().weapon_icons, box.x + box.w / 2, box.y - 14, cfg::g_cfg.visuals.weapon_color, true, false, false, true, weapon->get_icon());
					offset = 14;
				}

				if (cfg::g_cfg.visuals.weapon[WEAPON_TEXT])
					renderer::get().text(renderer::get().world, box.x + box.w / 2, box.y + box.h + 2, cfg::g_cfg.visuals.weapon_color, true, false, false, true, weapon->get_name().c_str());

				if (cfg::g_cfg.visuals.weapon[WEAPON_DISTANCE])
				{
					auto distance = g_ctx.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;
					renderer::get().text(renderer::get().world, box.x + box.w / 2, box.y - 13 - offset, cfg::g_cfg.visuals.weapon_color, true, false, false, true, "%i ft", (int)distance);
				}
			}
			else
			{
				if (cfg::g_cfg.visuals.weapon[WEAPON_ICON])
					renderer::get().text(renderer::get().weapon_icons, box.x + box.w / 2, box.y + box.h / 2 - 7, cfg::g_cfg.visuals.weapon_color, true, false, false, true, weapon->get_icon());

				if (cfg::g_cfg.visuals.weapon[WEAPON_TEXT])
					renderer::get().text(renderer::get().world, box.x + box.w / 2, box.y + box.h / 2 + 6, cfg::g_cfg.visuals.weapon_color, true, false, false, true, weapon->get_name().c_str());

				if (cfg::g_cfg.visuals.weapon[WEAPON_DISTANCE])
				{
					auto distance = g_ctx.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;

					if (cfg::g_cfg.visuals.weapon[WEAPON_ICON] && cfg::g_cfg.visuals.weapon[WEAPON_TEXT])
						offset = 21;
					else if (cfg::g_cfg.visuals.weapon[WEAPON_ICON])
						offset = 21;
					else if (cfg::g_cfg.visuals.weapon[WEAPON_TEXT])
						offset = 8;

					renderer::get().text(renderer::get().world, box.x + box.w / 2, box.y + box.h / 2 - offset, cfg::g_cfg.visuals.weapon_color, true, false, false, true, "%i ft", (int)distance);
				}
			}
		}

	}
}

void worldesp::bomb_timer(entity_t* entity)
{
	if (!cfg::g_cfg.visuals.bomb_timer)
		return;

	if (!g_ctx.globals.bomb_timer_enable)
		return;

	static auto mp_c4timer = m_cvar()->FindVar(crypt_str("mp_c4timer"));
	auto bomb = (CCSBomb*)entity;

	auto c4timer = mp_c4timer->GetFloat();
	auto bomb_timer = bomb->m_flC4Blow() - m_globals()->m_curtime;

	if (bomb_timer < 0.0f)
		return;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto factor = bomb_timer / c4timer * height;

	auto red_factor = (int)(255.0f - bomb_timer / c4timer * 255.0f);
	auto green_factor = (int)(bomb_timer / c4timer * 255.0f);

	renderer::get().rect_filled(0, height - factor, 26, factor, Color(red_factor, green_factor, 0, 100));

	auto text_position = height - factor + 11;

	if (text_position > height - 9)
		text_position = height - 9;

	renderer::get().text(renderer::get().indicators, 13, text_position, Color::White, true, true, false, true, "%0.1f", bomb_timer);

	Vector screen;

	if (math::world_to_screen(entity->GetAbsOrigin(), screen))
		renderer::get().text(renderer::get().indicators, screen.x, screen.y, Color(red_factor, green_factor, 0), true, true, false, true, "BOMB");
}

void worldesp::dropped_weapons(entity_t* entity)
{
	auto weapon = (weapon_t*)entity;
	auto owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(weapon->m_hOwnerEntity());

	if (owner->is_player())
		return;

	Box box;

	if (util::get_bbox(weapon, box, false))
	{
		auto offset = 0;

		if (cfg::g_cfg.visuals.weapon[WEAPON_BOX])
		{
			renderer::get().rect(box.x, box.y, box.w, box.h, cfg::g_cfg.visuals.box_color);

			if (cfg::g_cfg.visuals.weapon[WEAPON_ICON])
			{
				renderer::get().text(renderer::get().weapon_icons, box.x + box.w / 2, box.y - 14, cfg::g_cfg.visuals.weapon_color, true, false, false, true, weapon->get_icon());
				offset = 14;
			}

			if (cfg::g_cfg.visuals.weapon[WEAPON_TEXT])
				renderer::get().text(renderer::get().world, box.x + box.w / 2, box.y + box.h + 2, cfg::g_cfg.visuals.weapon_color, true, false, false, true, weapon->get_name().c_str());

			if (cfg::g_cfg.visuals.weapon[WEAPON_AMMO] && entity->GetClientClass()->m_ClassID != CBaseCSGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSmokeGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSensorGrenadeProjectile && entity->GetClientClass()->m_ClassID != CMolotovProjectile && entity->GetClientClass()->m_ClassID != CDecoyProjectile)
			{
				auto inner_back_color = Color::Black;
				inner_back_color.SetAlpha(153);

				renderer::get().rect_filled(box.x - 1, box.y + box.h + 14, box.w + 2, 4, inner_back_color);
				renderer::get().rect_filled(box.x, box.y + box.h + 15, weapon->m_iClip1() * box.w / weapon->get_csweapon_info()->iMaxClip1, 2, cfg::g_cfg.visuals.weapon_ammo_color);
			}

			if (cfg::g_cfg.visuals.weapon[WEAPON_DISTANCE])
			{
				auto distance = g_ctx.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;
				renderer::get().text(renderer::get().world, box.x + box.w / 2, box.y - 13 - offset, cfg::g_cfg.visuals.weapon_color, true, false, false, true, "%i ft", (int)distance);
			}
		}
		else
		{
			if (cfg::g_cfg.visuals.weapon[WEAPON_ICON])
				renderer::get().text(renderer::get().weapon_icons, box.x + box.w / 2, box.y + box.h / 2 - 7, cfg::g_cfg.visuals.weapon_color, true, false, false, true, weapon->get_icon());

			if (cfg::g_cfg.visuals.weapon[WEAPON_TEXT])
				renderer::get().text(renderer::get().world, box.x + box.w / 2, box.y + box.h / 2 + 6, cfg::g_cfg.visuals.weapon_color, true, false, false, true, weapon->get_name().c_str());

			if (cfg::g_cfg.visuals.weapon[WEAPON_AMMO] && entity->GetClientClass()->m_ClassID != CBaseCSGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSmokeGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSensorGrenadeProjectile && entity->GetClientClass()->m_ClassID != CMolotovProjectile && entity->GetClientClass()->m_ClassID != CDecoyProjectile)
			{
				static auto pos = 0;

				if (cfg::g_cfg.visuals.weapon[WEAPON_ICON] && cfg::g_cfg.visuals.weapon[WEAPON_TEXT])
					pos = 19;
				else if (cfg::g_cfg.visuals.weapon[WEAPON_ICON])
					pos = 8;
				else if (cfg::g_cfg.visuals.weapon[WEAPON_TEXT])
					pos = 19;

				auto inner_back_color = Color::Black;
				inner_back_color.SetAlpha(153);

				renderer::get().rect_filled(box.x - 1, box.y + box.h / 2 + pos - 1, box.w + 2, 4, inner_back_color);
				renderer::get().rect_filled(box.x, box.y + box.h / 2 + pos, weapon->m_iClip1() * box.w / weapon->get_csweapon_info()->iMaxClip1, 2, cfg::g_cfg.visuals.weapon_ammo_color);
			}

			if (cfg::g_cfg.visuals.weapon[WEAPON_DISTANCE])
			{
				auto distance = g_ctx.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;

				if (cfg::g_cfg.visuals.weapon[WEAPON_ICON] && cfg::g_cfg.visuals.weapon[WEAPON_TEXT])
					offset = 21;
				else if (cfg::g_cfg.visuals.weapon[WEAPON_ICON])
					offset = 21;
				else if (cfg::g_cfg.visuals.weapon[WEAPON_TEXT])
					offset = 8;

				renderer::get().text(renderer::get().world, box.x + box.w / 2, box.y + box.h / 2 - offset, cfg::g_cfg.visuals.weapon_color, true, false, false, true, "%i FT", (int)distance);
			}
		}
	}
}

void worldesp::sunset_mode() {
	//Requires netvars, do it later.
}

void worldesp::viewmodel_changer()
{
	//Don't do if statements like default legendware. When you set the number back to zero its stuck at the value it was previously.

	auto viewFOV = (float)cfg::g_cfg.visuals.viewmodel_fov + 68.0f;
	static auto viewFOVcvar = m_cvar()->FindVar(crypt_str("viewmodel_fov"));

	if (viewFOVcvar->GetFloat() != viewFOV) //-V550
	{
		*(float*)((DWORD)&viewFOVcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
		viewFOVcvar->SetValue(viewFOV);
	}

	auto viewX = 1 + (float)cfg::g_cfg.visuals.viewmodel_x / 2.0f;
	static auto viewXcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_x"));

	if (viewXcvar->GetFloat() != viewX) //-V550
	{
		*(float*)((DWORD)&viewXcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
		viewXcvar->SetValue(viewX);
	}

	auto viewY = 1 + (float)cfg::g_cfg.visuals.viewmodel_y / 2.0f;
	static auto viewYcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_y"));

	if (viewYcvar->GetFloat() != viewY) //-V550
	{
		*(float*)((DWORD)&viewYcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
		viewYcvar->SetValue(viewY);
	}

	auto viewZ = -1 + (float)cfg::g_cfg.visuals.viewmodel_z / 2.0f;
	static auto viewZcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_z"));

	if (viewZcvar->GetFloat() != viewZ) //-V550
	{
		*(float*)((DWORD)&viewZcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
		viewZcvar->SetValue(viewZ);
	}
}