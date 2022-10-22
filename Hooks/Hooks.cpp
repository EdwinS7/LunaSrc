// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "hooks.hpp"

#include <tchar.h>
#include <iostream>
#include <d3d9.h>
#include <dinput.h>

#include "..\Features\cheats\misc\logs.h"
#include "..\Features\cheats\misc\misc.h"
#include "..\Features\cheats\visuals\OtherEsp.h"
#include "../Features/ImGui/imgui_freetype.h"

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment(lib, "freetype.lib")

#include <shlobj.h>
#include <shlwapi.h>
#include <thread>
#include "..\Features\cheats\ui.h"
#include "..\Features\cheats\visuals\BulletTracers.h"
#include "..\Features\utils\render.h"

auto _visible = true;

namespace INIT
{
	HMODULE Dll;
	HWND Window;
	WNDPROC OldWindow;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//extern IMGUI_IMPL_API LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace hooks
{
	int rage_weapon = 0;
	int legit_weapon = 0;
	bool menu_open = true; //default menu open state.
	bool input_shouldListen = false;

	ButtonCode_t* input_receivedKeyval;

	LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{


		if (uMsg == WM_KEYDOWN)
			if (wParam == VK_INSERT)
			{
				menu_open = !menu_open;

				if (menu_open && g_ctx.available())
				{
					if (g_ctx.globals.current_weapon != -1)
					{
						if (cfg::g_cfg.ragebot.enable)
							rage_weapon = g_ctx.globals.current_weapon;
						else if (cfg::g_cfg.legitbot.enabled)
							legit_weapon = g_ctx.globals.current_weapon;
					}
				}
			}

		if (menu_open)
		{
			ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);


			auto pressed_buttons = false;
			auto pressed_menu_key = uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_MOUSEWHEEL || GetAsyncKeyState(VK_RETURN);
			// GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN)

			if (!pressed_menu_key && !g_ctx.globals.focused_on_input)
				pressed_buttons = true;

			if (!pressed_buttons && menu_open)
				return true;//blocks everything besides the menu

			if (menu_open && (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_MOUSEMOVE))
				return false;
		}


		return CallWindowProc(INIT::OldWindow, hWnd, uMsg, wParam, lParam);
	}

	long __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice)
	{
		static auto original_fn = directx_hook->get_func_address <EndSceneFn> (42);
		return original_fn(pDevice);
	}


	long __stdcall hooked_present(IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region)
	{
		static auto original = directx_hook->get_func_address <PresentFn>(17);
		g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true); //Crash on turning a buybot option here.

		renderer::get().gui_init(device);

		IDirect3DVertexDeclaration9* vert_dec;
		if (device->GetVertexDeclaration(&vert_dec))
			return original(device, src_rect, dest_rect, dest_wnd_override, dirty_region);

		IDirect3DVertexShader9* vert_shader;
		if (device->GetVertexShader(&vert_shader))
			return original(device, src_rect, dest_rect, dest_wnd_override, dirty_region);

		CMenu::get().device = device;
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();


		ImGui::NewFrame();

		//main
		CMenu::get().Draw(menu_open);
	
		//Actually do all the drawing here.
		renderer::get().scene_add();

		ImGui::EndFrame();
		ImGui::Render();

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->SetVertexShader(vert_shader);
		device->SetVertexDeclaration(vert_dec);

		return original(device, src_rect, dest_rect, dest_wnd_override, dirty_region);
	}

	long __stdcall Hooked_EndScene_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto ofunc = directx_hook->get_func_address<EndSceneResetFn>(16);

		if (!renderer::get().d3d_init)
			return ofunc(pDevice, pPresentationParameters);

		renderer::get().invalidate_objects();

		auto hr = ofunc(pDevice, pPresentationParameters);

		if (SUCCEEDED(hr))
			renderer::get().create_objects(pDevice);

		return hr;
	}

	// vmthook
	vmthook* directx_hook;
	vmthook* client_hook;
	vmthook* clientstate_hook;
	vmthook* engine_hook;
	vmthook* clientmode_hook;
	vmthook* inputinternal_hook;
	vmthook* renderview_hook;
	vmthook* panel_hook;
	vmthook* modelcache_hook;
	vmthook* materialsys_hook;
	vmthook* modelrender_hook;
	vmthook* surface_hook;
	vmthook* bspquery_hook;
	vmthook* prediction_hook;
	vmthook* game_movement_hook;
	vmthook* trace_hook;
	vmthook* filesystem_hook;
	vmthook* player_hook;
	vmthook* r_drawmodelstatsoverlay_hook;
	vmthook* netchan_hook;


	// Events
	C_HookedEvents hooked_events;

	// DWORD
	DWORD original_getforeignfallbackfontname;
	DWORD original_setupbones;
	DWORD original_doextrabonesprocessing;
	DWORD original_standardblendingrules;
	DWORD original_updateclientsideanimation;
	DWORD original_physicssimulate;
	DWORD original_modifyeyeposition;
	DWORD original_calcviewmodelbob;
	DWORD original_processinterpolatedlist;
	DWORD original_clmove;
	DWORD original_shouldskipanimframe;
}

void __fastcall hooks::hkSetKeyCodeState(void* thisptr, void* edx, ButtonCode_t code, bool bDown)
{
	static auto original_fn = inputinternal_hook->get_func_address <SetKeyCodeState_t> (91);

	if (input_shouldListen && bDown)
	{
		input_shouldListen = false;

		if (input_receivedKeyval)
			*input_receivedKeyval = code;
	}

	return original_fn(thisptr, code, bDown);
}

void __fastcall hooks::hkSetMouseCodeState(void* thisptr, void* edx, ButtonCode_t code, MouseCodeState_t state)
{
	static auto original_fn = inputinternal_hook->get_func_address <SetMouseCodeState_t> (92);

	if (input_shouldListen && state == BUTTON_PRESSED)
	{
		input_shouldListen = false;

		if (input_receivedKeyval)
			*input_receivedKeyval = code;
	}

	return original_fn(thisptr, code, state);
}