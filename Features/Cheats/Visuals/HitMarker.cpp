#include "..\..\includes.hpp"
#include "HitMarker.h"
#include "..\..\utils\render.h"

player_t* sget_entity(const int index) { return reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(index)); }

void hitmarker::listener(IGameEvent* game_event)
{
	const auto attacker = m_engine()->GetPlayerForUserID(game_event->GetInt("attacker"));

	const auto victim = m_engine()->GetPlayerForUserID(game_event->GetInt("userid"));

	if (attacker != m_engine()->GetLocalPlayer())
		return;

	if (victim == m_engine()->GetLocalPlayer())
		return;

	const auto player = sget_entity(victim);
	if (!player || (player->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && !m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1))
		return;

}
int bones(int event_bone)
{
	switch (event_bone)
	{
	case 1:
		return CSGOHitboxID::Head;
	case 2:
		return CSGOHitboxID::Chest;
	case 3:
		return CSGOHitboxID::Stomach;
	case 4:
		return CSGOHitboxID::LeftHand;
	case 5:
		return CSGOHitboxID::RightHand;
	case 6:
		return CSGOHitboxID::RightCalf;
	case 7:
		return CSGOHitboxID::LeftCalf;
	default:
		return CSGOHitboxID::Pelvis;
	}
}
void hitmarker::DrawDamageIndicator()////////
{
	float CurrentTime = m_globals()->m_curtime;

	if (erase_time < CurrentTime)
		return;


	if (erase_time - 2.7f < CurrentTime) {//fade
		col._CColor[3] = math::lerp(col.a(), 0, 0.05f);
	}

	if (!initialized) { //location
		position = player->hitbox_position(bones(hit_box));
		initialized = true;
	}

	Vector ScreenPosition;
	std::string txtdamage = std::to_string(damage);

	if (math::world_to_screen(position, ScreenPosition))
		renderer::get().text(renderer::get().damage, ScreenPosition.x, ScreenPosition.y - 23, col, true, false, false, true, txtdamage.c_str());
}
void hitmarker::draw_hits()
{
	DrawDamageIndicator();

}

void hitmarker::add_hit(const hitmarker_t hit)
{
	hits.push_back(hit);
}


