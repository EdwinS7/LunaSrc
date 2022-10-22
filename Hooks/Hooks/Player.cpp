// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\Features\cheats\lagcompensation\LocalAnimations.h"
#include "..\..\Features\cheats\Prediction\EnginePrediction.h"

_declspec(noinline)bool hooks::hkSetupBones_detour(void* ecx, matrix3x4_t* bone_world_out, int max_bones, int bone_mask, float current_time)
{
	auto result = true;

	static auto r_jiggle_bones = m_cvar()->FindVar(crypt_str("r_jiggle_bones"));
	auto r_jiggle_bones_backup = r_jiggle_bones->GetInt();

	r_jiggle_bones->SetValue(0);

	if (!ecx)
		result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
	else if (!cfg::g_cfg.ragebot.enable && !cfg::g_cfg.legitbot.enabled)
		result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
	else
	{
		auto player = (player_t*)((uintptr_t)ecx - 0x4);

		if (!player->valid(false, false))
			result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
		else
		{
			auto animstate = player->get_animation_state();
			auto previous_weapon = animstate ? animstate->m_pLastBoneSetupWeapon : nullptr;

			if (previous_weapon)
				animstate->m_pLastBoneSetupWeapon = animstate->m_pActiveWeapon; //-V1004

			if (g_ctx.globals.setuping_bones)
				result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (cfg::g_cfg.legitbot.enabled && player != g_ctx.local())
				result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (!g_ctx.local()->is_alive())
				result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (player == g_ctx.local())
				result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (!player->m_CachedBoneData().Count())
				result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (bone_world_out && max_bones != -1)
				memcpy(bone_world_out, player->m_CachedBoneData().Base(), player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));

			if (previous_weapon)
				animstate->m_pLastBoneSetupWeapon = previous_weapon;
		}
	}

	r_jiggle_bones->SetValue(r_jiggle_bones_backup);
	return result;
}

bool __fastcall hooks::hkSetupBones(void* ecx, void* edx, matrix3x4_t* bone_world_out, int max_bones, int bone_mask, float current_time)
{
	return hkSetupBones_detour(ecx, bone_world_out, max_bones, bone_mask, current_time);
}

_declspec(noinline)void hooks::hkStandardBlendingRules_detour(player_t* player, int i, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask)
{
	auto backup_effects = player->m_fEffects();

	if (player == g_ctx.local())
		player->m_fEffects() |= 8;

	((StandardBlendingRulesFn)original_standardblendingrules)(player, hdr, pos, q, curtime, boneMask);

	if (player == g_ctx.local())
		player->m_fEffects() = backup_effects;
}

void __fastcall hooks::hkStandardBlendingRules(player_t* player, int i, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask)
{
	return hkStandardBlendingRules_detour(player, i, hdr, pos, q, curtime, boneMask);
}

_declspec(noinline)void hooks::hkDoExtraBonesProcessing_detour(player_t* player, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context)
{

}

void __fastcall hooks::hkDoExtraBonesProcessing(player_t* player, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context)
{
	return hkDoExtraBonesProcessing_detour(player, hdr, pos, q, matrix, bone_list, context);
}

_declspec(noinline)void hooks::hkUpdateClientsideAnimation_detour(player_t* player)
{
	if (g_ctx.globals.updating_skins)
		return;

	if (g_ctx.globals.updating_animation)
		return ((UpdateClientSideAnimationFn)original_updateclientsideanimation)(player);

	if (player == g_ctx.local())
		return ((UpdateClientSideAnimationFn)original_updateclientsideanimation)(player);

	if (!cfg::g_cfg.ragebot.enable && !cfg::g_cfg.legitbot.enabled)
		return ((UpdateClientSideAnimationFn)original_updateclientsideanimation)(player);

	if (!player->valid(false, false))
		return ((UpdateClientSideAnimationFn)original_updateclientsideanimation)(player);
}


void __fastcall hooks::hkUpdateClientsideAnimation(player_t* player, uint32_t i)
{
	return hkUpdateClientsideAnimation_detour(player);
}

_declspec(noinline)void hooks::hkPhysicssimulate_detour(player_t* player)
{
	auto simulation_tick = *(int*)((uintptr_t)player + 0x2AC);

	if (player != g_ctx.local() || !g_ctx.local()->is_alive() || m_globals()->m_tickcount == simulation_tick)
	{
		((PhysicsSimulateFn)original_physicssimulate)(player);
		return;
	}

	engineprediction::get().restore_netvars();
	((PhysicsSimulateFn)original_physicssimulate)(player);
	engineprediction::get().store_netvars();
}

void __fastcall hooks::hkPhysicssimulate(player_t* player)
{
	return hkPhysicssimulate_detour(player);
}

_declspec(noinline)void hooks::hkModifyEyePosition_detour(c_baseplayeranimationstate* state, Vector& position)
{
	if (state && g_ctx.globals.in_createmove)
		return ((ModifyEyePositionFn)original_modifyeyeposition)(state, position);
}

void __fastcall hooks::hkModifyEyePosition(c_baseplayeranimationstate* state, void* edx, Vector& position)
{
	return hkModifyEyePosition_detour(state, position);
}

_declspec(noinline)void hooks::hkCalcviewmodelbob_detour(player_t* player, Vector& position)
{
	if (!cfg::g_cfg.visuals.removals[REMOVALS_LANDING_BOB] || player != g_ctx.local() || !g_ctx.local()->is_alive())
		return ((CalcViewmodelBobFn)original_calcviewmodelbob)(player, position);
}

void __fastcall hooks::hkCalcviewmodelbob(player_t* player, void* edx, Vector& position)
{
	return hkCalcviewmodelbob_detour(player, position);
}

bool __fastcall hooks::hkShouldSkipAnimFrame(player_t* ecx, void* edx)
{
	if (!g_ctx.available() || !g_ctx.local()->is_alive() || ecx != g_ctx.local())
		return ((ShouldSkipAnimFrameFn)hooks::original_shouldskipanimframe)(ecx);

	return false;
}

int hooks::hkProcessInterpolatedList()
{
	static auto allow_extrapolation = *(bool**)(util::FindSignature(CLIENT_DLL, crypt_str("A2 ? ? ? ? 8B 45 E8")) + 0x1);

	if (allow_extrapolation)
		*allow_extrapolation = false;

	return ((ProcessInterpolatedListFn)original_processinterpolatedlist)();
}