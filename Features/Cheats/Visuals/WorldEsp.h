#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

class weapon_t;
class Box;

struct client_hit_verify_t
{
	Vector position;
	float time;
	float expires;
};

class worldesp : public singleton <worldesp> 
{
public:
	void paint_traverse();
	void skybox_changer();
	void draw_client_impacts();
	void fog_changer();
	void world_modulation(entity_t* entity);
	void molotov_timer(entity_t* entity);
	void smoke_timer(entity_t* entity);
	void grenade_projectiles(entity_t* entity);
	void bomb_timer(entity_t* entity);
	void dropped_weapons(entity_t* entity);
	void sunset_mode();
	void viewmodel_changer();

	bool changed = false;
	std::string backup_skybox = "";
private:
	int32_t m_iLastProcessedImpact = 0;
};