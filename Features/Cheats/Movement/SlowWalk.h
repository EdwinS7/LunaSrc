#pragma once

#include "..\..\includes.hpp"
#include "..\Prediction\EnginePrediction.h"

class slowwalk : public singleton <slowwalk> 
{
public:
	void create_move(CUserCmd* m_pcmd, float custom_speed = -1.0f);
};