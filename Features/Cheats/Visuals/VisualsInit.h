#pragma once

#include "..\..\hooks\hooks.hpp"
#include "..\..\Features\cheats\ui.h"
#include "..\..\Features\cheats\lagcompensation\LagCompensation.h"
#include "..\..\Features\cheats\visuals\PlayerEsp.h"
#include "..\..\Features\cheats\visuals\OtherEsp.h"
#include "..\..\Features\cheats\misc\logs.h"
#include "..\..\Features\cheats\visuals\WorldEsp.h"
#include "..\..\Features\cheats\misc\misc.h"
#include "..\..\Features\cheats\visuals\GrenadePrediction.h"
#include "..\..\Features\cheats\visuals\BulletTracers.h"
#include "..\..\Features\cheats\visuals\DormantEsp.h"
#include "..\..\Features\cheats\lagcompensation\LocalAnimations.h"

class visuals_init : public singleton<visuals_init>
{
public:
	void init();
private:
};