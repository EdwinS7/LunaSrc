#pragma once
#include "..\..\includes.hpp"
#include "..\lagcompensation\LagCompensation.h"
#include "ragebot.h"

class knifebot : public singleton <knifebot>
{
	void scan_targets();
	void fire(CUserCmd* cmd);
	int determinate_hit_type(bool stab_type, const Vector& delta);

	scanned_target final_target;
public:
	void run(CUserCmd* cmd);
};