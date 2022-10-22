#pragma once
#include "..\..\includes.hpp"

struct hitmarker_t
{
	hitmarker_t(const float& time, const int& index, const int& damage, const int& hitgroup, const Vector& pos)
	{
		this->time = time;
		this->index = index;
		this->damage = damage;
		this->hitgroup = hitgroup;
		this->pos = pos;
		moved = 0.f;
		alpha = 255.f;
	}
	float time;
	int index;
	int damage;
	int hitgroup;
	float moved;
	float alpha;
	Color col;
	Vector pos;
};

class hitmarker : public singleton<hitmarker>
{
public:
	void DrawDamageIndicator();
	void listener(IGameEvent* game_event);
	void draw_hits();
	void add_hit(hitmarker_t hit);

	std::deque<hitmarker_t> hits;

	int damage;
	bool initialized;
	float erase_time;
	float last_update;
	player_t* player;
	int hit_box;
	Color col = cfg::g_cfg.visuals.damage_marker_color;
	Vector position;
	Vector end_position;
};