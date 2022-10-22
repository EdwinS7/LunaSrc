// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "zeusbot.h"
#include "..\misc\misc.h"
#include "..\misc\logs.h"
#include "..\ragebot\penetration.h"
#include "..\Prediction\EnginePrediction.h"
#include "..\movement\slowwalk.h"
#include "..\lagcompensation\LocalAnimations.h"
#include <random>
#include "..\exploits\TickBase.h"

void zeusbot::run(CUserCmd* cmd)
{
	scanned_targets.clear();
	final_target.reset();

	if (!cfg::g_cfg.ragebot.enable)
		return;

	if (!cfg::g_cfg.ragebot.zeus_bot)
		return;

	if (g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_TASER)
		return;

	scan_targets();

	if (scanned_targets.empty())
		return;

	find_best_target();

	if (!final_target.data.valid())
		return;

	fire(cmd);
}

static bool compare_records(const adjust_data& first, const adjust_data& second)
{
	if (first.origin != second.origin)
		return first.origin.DistTo(g_ctx.local()->GetAbsOrigin()) < second.origin.DistTo(g_ctx.local()->GetAbsOrigin());

	return first.simulation_time > second.simulation_time;
}

void zeusbot::scan_targets()
{
	if (Rbot::get().targets.empty())
		return;

	for (auto& target : Rbot::get().targets)
	{
		if (target.history_record->valid())
		{
			scan_data last_data;

			if (target.last_record->valid())
			{
				target.last_record->adjust_player();
				scan(target.last_record, last_data);
			}

			scan_data history_data;

			target.history_record->adjust_player();
			scan(target.history_record, history_data);

			if (last_data.valid() && last_data.damage > history_data.damage)
				scanned_targets.emplace_back(scanned_target(target.last_record, last_data));
			else if (history_data.valid())
				scanned_targets.emplace_back(scanned_target(target.history_record, history_data));
		}
		else
		{
			if (!target.last_record->valid())
				continue;

			scan_data last_data;

			target.last_record->adjust_player();
			scan(target.last_record, last_data);

			if (!last_data.valid())
				continue;

			scanned_targets.emplace_back(scanned_target(target.last_record, last_data));
		}
	}
}

void zeusbot::scan(adjust_data* record, scan_data& data)
{
	auto best_damage = 0;
	auto hitbox = 0;

	std::vector <scan_point> points;

	for (hitbox = CSGOHitboxID::Pelvis; hitbox <= CSGOHitboxID::UpperChest; ++hitbox)
	{
		auto point = scan_point(record->player->hitbox_position_matrix(hitbox, record->matrixes_data.main), hitbox, true);

		if (!record->bot)
		{
			auto safe = 0.0f;

			if (record->matrixes_data.zero[0].GetOrigin() == record->matrixes_data.first[0].GetOrigin() || record->matrixes_data.zero[0].GetOrigin() == record->matrixes_data.second[0].GetOrigin() || record->matrixes_data.first[0].GetOrigin() == record->matrixes_data.second[0].GetOrigin())
				safe = 0.0f;
			else if (!hitbox_intersection(record->player, record->matrixes_data.zero, hitbox, g_ctx.globals.eye_pos, point.point, &safe))
				safe = 0.0f;
			else if (!hitbox_intersection(record->player, record->matrixes_data.first, hitbox, g_ctx.globals.eye_pos, point.point, &safe))
				safe = 0.0f;
			else if (!hitbox_intersection(record->player, record->matrixes_data.second, hitbox, g_ctx.globals.eye_pos, point.point, &safe))
				safe = 0.0f;

			point.safe = safe;
		}
		else
			point.safe = 1.0f;

		points.emplace_back(point);
	}

	if (points.empty())
		return;

	for (auto& point : points)
	{
		if (data.point.safe && data.point.safe < point.safe)
			continue;

		auto fire_data = autowall::get().wall_penetration(g_ctx.globals.eye_pos, point.point, record->player);

		if (!fire_data.valid)
			continue;

		if (fire_data.damage < 1)
			continue;

		if (!fire_data.visible)
			continue;

		if (fire_data.damage >= record->player->m_iHealth() * 1.5f && fire_data.damage >= best_damage)
		{
			best_damage = fire_data.damage;

			data.point = point;
			data.visible = fire_data.visible;
			data.damage = fire_data.damage;
			data.hitbox = fire_data.hitbox;
		}
	}
}

static bool compare_targets(const scanned_target& first, const scanned_target& second)
{
	if (cfg::g_cfg.player_list.high_priority[first.record->i] != cfg::g_cfg.player_list.high_priority[second.record->i])
		return cfg::g_cfg.player_list.high_priority[first.record->i];

	return first.data.damage > second.data.damage;
}

void zeusbot::find_best_target()
{
	std::sort(scanned_targets.begin(), scanned_targets.end(), compare_targets);

	for (auto& target : scanned_targets)
	{
		if (target.fov > 180)
			continue;

		final_target = target;
		final_target.record->adjust_player();
		break;
	}
}

void zeusbot::fire(CUserCmd* cmd)
{
	if (!g_ctx.globals.weapon->can_fire(true))
		return;

	auto aim_angle = math::calculate_angle(g_ctx.globals.eye_pos, final_target.data.point.point).Clamp();

	if (!cfg::g_cfg.ragebot.autoshoot && !(cmd->m_buttons & IN_ATTACK))
		return;

	auto final_hitchance = hitchance(aim_angle);

	if (final_hitchance < cfg::g_cfg.ragebot.zeus_bot_hitchance)
		return;

	auto backtrack_ticks = 0;
	auto net_channel_info = m_engine()->GetNetChannelInfo();

	if (net_channel_info)
	{
		auto original_tickbase = g_ctx.globals.backup_tickbase;

		//if (exploit::get().double_tap_enabled && exploit::get().double_tap_key)
		//	original_tickbase = g_ctx.globals.backup_tickbase + g_ctx.globals.weapon->get_max_tickbase_shift();

		static auto sv_maxunlag = m_cvar()->FindVar(crypt_str("sv_maxunlag"));

		auto correct = math::clamp(net_channel_info->GetLatency(FLOW_OUTGOING) + net_channel_info->GetLatency(FLOW_INCOMING) + util::get_interpolation(), 0.0f, sv_maxunlag->GetFloat());
		auto delta_time = correct - (TICKS_TO_TIME(original_tickbase) - final_target.record->simulation_time);

		backtrack_ticks = TIME_TO_TICKS(fabs(delta_time));
	}

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

	player_info_t player_info;
	m_engine()->GetPlayerInfo(final_target.record->i, &player_info);

#if BETA
	std::stringstream log;

	log << crypt_str("Fired shot at ") + (std::string)player_info.szName + crypt_str(": ");
	log << crypt_str("hitchance: ") + (final_hitchance == 101 ? crypt_str("MA") : std::to_string(final_hitchance)) + crypt_str(", ");
	log << crypt_str("hitbox: ") + get_hitbox_name(final_target.data.hitbox) + crypt_str(", ");
	log << crypt_str("damage: ") + std::to_string(final_target.data.damage) + crypt_str(", ");
	log << crypt_str("backtrack: ") + std::to_string(backtrack_ticks);

	if (cfg::g_cfg.misc.events_to_log[EVENTLOG_HIT])
		eventlogs::get().add(log.str());
#endif
	cmd->m_viewangles = aim_angle;
	cmd->m_buttons |= IN_ATTACK;
	cmd->m_tickcount = TIME_TO_TICKS(final_target.record->simulation_time + util::get_interpolation());

	g_ctx.globals.aimbot_working = true;
}

int zeusbot::hitchance(const Vector& aim_angle)
{
	auto final_hitchance = 0;
	auto weapon_info = g_ctx.globals.weapon->get_csweapon_info();

	if (!weapon_info)
		return final_hitchance;

	if ((g_ctx.globals.eye_pos - final_target.data.point.point).Length() > weapon_info->flRange)
		return final_hitchance;

	auto forward = ZERO;
	auto right = ZERO;
	auto up = ZERO;

	math::angle_vectors(aim_angle, &forward, &right, &up);

	math::fast_vec_normalize(forward);
	math::fast_vec_normalize(right);
	math::fast_vec_normalize(up);

	auto inaccuracy = weapon_info->flInaccuracyStand;

	if (g_ctx.local()->m_fFlags() & FL_DUCKING)
		inaccuracy = weapon_info->flInaccuracyCrouch;

	if (g_ctx.globals.inaccuracy - 0.000001f < inaccuracy)
		final_hitchance = 101;
	else
	{
		static auto setup_spread_values = true;
		static float spread_values[256][6];

		if (setup_spread_values)
		{
			setup_spread_values = false;

			for (auto i = 0; i < 256; ++i)
			{
				math::random_seed(i + 1);

				auto a = math::random_float(0.0f, 1.0f);
				auto b = math::random_float(0.0f, DirectX::XM_2PI);
				auto c = math::random_float(0.0f, 1.0f);
				auto d = math::random_float(0.0f, DirectX::XM_2PI);

				spread_values[i][0] = a;
				spread_values[i][1] = c;

				auto sin_b = 0.0f, cos_b = 0.0f;
				DirectX::XMScalarSinCos(&sin_b, &cos_b, b);

				auto sin_d = 0.0f, cos_d = 0.0f;
				DirectX::XMScalarSinCos(&sin_d, &cos_d, d);

				spread_values[i][2] = sin_b;
				spread_values[i][3] = cos_b;
				spread_values[i][4] = sin_d;
				spread_values[i][5] = cos_d;
			}
		}

		auto hits = 0;

		for (auto i = 0; i < 256; ++i)
		{
			auto inaccuracy = spread_values[i][0] * g_ctx.globals.inaccuracy;
			auto spread = spread_values[i][1] * g_ctx.globals.spread;

			auto spread_x = spread_values[i][3] * inaccuracy + spread_values[i][5] * spread;
			auto spread_y = spread_values[i][2] * inaccuracy + spread_values[i][4] * spread;

			auto direction = ZERO;

			direction.x = forward.x + right.x * spread_x + up.x * spread_y;
			direction.y = forward.y + right.y * spread_x + up.y * spread_y;
			direction.z = forward.z + right.z * spread_x + up.z * spread_y; //-V778

			auto end = g_ctx.globals.eye_pos + direction * weapon_info->flRange;

			if (hitbox_intersection(final_target.record->player, final_target.record->matrixes_data.main, final_target.data.hitbox, g_ctx.globals.eye_pos, end))
				++hits;
		}

		final_hitchance = (int)((float)hits / 2.56f);
	}

	return final_hitchance;
}

static int clip_ray_to_hitbox(const Ray_t& ray, mstudiobbox_t* hitbox, matrix3x4_t& matrix, trace_t& trace)
{
	static auto fn = util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F8 F3 0F 10 42"));

	trace.fraction = 1.0f;
	trace.startsolid = false;

	return reinterpret_cast <int(__fastcall*)(const Ray_t&, mstudiobbox_t*, matrix3x4_t&, trace_t&)> (fn)(ray, hitbox, matrix, trace);
}

bool zeusbot::hitbox_intersection(player_t* e, matrix3x4_t* matrix, int hitbox, const Vector& start, const Vector& end, float* safe)
{
	auto model = e->GetModel();

	if (!model)
		return false;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return false;

	auto studio_set = studio_model->pHitboxSet(e->m_nHitboxSet());

	if (!studio_set)
		return false;

	auto studio_hitbox = studio_set->pHitbox(hitbox);

	if (!studio_hitbox)
		return false;

	trace_t trace;

	Ray_t ray;
	ray.Init(start, end);

	auto intersected = clip_ray_to_hitbox(ray, studio_hitbox, matrix[studio_hitbox->bone], trace) >= 0;

	if (!safe)
		return intersected;

	Vector min, max;

	math::vector_transform(studio_hitbox->bbmin, matrix[studio_hitbox->bone], min);
	math::vector_transform(studio_hitbox->bbmax, matrix[studio_hitbox->bone], max);

	auto center = (min + max) * 0.5f;
	auto distance = center.DistTo(end);

	if (distance > *safe)
		*safe = distance;

	return intersected;
}