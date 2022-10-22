#include <Features\logging.h>
#include "..\hooks.hpp"
#include "..\..\Features\cheats\Prediction\EnginePrediction.h"
#include "..\..\Features\cheats\lagcompensation\LocalAnimations.h"
#include "..\..\Features\cheats\misc\misc.h"
#include "..\..\Features\cheats\misc\logs.h"

typedef void(__cdecl* clMove_fn)(float, bool);

void __cdecl hooks::hkClMove(float flAccumulatedExtraSamples, bool bFinalTick)
{
	/*
	Thanks kitten poo poo -PainFull
	If you want to move while recharging also with teleportion doubletap then recharge every other tick, This lets you move
	inbetween each tick charged. This will take double the charge amount tho to do!
	*/

	if (/*m_globals()->m_tickcount % 2 && */g_ctx.globals.startcharge && g_ctx.globals.tocharge < g_ctx.globals.tochargeamount)
	{
		g_ctx.globals.tocharge++;
		g_ctx.globals.ticks_allowed = g_ctx.globals.tocharge;
		return;
	}

	(clMove_fn(hooks::original_clmove)(flAccumulatedExtraSamples, bFinalTick));

	g_ctx.globals.isshifting = true;
	{
		for (g_ctx.globals.shift_ticks = min(g_ctx.globals.tocharge, g_ctx.globals.shift_ticks); g_ctx.globals.shift_ticks > 0; g_ctx.globals.shift_ticks--, g_ctx.globals.tocharge--)
		{
			Rbot::get().lastshifttime = m_globals()->m_realtime;
			(clMove_fn(hooks::original_clmove)(flAccumulatedExtraSamples, bFinalTick));
		}

		g_ctx.globals.shifted_tick = true;
	}
	g_ctx.globals.isshifting = false;
}