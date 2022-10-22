// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"

using SceneEnd_t = void(__thiscall*)(void*);

void __fastcall hooks::hkSceneend(void* ecx, void* edx)
{
	static auto original_fn = renderview_hook->get_func_address <SceneEnd_t>(9);
	return original_fn(ecx);
}