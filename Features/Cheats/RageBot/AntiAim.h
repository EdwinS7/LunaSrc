#pragma once

#include "..\ragebot\penetration.h"
#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

class antiaim : public singleton <antiaim>
{
public:
	void RunAntiaim(CUserCmd* m_pcmd);
	float GetPitch(CUserCmd* m_pcmd);
	float GetYawDirection(CUserCmd* m_pcmd);
	float GetYaw(CUserCmd* m_pcmd);
	bool CanAntiAim(CUserCmd* m_pcmd, bool dynamic_check = true);
	bool ShouldBreakLowerBody(CUserCmd* m_pcmd, int lby_type);
	float GetAtTargetAngles();
	void Freestanding(CUserCmd* m_pcmd);

	float local_pitch = 0;
	int type = 0;
	int manual_side = -1;
	int final_manual_side = -1;
	bool flip = false;
	bool freeze_check = false;
	bool breaking_lby = false;
	float desync_angle = 0.0f;
};

enum 
{
	SIDE_NONE = -1,
	SIDE_BACK,
	SIDE_LEFT,
	SIDE_RIGHT
};

struct angle_data {
	float angle;
	float thickness;
	angle_data(const float angle, const float thickness) : angle(angle), thickness(thickness) {}
};