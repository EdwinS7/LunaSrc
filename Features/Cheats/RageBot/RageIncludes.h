#pragma once
#include "..\misc\misc.h"
#include "..\misc\logs.h"
#include "..\ragebot\penetration.h"
#include "..\Prediction\EnginePrediction.h"
#include "..\lagcompensation\LocalAnimations.h"
#include "..\..\sdk\math\Vector.hpp"
#include "..\..\logging.h"

namespace RageConfig {
	bool enable;
	bool zeus_bot;
	bool knife_bot;
	bool autoshoot;
	bool extrapolation;
	bool headshot_only;
	bool accurate_fd;
	bool dormant_aimbot;
	int charge_time;
	bool double_tap;
	std::vector <int> exploit_modifiers;
	bool force_safe;
	bool force_body;
	bool damage_override;
	bool resolver;
	bool roll_resolver;

	struct {
		int dt_hitchance;
		int hitchance;
		int min_dmg;
		int min_override_dmg;
		std::vector <int> hitboxes;
		float head_scale;
		float body_scale;
		std::vector <int> multipoints;
		bool prefer_baim;
		int prefer_baim_mode;
		bool autostop;
		bool autoscope;
		int autoscope_mode;

		std::vector <int> autostop_modifiers;

		int targeting_mode;
	} weapon[9];
}