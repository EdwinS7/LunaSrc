#pragma once
#include "..\..\includes.hpp"
#include "..\lagcompensation\LagCompensation.h"
#include "..\Prediction\EnginePrediction.h"
#include "..\exploits\TickBase.h"
#include "..\misc\logs.h"
#include "..\movement\slowwalk.h"

class target
{
public:
	player_t* e;

	adjust_data* last_record;
	adjust_data* history_record;

	target()
	{
		e = nullptr;

		last_record = nullptr;
		history_record = nullptr;
	}

	target(player_t* e, adjust_data* last_record, adjust_data* history_record) //-V818
	{
		this->e = e;

		this->last_record = last_record;
		this->history_record = history_record;
	}
};

class scan_point
{
public:
	Vector point;
	int hitbox;
	bool center;
	float safe;
	float semi_safe;

	scan_point()
	{
		point.Zero();
		hitbox = -1;
		center = false;
		safe = 0.0f;
		semi_safe = 0.0f;
	}

	scan_point(const Vector& point, const int& hitbox, const bool& center) //-V818 //-V730
	{
		this->point = point;
		this->hitbox = hitbox;
		this->center = center;
	}

	void reset()
	{
		point.Zero();
		hitbox = -1;
		center = false;
		safe = 0.0f;
		semi_safe = 0.0f;
	}
};

class scan_data
{
public:
	scan_point point;
	bool visible;
	int damage;
	int hitbox;

	scan_data()
	{
		reset();
	}

	void reset()
	{
		point.reset();
		visible = false;
		damage = -1;
		hitbox = -1;
	}

	bool valid()
	{
		return damage >= 1 && hitbox != -1;
	}
};

struct Last_target
{
	adjust_data record;
	scan_data data;
	float distance;
};

class scanned_target
{
public:
	adjust_data* record;
	scan_data data;

	float fov;
	float distance;
	int health;

	scanned_target()
	{
		reset();
	}

	scanned_target(const scanned_target& data) //-V688
	{
		this->record = data.record;
		this->data = data.data;
		this->fov = data.fov;
		this->distance = data.distance;
		this->health = data.health;
	}

	scanned_target& operator=(const scanned_target& data) //-V688
	{
		this->record = data.record;
		this->data = data.data;
		this->fov = data.fov;
		this->distance = data.distance;
		this->health = data.health;

		return *this;
	}

	scanned_target(adjust_data* record, const scan_data& data) //-V688 //-V818
	{
		this->record = record;
		this->data = data;

		Vector viewangles;
		m_engine()->GetViewAngles(viewangles);

		auto aim_angle = math::calculate_angle(g_ctx.globals.eye_pos, data.point.point); //-V688
		auto fov = math::get_fov(viewangles, aim_angle); //-V688

		this->fov = fov;
		this->distance = g_ctx.globals.eye_pos.DistTo(data.point.point);
		this->health = record->player->m_iHealth();
	}

	void reset()
	{
		record = nullptr;
		data.reset();

		fov = 0.0f;
		distance = 0.0f;
		health = 0;
	}
};

static auto get_hitbox_name = [](int hitbox, bool shot_info = false) -> std::string {
	switch (hitbox)
	{
	case CSGOHitboxID::Head:
		return crypt_str("head");
	case CSGOHitboxID::LowerChest:
		return crypt_str("lower chest");
	case CSGOHitboxID::Chest:
		return crypt_str("chest");
	case CSGOHitboxID::UpperChest:
		return crypt_str("upper chest");
	case CSGOHitboxID::Stomach:
		return crypt_str("stomach");
	case CSGOHitboxID::Pelvis:
		return crypt_str("pelvis");
	case CSGOHitboxID::RightUpperArm:
		return crypt_str("left arm");
	case CSGOHitboxID::RightLowerArm:
		return crypt_str("left arm");
	case CSGOHitboxID::RightHand:
		return crypt_str("left arm");
	case CSGOHitboxID::LeftUpperArm:
		return crypt_str("right arm");
	case CSGOHitboxID::LeftLowerArm:
		return crypt_str("right arm");
	case CSGOHitboxID::LeftHand:
		return crypt_str("right arm");
	case CSGOHitboxID::RightThigh:
		return crypt_str("left leg");
	case CSGOHitboxID::RightCalf:
		return crypt_str("left leg");
	case CSGOHitboxID::LeftThigh:
		return crypt_str("right leg");
	case CSGOHitboxID::LeftCalf:
		return crypt_str("right leg");
	case CSGOHitboxID::RightFoot:
		return crypt_str("left foot");
	case CSGOHitboxID::LeftFoot:
		return crypt_str("right foot");
	}
};

//Does not work in class?
static std::vector < std::tuple < float, float, float >> precomputed_seeds = {};

class Rbot : public singleton <Rbot>
{
	int shots_fired = 0;
	void Reset();
	void UpdateConfig();
	bool SanityCheck(CUserCmd* cmd, bool weapon = false, int idx = -1, bool check_weapon = false);
	int GetDamage(int health);
	void AutoR8(CUserCmd* cmd);
	int GetTicksToShoot();
	int GetTicksToStop();
	void PredictiveQuickStop(CUserCmd* cmd, int idx);
	void QuickStop(CUserCmd* cmd);
	void PrepareTargets();
	adjust_data* RecieveRecords(std::deque <adjust_data>* records, bool history);
	bool IsSafePoint(adjust_data* record, Vector start_position, Vector end_position, int hitbox);
	void StartScan();
	void FindOptimalTarget();
	void Fire(CUserCmd* cmd);

	static int ClipRayToHitbox(const Ray_t& ray, mstudiobbox_t* hitbox, matrix3x4_t& matrix, trace_t& trace)
	{
		static auto fn = (util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F8 F3 0F 10 42")));

		trace.fraction = 1.0f;
		trace.startsolid = false;

		return reinterpret_cast <int(__fastcall*)(const Ray_t&, mstudiobbox_t*, matrix3x4_t&, trace_t&)> (fn)(ray, hitbox, matrix, trace);
	}

	void BuildSeedTable() {
		if (!precomputed_seeds.empty())
			return;

		for (auto i = 0; i < 255; i++) {
			math::random_seed(i + 1);

			const auto pi_seed = math::random_float(0.f, twopi);

			precomputed_seeds.emplace_back(math::random_float(0.f, 1.f),
				sin(pi_seed), cos(pi_seed));
		}
	}

	std::vector <scanned_target> scanned_targets;
	scanned_target final_target;
public:
	bool force_stop;
	void Run(CUserCmd* cmd);
	void Scan(adjust_data* record, scan_data& data, const Vector& shoot_position = g_ctx.globals.eye_pos);
	std::vector <int> GetHitboxes(adjust_data* record);
	std::vector <scan_point> GetPoints(adjust_data* record, int hitbox, bool optimized = false);
	bool HitboxIntersection(player_t* e, matrix3x4_t* matrix, int hitbox, const Vector& start, const Vector& end);
	bool CalculateHitchance(const Vector& aim_angle, int& final_hitchance);

	std::vector <target> targets;
	std::vector <adjust_data> backup;

	int final_hitchance = 0;
	int last_target_index = -1;
	Last_target last_target[65];
	float hitchance_logged;
	Vector last_shoot_position;
	bool fire_data_saved;
	bool Should_AutoStop;
	int lastshifttime;
};