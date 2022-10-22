#include "LegitBacktrack.h"
#include "legitbot.h"
#include <algorithm> 


backtrackData headPositions[64][12];
player_bt bt_data[64][12];
#define TICK_INTERVAL			(m_globals()->m_intervalpertick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

inline float distance_point_to_line(Vector Point, Vector LineOrigin, Vector Dir)
{
	auto PointDir = Point - LineOrigin;

	auto TempOffset = PointDir.Dot(Dir) / (Dir.x * Dir.x + Dir.y * Dir.y + Dir.z * Dir.z);
	if (TempOffset < 0.000001f)
		return FLT_MAX;

	auto PerpendicularPoint = LineOrigin + (Dir * TempOffset);

	return (Point - PerpendicularPoint).Length();
}

inline Vector angle_vector(Vector meme)
{
	auto sy = sin(meme.y / 180.f * static_cast<float>(3.14159265358979323846f));
	auto cy = cos(meme.y / 180.f * static_cast<float>(3.14159265358979323846f));

	auto sp = sin(meme.x / 180.f * static_cast<float>(3.14159265358979323846f));
	auto cp = cos(meme.x / 180.f * static_cast<float>(3.14159265358979323846f));

	return Vector(cp * cy, cp * sy, -sp);
}

void NewBacktrack::LegitBacktrack(CUserCmd* cmd)
{
	int bestTargetIndex = -1;
	float bestFov = FLT_MAX;
	player_info_t info;

	auto local_id = m_engine()->GetLocalPlayer();
	if (!local_id) return;

	player_t* local = (player_t*)m_entitylist()->GetClientEntity(local_id);
	if (!local) return;

	for (int i = 1; i < m_engine()->GetMaxClients(); i++)
	{
		player_t* entity = (player_t*)m_entitylist()->GetClientEntity(i);

		if (!entity) continue;
		if (entity == local) continue;
		if (!m_engine()->GetPlayerInfo(i, &info)) continue;
		if (entity->IsDormant()) continue;
		if (entity->m_iTeamNum() == local->m_iTeamNum()) continue;
		if (!entity->is_alive()) continue;

		float simtime = entity->m_flSimulationTime();
		Vector hitboxPos = entity->GetHitboxPos(0);
		matrix3x4_t bones[128];
		entity->SetupBones(bones, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, m_globals()->m_curtime);

		for (int idx = 0; idx <= 128; ++idx)
			bt_data[i][cmd->m_command_number % 13].bone[idx] = bones[idx];

		headPositions[i][cmd->m_command_number % 13] = backtrackData{ simtime, hitboxPos };
		Vector ViewDir = angle_vector(cmd->m_viewangles + (local->m_aimPunchAngle() * 2.f));
		float FOVDistance = distance_point_to_line(hitboxPos, local->GetEyePos(), ViewDir);

		if (bestFov > FOVDistance)
		{
			bestFov = FOVDistance;
			bestTargetIndex = i;
		}
	}

	float bestTargetSimTime = 0;
	if (bestTargetIndex != -1)
	{
		float tempFloat = FLT_MAX;
		Vector ViewDir = angle_vector(cmd->m_viewangles + (local->m_aimPunchAngle() * 2.f));
		for (int t = 0; t < cfg::g_cfg.legitbot.backtrack_ticks / 10; ++t)
		{
			float tempFOVDistance = distance_point_to_line(headPositions[bestTargetIndex][t].hitboxPos, local->GetEyePos(), ViewDir);
			if (tempFloat > tempFOVDistance && headPositions[bestTargetIndex][t].simtime > local->m_flSimulationTime() - 1)
			{
				tempFloat = tempFOVDistance;
				bestTargetSimTime = headPositions[bestTargetIndex][t].simtime;
			}
		}
	}


	cmd->m_tickcount = TIME_TO_TICKS(bestTargetSimTime);
}