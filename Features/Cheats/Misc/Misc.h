#include "..\..\includes.hpp"
#include "fakelag.h"
#include "..\ragebot\ragebot.h"
#include "..\visuals\WorldEsp.h"
#include "../Prediction/EnginePrediction.h"
#include "logs.h"
#include "..\ui.h"
#include "..\..\utils\render.h"

class misc : public singleton <misc> 
{
public:

	void infiniteduck_stamina(CUserCmd* cmd);
	void fakeduck(CUserCmd* cmd);
	void slidewalk(CUserCmd* cmd);
	void automatic_peek(CUserCmd* cmd, float wish_yaw);
	void ragdolls();
	void rank_reveal(CUserCmd* m_pcmd);
	void fast_stop(CUserCmd* m_pcmd);

	bool SwayBoolChanged = false;
};