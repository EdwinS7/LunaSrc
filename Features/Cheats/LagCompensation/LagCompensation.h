#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"


enum
{
	MAIN,
	NONE,
	FIRST,
	SECOND
};

enum resolve_type
{
	RESOLVE_STAND,
	RESOLVE_MOVE,
	RESOLVE_WALK,
	RESOLVE_LBY,
	RESOLVE_AIR
};

struct matrixes
{
	matrix3x4_t main[MAXSTUDIOBONES];
	matrix3x4_t zero[MAXSTUDIOBONES];
	matrix3x4_t first[MAXSTUDIOBONES];
	matrix3x4_t second[MAXSTUDIOBONES];
};

class adjust_data;

enum resolver_side
{
	RESOLVER_ORIGINAL,
	RESOLVER_ZERO,
	RESOLVER_FIRST,
	RESOLVER_SECOND,
	RESOLVER_LOW_FIRST,
	RESOLVER_LOW_SECOND,
	RESOLVER_LOW_FIRST1,
	RESOLVER_LOW_SECOND1
};

class CResolver
{
	player_t* player = nullptr;
	adjust_data* player_record = nullptr;

	float original_goal_feet_yaw = 0.0f;
	float original_pitch = 0.0f;
	int FreestandSide[64];
	float low_desync_brute = 0.0f;
	float lock_side = 0.0f;
	float desync_angle = 0.0f;
	int global_side = 0;

	AnimationLayer layerz[13];
	AnimationLayer moveLayerz[3][13];

	int last_ticks[65];

public:
	float ResolvedAngle[65];
	void Init(player_t* e, adjust_data* record, const float& goal_feet_yaw, const float& pitch);
	void Reset();
	float BruteForce(player_t* player, bool roll);
	int GetThinkSide(player_t* player);
	bool DoesHaveFakeAngles(player_t* player);
	bool GetLowDeltaState(player_t* player);
	float GetAngle(player_t* player);
	float GetAwayAngle(player_t* player);
	float GetBackwardYaw(player_t* player);
	float GetForwardYaw(player_t* player);
	float GetLeftYaw(player_t* player);
	float GetRightYaw(player_t* player);
	void RunResolve(player_t* player);

	float gfy_default = 0.0f;
	bool should_force_safepoint = false;

	AnimationLayer resolver_layers[3][13]; //13 was 15 before.
	AnimationLayer previous_layers[13];
	resolver_side last_side = RESOLVER_ORIGINAL;
};

enum resolver_type
{
	DEFAULT,
	STAND_RESOLVER,
	SLOWWALK_RESOLVER,
	MOVE_RESOLVER,
	AIR_RESOLVER,
	BRUTEFORCE_RESOLVER,
	LEAN_RESOLVER,
	BRUTEFORCEROLL_RESOLVER,
};


enum animstate_layer_t
{
	ANIMATION_LAYER_AIMMATRIX = 0,
	ANIMATION_LAYER_WEAPON_ACTION,
	ANIMATION_LAYER_WEAPON_ACTION_RECROUCH,
	ANIMATION_LAYER_ADJUST,
	ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL,
	ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB,
	ANIMATION_LAYER_MOVEMENT_MOVE,
	ANIMATION_LAYER_MOVEMENT_STRAFECHANGE,
	ANIMATION_LAYER_WHOLE_BODY,
	ANIMATION_LAYER_FLASHED,
	ANIMATION_LAYER_FLINCH,
	ANIMATION_LAYER_ALIVELOOP,
	ANIMATION_LAYER_LEAN,
	ANIMATION_LAYER_COUNT,
};


enum resolver_history
{
	HISTORY_UNKNOWN = -1,
	HISTORY_ORIGINAL,
	HISTORY_ZERO,
	HISTORY_DEFAULT,
	HISTORY_LOW
};

class adjust_data //-V730
{
public:
	player_t* player;
	int i;
	AnimationLayer anim_layers[13];
	AnimationLayer right_layers[13];
	AnimationLayer left_layers[13];
	AnimationLayer center_layers[13];
	AnimationLayer layers[13];


	matrixes matrixes_data;
	float exit_angle = 0;
	resolver_type type;
	resolver_side side;

	bool invalid;
	bool immune;
	bool dormant;
	bool bot;
	bool shot;

	int flags;
	int bone_count;

	float simulation_time;
	float duck_amount;
	float lby;
	int m_ChokedTicks;

	Vector angles;
	Vector abs_angles;
	Vector velocity;
	Vector origin;
	Vector mins;
	Vector maxs;

	adjust_data() //-V730
	{
		reset();
	}

	void reset()
	{
		player = nullptr;
		i = -1;

		type = DEFAULT;
		side = RESOLVER_ORIGINAL;

		invalid = false;
		immune = false;
		dormant = false;
		bot = false;
		shot = false;

		flags = 0;
		bone_count = 0;

		simulation_time = 0.0f;
		duck_amount = 0.0f;
		lby = 0.0f;

		angles.Zero();
		abs_angles.Zero();
		velocity.Zero();
		origin.Zero();
		mins.Zero();
		maxs.Zero();
	}

	adjust_data(player_t* e, bool store = true)
	{
		type = DEFAULT;
		side = RESOLVER_ORIGINAL;

		invalid = false;
		store_data(e, store);
	}

	void store_data(player_t* e, bool store = true)
	{
		if (!e->is_alive())
			return;

		player = e;
		i = player->EntIndex();

		if (store)
		{
			memcpy(layers, e->get_animlayers(), e->animlayer_count() * sizeof(AnimationLayer));
			memcpy(matrixes_data.main, player->m_CachedBoneData().Base(), player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
		}

		immune = player->m_bGunGameImmunity() || player->m_fFlags() & FL_FROZEN;
		dormant = player->IsDormant();

		player_info_t player_info;
		m_engine()->GetPlayerInfo(i, &player_info);

		bot = player_info.fakeplayer;
		shot = player->m_hActiveWeapon() && (player->m_hActiveWeapon()->m_fLastShotTime() == player->m_flSimulationTime());

		flags = player->m_fFlags();
		bone_count = player->m_CachedBoneData().Count();

		simulation_time = player->m_flSimulationTime();
		duck_amount = player->m_flDuckAmount();
		lby = player->m_flLowerBodyYawTarget();

		angles = player->m_angEyeAngles();
		abs_angles = player->GetAbsAngles();
		velocity = player->m_vecVelocity();
		origin = player->m_vecOrigin();
		mins = player->GetCollideable()->OBBMins();
		maxs = player->GetCollideable()->OBBMaxs();
	}

	void adjust_player()
	{
		if (!valid(false))
			return;

		memcpy(player->get_animlayers(), layers, player->animlayer_count() * sizeof(AnimationLayer));
		memcpy(player->m_CachedBoneData().Base(), matrixes_data.main, player->m_CachedBoneData().Count() * sizeof(matrix3x4_t)); //-V807

		player->m_fFlags() = flags;
		player->m_CachedBoneData().m_Size = bone_count;

		player->m_flSimulationTime() = simulation_time;
		player->m_flDuckAmount() = duck_amount;
		player->m_flLowerBodyYawTarget() = lby;

		player->m_angEyeAngles() = angles;
		player->set_abs_angles(abs_angles);
		player->m_vecVelocity() = velocity;
		player->m_vecOrigin() = origin;
		player->set_abs_origin(origin);
		player->GetCollideable()->OBBMins() = mins;
		player->GetCollideable()->OBBMaxs() = maxs;
	}

	bool valid(bool extra_checks = true)
	{
		if (!this) //-V704
			return false;

		if (i > 0)
			player = (player_t*)m_entitylist()->GetClientEntity(i);

		if (!player)
			return false;

		if (player->m_lifeState() != LIFE_ALIVE)
			return false;

		if (immune)
			return false;

		if (dormant)
			return false;

		if (!extra_checks)
			return true;

		if (invalid)
			return false;

		auto net_channel_info = m_engine()->GetNetChannelInfo();

		if (!net_channel_info)
			return false;

		static auto sv_maxunlag = m_cvar()->FindVar(crypt_str("sv_maxunlag"));

		auto outgoing = net_channel_info->GetLatency(FLOW_OUTGOING);
		auto incoming = net_channel_info->GetLatency(FLOW_INCOMING);

		float correct = 0.f;
		correct += net_channel_info->GetLatency(FLOW_OUTGOING);
		correct += net_channel_info->GetLatency(FLOW_INCOMING);
		correct += util::get_interpolation(); //was 2
		correct = math::clamp< float >(correct, 0.f, 1.f);

		auto curtime = g_ctx.local()->is_alive() ? TICKS_TO_TIME(g_ctx.globals.fixed_tickbase) : m_globals()->m_curtime; //-V807

		auto delta_time = correct - (curtime - simulation_time);
		//auto delta_time = correct - (TICKS_TO_TIME(original_tickbase) - final_target.record->simulation_time);

		if (fabs(delta_time) > 0.2f)
			return false;

		auto extra_choke = 0;

		if (g_ctx.globals.fakeducking)
			extra_choke = 14 - m_clientstate()->iChokedCommands;

		auto server_tickcount = extra_choke + m_globals()->m_tickcount + TIME_TO_TICKS(outgoing + incoming);
		auto dead_time = (int)(TICKS_TO_TIME(server_tickcount) - sv_maxunlag->GetFloat());

		if (simulation_time < (float)dead_time)
			return false;

		return true;
	}
};

class optimized_adjust_data
{
public:
	int i;
	player_t* player;

	float simulation_time;
	float duck_amount;

	Vector angles;
	Vector origin;

	bool shot;

	optimized_adjust_data() //-V730
	{
		reset();
	}

	void reset()
	{
		i = 0;
		player = nullptr;

		simulation_time = 0.0f;
		duck_amount = 0.0f;
		shot = false;
		angles.Zero();
		origin.Zero();
	}
};

struct incoming_sequence_t {
	incoming_sequence_t::incoming_sequence_t(int instate, int outstate, int seqnr, float time) {
		m_in_reliable_state = instate;
		m_out_reliable_state = outstate;
		m_sequence_nr = seqnr;
		m_current_time = time;
	}

	int m_in_reliable_state;
	int m_out_reliable_state;
	int m_sequence_nr;
	float m_current_time;
};

extern std::deque <adjust_data> player_records[65];
struct player_settings
{
	__int64 id;
	resolver_history res_type;
	bool low_stand;
	bool low_move;
	bool faking;
	int neg;
	int pos;

	player_settings(__int64 id, resolver_history res_type, bool low_stand, bool low_move, bool faking, int left, int right) noexcept : id(id), res_type(res_type), low_stand(low_stand), low_move(low_move), faking(faking), neg(neg), pos(pos)
	{

	}
};
class lagcompensation : public singleton <lagcompensation>
{
public:
	std::vector<player_settings> player_sets;

	void apply_interpolation_flags(player_t* e, bool flag);	void FixLandingAnimations(player_t* m_pPlayer, adjust_data* LagRecord, adjust_data* PreviousLagRecord);
	void extrapolate(player_t* player, Vector& origin, Vector& velocity, int& flags, bool on_ground);
	void fsn(ClientFrameStage_t stage);
	bool valid(int i, player_t* e);
	void update_player_animations(player_t* e);

	CResolver CPlayerResolver[65];

	bool is_dormant[65];
	float previous_goal_feet_yaw[65];

	std::deque< incoming_sequence_t > sequences;
	int m_last_incoming_sequence_number;
	BoneArray* m_Matrix;
	BoneArray* m_res;

	void update_sequence();
	void clear_sequence();
	void add_latency(INetChannel* net_channel);
};