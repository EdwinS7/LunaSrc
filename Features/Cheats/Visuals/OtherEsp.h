#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"
struct m_indicator
{
	std::string m_text;
	Color m_color;

	m_indicator(const char* text, Color color) :
		m_text(text), m_color(color)
	{

	}
	m_indicator(std::string text, Color color) :
		m_text(text), m_color(color)
	{

	}
};
class otheresp : public singleton< otheresp >
{
	float radius = 25.f;
	float rotation_step = 0.06f;
	Vector Direction_Vector = Vector(0, 0, 0);
	float current_rotation = 0.0f;
	Vector current_peek_position = Vector(0, 0, 0);

public:
	bool CanPenetrate(weapon_t* weapon);
	void PenetrationCrosshair();
	void HitMarker();
	void AutoPeekIndicator();
	void CrosshairIndicators();

	int indicators_on = 0;
	int total_indicators_on = 0;
	struct Hitmarker
	{
		float hurt_time_world = FLT_MIN;
		float hurt_time = FLT_MIN;
		Color hurt_color = Color::White;
		Vector point = ZERO;
	} hitmarker;

	struct Damage_marker
	{
		Vector position = ZERO;
		float hurt_time = FLT_MIN;
		Color hurt_color = Color::White;
		int damage = -1;
		int hitgroup = -1;
		float animation = 0;

		void reset()
		{
			position.Zero();
			hurt_time = FLT_MIN;
			hurt_color = Color::White;
			damage = -1;
			hitgroup = -1;
		}
	} damage_marker[65];

	std::vector<m_indicator> m_indicators;
};