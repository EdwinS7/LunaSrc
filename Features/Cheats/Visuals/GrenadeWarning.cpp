// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "GrenadeWarning.h"
#include "..\..\utils\render.h"
#include <Features\logging.h>

const char* index_to_grenade_name_icon(int index)
{
	switch (index)
	{
	case WEAPON_SMOKEGRENADE: return "k"; break;
	case WEAPON_HEGRENADE: return "j"; break;
	case WEAPON_MOLOTOV:return "l"; break;
	case WEAPON_INCGRENADE:return "n"; break;
	}
}

bool c_grenade_warning::data_t::draw() const
{
	if (m_path.size() <= 1u || m_globals()->m_curtime >= m_expire_time)
		return false;

	float distance = g_ctx.local()->m_vecOrigin().DistTo(m_origin) / 12;

	if (distance > 200.f)
		return false;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto previous_screen = ZERO;
	auto previous_on_screen = math::world_to_screen(std::get< Vector >(m_path.front()), previous_screen);

	for (auto i = 1u; i < m_path.size(); ++i)
	{
		auto cur_screen = ZERO, last_cur_screen = ZERO;
		const auto cur_on_screen = math::world_to_screen(std::get< Vector >(m_path.at(i)), cur_screen);

		if (previous_on_screen && cur_on_screen)
		{
			auto color = cfg::g_cfg.visuals.grenade_proximity_warning_progress_color;
			renderer::get().line((int)previous_screen.x, (int)previous_screen.y, (int)cur_screen.x, (int)cur_screen.y, color, 2);
		}

		previous_screen = cur_screen;
		previous_on_screen = cur_on_screen;
	}

	float percent = (((m_expire_time - 0.125) - m_globals()->m_curtime) / TICKS_TO_TIME(m_tick));

	int alpha_damage = 0;

	if (m_index == WEAPON_HEGRENADE && distance <= 20)
		alpha_damage = 255 - 255 * (distance / 20);

	if ((m_index == WEAPON_MOLOTOV || m_index == WEAPON_INCGRENADE) && distance <= 15)
		alpha_damage = 255 - 255 * (distance / 15);

	auto col = cfg::g_cfg.visuals.grenade_proximity_warning_progress_color;

	Ray_t ray;
	CTraceFilterWorldAndPropsOnly filter;
	CGameTrace trace;

	ray.Init(m_origin, m_origin - Vector(0.0f, 0.0f, 105.0f));
	m_trace()->TraceRay(ray, MASK_SOLID, &filter, &trace);

	static auto global_size = Vector2D(35.0f, 5.0f);

	auto endpos_calc = trace.endpos + Vector(0.0f, 0.0f, 2.0f);

	renderer::get().circle_filled(previous_screen.x, previous_screen.y - global_size.y * 0.5f - 12, 31, 100, Color(15, 15, 15, col.a()));
	renderer::get().circle_filled(previous_screen.x, previous_screen.y - global_size.y * 0.5f - 12, 31, 100, Color(100, 0, 0, alpha_damage));
	renderer::get().sided_arc(previous_screen.x, previous_screen.y - global_size.y * 0.5f - 12, 29, percent, Color(col.r(), col.g(), col.b(), 255), 2);
	renderer::get().text(renderer::get().grenades_warning, previous_screen.x + 1, previous_screen.y - global_size.y * 0.5f - 12, Color(200, 200, 200, 255), true, true, false, true, index_to_grenade_name_icon(m_index));

	auto is_on_screen = [](Vector origin, Vector& screen) -> bool
	{
		if (!math::world_to_screen(origin, screen))
			return false;

		static int width_worldtoscreen, height_worldtoscreen;
		m_engine()->GetScreenSize(width_worldtoscreen, height_worldtoscreen);

		auto x = width_worldtoscreen > screen.x;
		auto y = height_worldtoscreen > screen.y;

		return x && y;
	};

	Vector screen_pos;
	Vector enemy_origin = m_origin;
	Vector local_origin = g_ctx.local()->GetAbsOrigin();

	if (!g_ctx.local()->is_alive())
		local_origin = m_input()->m_vecCameraOffset;

	Vector viewangles;
	m_engine()->GetViewAngles(viewangles);

	static int width_default, height_default;
	m_engine()->GetScreenSize(width_default, height_default);

	if (cfg::g_cfg.visuals.grenade_proximity_warning && !is_on_screen(enemy_origin, screen_pos))
	{
		auto screen_center = Vector2D(width * 0.5f, height * 0.5f);
		auto angleYawRad = DEG2RAD(viewangles.y - math::calculate_angle(g_ctx.globals.eye_pos, enemy_origin).y - 90.0f);

		auto radius = 80 - 70 * float(alpha_damage / 255.f);
		auto size = 3 + 3 * float(alpha_damage / 255.f);

		auto newPointX = screen_center.x + ((((width_default - (size * 3)) * 0.5f) * (radius / 100.0f)) * cos(angleYawRad)) + (int)(6.0f * (((float)size - 4.0f) / 16.0f));
		auto newPointY = screen_center.y + ((((height_default - (size * 3)) * 0.5f) * (radius / 100.0f)) * sin(angleYawRad));
		auto newPointX2 = screen_center.x + ((((width_default - (size * 15)) * 0.5f) * (radius / 100.0f)) * cos(angleYawRad)) + (int)(6.0f * (((float)size - 4.0f) / 16.0f));
		auto newPointY2 = screen_center.y + ((((height_default - (size * 15)) * 0.5f) * (radius / 100.0f)) * sin(angleYawRad));

		std::array <Vector2D, 3> points
		{
			Vector2D(newPointX - size, newPointY - size),
			Vector2D(newPointX + size, newPointY),
			Vector2D(newPointX - size, newPointY + size)
		};

		auto warn = Vector2D(newPointX2, newPointY2);
		math::rotate_triangle(points, viewangles.y - math::calculate_angle(g_ctx.globals.eye_pos, enemy_origin).y - 90.0f);

		renderer::get().circle_filled(warn.x, warn.y - (size * 3) * 0.5f, 31, 100, Color(15, 15, 15, col.a()));
		renderer::get().circle_filled(warn.x, warn.y - (size * 3) * 0.5f, 31, 100, Color(100, 0, 0, alpha_damage));
		renderer::get().sided_arc(warn.x, warn.y - (size * 3) * 0.5f, 29, percent, Color(col.r(), col.g(), col.b(), 255), 2);
		renderer::get().text(renderer::get().grenades_warning, warn.x, warn.y - (size * 3) * 0.5f, Color(200, 200, 200, 255), true, true, false, true, index_to_grenade_name_icon(m_index));
	}

	return true;
}

void c_grenade_warning::grenade_warning(entity_t* entity)
{
	auto& predicted_grenades = c_grenade_warning::get().get_list();

	static auto last_server_tick = m_globals()->m_curtime;

	if (last_server_tick != m_globals()->m_curtime)
	{
		predicted_grenades.clear();
		last_server_tick = m_globals()->m_curtime;
	}

	// HOMO CHECK BY REIS NEMESIS PASTE.. :/
	if (entity->IsDormant() || !cfg::g_cfg.visuals.grenade_proximity_warning)
		return;

	const auto client_class = entity->GetClientClass();

	if (!client_class || client_class->m_ClassID != CMolotovProjectile && client_class->m_ClassID != CBaseCSGrenadeProjectile)
		return;

	if (client_class->m_ClassID == CBaseCSGrenadeProjectile)
	{
		const auto model = entity->GetModel();

		if (!model)
			return;

		const auto studio_model = m_modelinfo()->GetStudioModel(model);

		if (!studio_model || std::string_view(studio_model->szName).find("fraggrenade") == std::string::npos)
			return;
	}

	const auto handle = entity->GetRefEHandle().ToLong();

	if (entity->m_nExplodeEffectTickBegin() || !entity->m_hThrower().IsValid() || ((entity->m_hThrower().Get()->m_iTeamNum() == g_ctx.local()->m_iTeamNum() || m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1) && entity->m_hThrower().Get() != g_ctx.local()))
	{
		predicted_grenades.erase(handle);
		return;
	}

	if (predicted_grenades.find(handle) == predicted_grenades.end())
	{
		predicted_grenades.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(handle),
			std::forward_as_tuple(
				entity->m_hThrower().Get(),
				client_class->m_ClassID == CMolotovProjectile ? WEAPON_MOLOTOV : WEAPON_HEGRENADE,
				entity->m_vecOriginGrenade(), reinterpret_cast<player_t*>(entity)->m_vecVelocityGrenade(),
				entity->m_flSpawnTime(), TIME_TO_TICKS(reinterpret_cast<player_t*>(entity)->m_flSimulationTime() - entity->m_flSpawnTime())
			)
		);
	}

	if (predicted_grenades.at(handle).draw())
		return;

	predicted_grenades.erase(handle);
}