#pragma once

#include "..\Features/ImGui\imgui.h"
#include "..\Features/ImGui\imgui_internal.h"
#include "..\Features/ImGui\imgui_impl_dx9.h"
#include "..\Features/ImGui\imgui_impl_win32.h"

#include "..\Features\includes.hpp"
#include "..\sdk\interfaces\IBaseClientDll.hpp"
#include "..\sdk\interfaces\IClientMode.hpp"
#include "..\sdk\misc\CUserCmd.hpp"
#include "..\sdk\interfaces\IInputSystem.hpp"
#include "..\sdk\interfaces\IMDLCache.hpp"
#include "..\sdk\interfaces\IGameEventManager.hpp"
#include "..\Features\utils\util.hpp"

#include "VFuncHook.hpp"

class C_HookedEvents : public IGameEventListener2
{
public:
	void FireGameEvent(IGameEvent * event);
	void RegisterSelf();
	void RemoveSelf();
	int GetEventDebugID(void);
};

namespace INIT
{
	extern HMODULE Dll;
	extern HWND Window;
	extern WNDPROC OldWindow;
}

class c_baseplayeranimationstate;

namespace hooks
{
	extern bool menu_open;
	extern bool input_shouldListen;
	extern int rage_weapon;
	extern int legit_weapon;

	using GetForeignFallbackFontNameFn = const char*(__thiscall*)(void*);
	using SetupBonesFn = bool(__thiscall*)(void*, matrix3x4_t*, int, int, float);
	using DoExtraBonesProcessingFn = void(__thiscall*)(player_t*, CStudioHdr*, Vector*, Quaternion*, const matrix3x4_t&, uint8_t*, void*);
	using StandardBlendingRulesFn = void(__thiscall*)(player_t*, CStudioHdr*, Vector*, Quaternion*, float, int);
	using UpdateClientSideAnimationFn = void(__fastcall*)(player_t*);
	using PhysicsSimulateFn = void(__fastcall*)(player_t*);
	using ModifyEyePositionFn = void(__thiscall*)(c_baseplayeranimationstate*, Vector&);
	using CalcViewmodelBobFn = void(__thiscall*)(player_t*, Vector&);
	using ProcessInterpolatedListFn = int(*)(void);
	using ShouldSkipAnimFrameFn = bool(__thiscall*)(player_t*);

	// vmthook
	extern vmthook* directx_hook;
	extern vmthook* client_hook;
	extern vmthook* clientstate_hook;
	extern vmthook* engine_hook;
	extern vmthook* clientmode_hook;
	extern vmthook* inputinternal_hook;
	extern vmthook* renderview_hook;
	extern vmthook* modelcache_hook;
	extern vmthook* panel_hook;
	extern vmthook* materialsys_hook;
	extern vmthook* modelrender_hook;
	extern vmthook* surface_hook;
	extern vmthook* bspquery_hook;
	extern vmthook* game_movement_hook;
	extern vmthook* prediction_hook;
	extern vmthook* trace_hook;
	extern vmthook* filesystem_hook;
	extern vmthook* player_hook;
	extern vmthook* r_drawmodelstatsoverlay_hook;
	extern vmthook* netchan_hook;

	// Events
	extern ButtonCode_t* input_receivedKeyval;
	extern C_HookedEvents hooked_events;

	// DWORD
	extern DWORD original_getforeignfallbackfontname;
	extern DWORD original_setupbones;
	extern DWORD original_doextrabonesprocessing;
	extern DWORD original_standardblendingrules;
	extern DWORD original_updateclientsideanimation;
	extern DWORD original_physicssimulate;
	extern DWORD original_modifyeyeposition;
	extern DWORD original_calcviewmodelbob;
	extern DWORD original_processinterpolatedlist;
	extern DWORD original_clmove;
	extern DWORD original_shouldskipanimframe;

	// __stdcall
	void __stdcall hkFrameStageNotify(ClientFrameStage_t);
	//void __stdcall hooked_createmove(int sequence_number, float input_sample_frametime, bool active, bool& send_packet);
	void __stdcall hkCreatemove_Proxy(int, float, bool);
	void __stdcall hkOverrideview(CViewSetup* setup);
	void __stdcall hkDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone_to_world);
	void __stdcall hkLockCursor();
	bool __stdcall hkInPrediction();

	void __cdecl hkClMove(float flAccumulatedExtraSamples, bool bFinalTick);

	LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	long __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice);
	long __stdcall Hooked_EndScene_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	long __stdcall hooked_present(IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region);

	// __stdcall
	//void __stdcall hooked_fsn(ClientFrameStage_t);
	//void __stdcall hooked_createmove_naked(int, float, bool);
	//void __stdcall hooked_overrideview(CViewSetup* setup);
	//void __stdcall hooked_dme(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone_to_world);
	//void __stdcall hooked_lockcursor();
	//bool __stdcall hooked_inprediction();
	//LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//long __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice);
	//long __stdcall Hooked_EndScene_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	//long __stdcall hooked_present(IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region);

	typedef long(__stdcall* EndSceneFn)(IDirect3DDevice9* device);
	typedef long(__stdcall* EndSceneResetFn)(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	typedef long(__stdcall* PresentFn)(IDirect3DDevice9*, RECT*, RECT*, HWND, RGNDATA*);

	// noinline
	_declspec(noinline)const char* hkGetForignFallbackfontname_detour(void* ecx, uint32_t i);
	_declspec(noinline)bool hkSetupBones_detour(void* ecx, matrix3x4_t* bone_world_out, int max_bones, int bone_mask, float current_time);
	_declspec(noinline)void hkDoExtraBonesProcessing_detour(player_t* player, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context);
	_declspec(noinline)void hkStandardBlendingRules_detour(player_t* player, int i, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask);
	_declspec(noinline)void hkUpdateClientsideAnimation_detour(player_t* player);
	_declspec(noinline)void hkPhysicssimulate_detour(player_t* player);
	_declspec(noinline)void hkModifyEyePosition_detour(c_baseplayeranimationstate* state, Vector& position);
	_declspec(noinline)void hkCalcviewmodelbob_detour(player_t* player, Vector& position);

	// __fastcall
	bool __fastcall hkDrawFog(void* ecx, void* edx);
	bool __fastcall hkIsConnected(void* ecx, void* edx);
	float __fastcall hkGetScreenAspectRatio(void* ecx, void* edx, int width, int height);
	bool __fastcall hkIsHLTV(void* ecx, void* edx);
	void  __fastcall hkPostScreenEffects(void* thisptr, void* edx, CViewSetup* setup);
	bool FASTCALL hkSvCheatsGetBool(ConVar* thisptr, int edx);
	void __fastcall hkSetKeyCodeState(void* thisptr, void* edx, ButtonCode_t code, bool bDown);
	void __fastcall hkSetMouseCodeState(void* thisptr, void* edx, ButtonCode_t code, MouseCodeState_t state);
	void __fastcall hkSceneend(void* ecx, void* edx);
	void __fastcall hkFindMdl(void* ecx, void* edx, char* FilePath);
	void __fastcall hkPaintTraverse(void* ecx, void* edx, vgui::VPANEL panel, bool force_repaint, bool allow_force);
	void __fastcall hkBeginFrame(void* ecx, void* edx, float ft);
	const char* __fastcall hkGetForignFallbackfontname(void* ecx, uint32_t i);
	bool __fastcall hkSetupBones(void* ecx, void* edx, matrix3x4_t* bone_world_out, int max_bones, int bone_mask, float current_time);
	void __fastcall hkDoExtraBonesProcessing(player_t* player, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context);
	void __fastcall hkStandardBlendingRules(player_t* player, int i, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask);
	void __fastcall hkUpdateClientsideAnimation(player_t* player, uint32_t i);
	void __fastcall hkPhysicssimulate(player_t* player);
	void __fastcall hkModifyEyePosition(c_baseplayeranimationstate* state, void* edx, Vector& position);
	void __fastcall hkCalcviewmodelbob(player_t* player, void* edx, Vector& position);
	bool __fastcall hkShouldSkipAnimFrame(player_t* ecx, void* edx);
	IMaterial* __fastcall hkGetMaterial(void* ecx, void* edx, const char* material_name, const char* texture_group_name, bool complain, const char* complain_prefix);
	void __fastcall hkPacketstart(void* ecx, void* edx, int incoming, int outgoing);
	void __fastcall hkPacketend(void* ecx, void* edx);
	int __fastcall hkSendDataGram(void* netchan, void*, void* datagram);
	int __fastcall hkListLeavesinbox(void* ecx, void* edx, Vector& mins, Vector& maxs, unsigned short* list, int list_max);
	void __fastcall	hooked_processmovement(void* ecx, void* edx, player_t* ent, CMoveData* data);
	void __fastcall hkRunCommand(void* ecx, void* edx, player_t* player, CUserCmd* m_pcmd, IMoveHelper* move_helper);
	bool __fastcall hkWriteUserCmdDeltaToBuffer(void* ecx, void* edx, int slot, bf_write* buf, int from, int to, bool is_new_command);
	void __fastcall hkClipRayCollideable(void* ecx, void* edx, const Ray_t& ray, uint32_t fMask, ICollideable* pCollide, CGameTrace* pTrace);
	void __fastcall hkTraceRay(void* ecx, void* edx, const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace);
	bool __fastcall hkLooseFileAllowed(void* ecx, void* edx);
	void __fastcall hkCheckFileCrcsWithServer(void* ecx, void* edx);
	int __fastcall hkDrawModelStatsOverlay(ConVar* thisptr, void* edx);

	// __fastcall
	//bool __fastcall hooked_drawfog(void* ecx, void* edx);
	//bool __fastcall hooked_isconnected(void* ecx, void* edx);
	//float __fastcall hooked_getscreenaspectratio(void* ecx, void* edx, int width, int height);
	//bool __fastcall hooked_ishltv(void* ecx, void* edx);
	//void  __fastcall hooked_postscreeneffects(void * thisptr, void * edx, CViewSetup * setup);
	//void __fastcall hooked_setkeycodestate(void* thisptr, void* edx, ButtonCode_t code, bool bDown);
	//void __fastcall hooked_setmousecodestate(void* thisptr, void* edx, ButtonCode_t code, MouseCodeState_t state);
	//void __fastcall hooked_sceneend(void* ecx, void* edx);
	//void __fastcall hooked_findmdl(void* ecx, void* edx, char* FilePath);
	//void __fastcall hooked_painttraverse(void* ecx, void* edx, vgui::VPANEL panel, bool force_repaint, bool allow_force);
	//void __fastcall hooked_beginframe(void* ecx, void* edx, float ft);
	//const char* __fastcall hooked_getforeignfallbackfontname(void* ecx, uint32_t i);
	//bool __fastcall hooked_setupbones(void* ecx, void* edx, matrix3x4_t* bone_world_out, int max_bones, int bone_mask, float current_time);
	//void __fastcall hooked_doextrabonesprocessing(player_t* player, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context);
	//void __fastcall hooked_standardblendingrules(player_t* player, int i, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask);
	//void __fastcall hooked_updateclientsideanimation(player_t* player, uint32_t i);
	//void __fastcall hooked_physicssimulate(player_t* player);
	//void __fastcall hooked_modifyeyeposition(c_baseplayeranimationstate* state, void* edx, Vector& position);
	//void __fastcall hooked_calcviewmodelbob(player_t* player, void* edx, Vector& position);
	//bool __fastcall hooked_shouldskipanimframe();
	//IMaterial* __fastcall hooked_getmaterial(void* ecx, void* edx, const char* material_name, const char* texture_group_name, bool complain, const char* complain_prefix);
	//void __fastcall hooked_packetstart(void* ecx, void* edx, int incoming, int outgoing);
	//void __fastcall hooked_packetend(void* ecx, void* edx);
	//int __fastcall hooked_listleavesinbox(void* ecx, void* edx, Vector& mins, Vector& maxs, unsigned short* list, int list_max);
	//void __fastcall hooked_runcommand(void* ecx, void* edx, player_t* player, CUserCmd* m_pcmd, IMoveHelper* move_helper);
	//bool __fastcall hooked_writeusercmddeltatobuffer(void* ecx, void* edx, int slot, bf_write* buf, int from, int to, bool is_new_command);
	//void __fastcall hooked_clip_ray_collideable(void* ecx, void* edx, const Ray_t& ray, uint32_t fMask, ICollideable* pCollide, CGameTrace* pTrace);
	//void __fastcall hooked_trace_ray(void* ecx, void* edx, const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace);
	//bool __fastcall hooked_loosefileallowed(void* ecx, void* edx);
	//void __fastcall hooked_checkfilecrcswithserver(void* ecx, void* edx);
	//int __fastcall hooked_r_drawmodelstatsoverlay(ConVar* thisptr, void* edx);

	// __thiscall
	typedef void(__thiscall* SetKeyCodeState_t) (void*, ButtonCode_t, bool);
	extern SetKeyCodeState_t o_SetKeyCodeState;
	typedef void(__thiscall* SetMouseCodeState_t) (void*, ButtonCode_t, MouseCodeState_t);
	extern SetMouseCodeState_t o_SetMouseCodeState;

	// default
	int hkProcessInterpolatedList();
	void GUI_Init(IDirect3DDevice9* pDevice);
}

extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);