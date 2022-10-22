#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

struct Box;

class RadarPlayer_t
{
public:
	Vector pos;
	Vector angle;
	Vector spotted_map_angle_related;
	DWORD tab_related;
	char pad_0x0028[0xC];
	float spotted_time;
	float spotted_fraction;
	float time;
	char pad_0x0040[0x4];
	__int32 player_index;
	__int32 entity_index;
	char pad_0x004C[0x4];
	__int32 health;
	char name[32];
	char pad_0x0074[0x75];
	unsigned char spotted;
	char pad_0x00EA[0x8A];
};

class CCSGO_HudRadar
{
public:
	char pad_0x0000[0x14C];
	RadarPlayer_t radar_info[65];
};

class playeresp : public singleton <playeresp> 
{
public:
	int type = ENEMY;
	float esp_alpha_fade[65];
	int health[65];
	HPInfo hp_info[65];

	void paint_traverse();
	void draw_box(player_t* m_entity, const Box& box);
	void draw_health(player_t* m_entity, const Box& box, const HPInfo& hpbox);
	void draw_skeleton(player_t* e, matrix3x4_t matrix[MAXSTUDIOBONES]);
	bool draw_ammobar(player_t* m_entity, const Box& box);
	void draw_name(player_t* m_entity, const Box& box);
	void draw_weapon(player_t* m_entity, const Box& box, bool space);
	void draw_flags(player_t* e, const Box& box);
	void draw_multi_points(player_t* e);
	void fov_arrows(player_t* e, Color color);
	void shot_capsule();
};