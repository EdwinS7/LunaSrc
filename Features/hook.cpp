#include "hook.h"

void hook::setup_sigs() {
	{
		g_ctx.signatures = {
		XorStr("A1 ? ? ? ? 50 8B 08 FF 51 0C"),
		XorStr("B9 ?? ?? ?? ?? A1 ?? ?? ?? ?? FF 10 A1 ?? ?? ?? ?? B9"),
		XorStr("0F 11 05 ?? ?? ?? ?? 83 C8 01"),
		XorStr("8B 0D ?? ?? ?? ?? 8B 46 08 68"),
		XorStr("B9 ? ? ? ? F3 0F 11 04 24 FF 50 10"),
		XorStr("8B 3D ? ? ? ? 85 FF 0F 84 ? ? ? ? 81 C7"),
		XorStr("A1 ? ? ? ? 8B 0D ? ? ? ? 6A 00 68 ? ? ? ? C6"),
		XorStr("80 3D ? ? ? ? ? 53 56 57 0F 85"),
		XorStr("55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C"),
		XorStr("80 3D ? ? ? ? ? 74 06 B8"),
		XorStr("55 8B EC 83 E4 F0 B8 D8"),
		XorStr("55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C"),
		XorStr("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6"),
		XorStr("55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36"),
		XorStr("56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 23"),
		XorStr("55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 14 83 7F 60"),
		XorStr("55 8B EC A1 ? ? ? ? 83 EC 10 56 8B F1 B9"),
		XorStr("57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02"),
		XorStr("55 8B EC 81 EC ? ? ? ? 53 8B D9 89 5D F8"),
		XorStr("53 0F B7 1D ? ? ? ? 56"),
		XorStr("8B 0D ? ? ? ? 8D 95 ? ? ? ? 6A 00 C6"),
		XorStr("8B 35 ? ? ? ? FF 10 0F B7 C0")
		};
		g_ctx.indexes = {
			5,
			33,
			340,
			219,
			220,
			34,
			158,
			75,
			461,
			483,
			453,
			484,
			285,
			224,
			247,
			27,
			17,
			123
		};
	}
}

void hook::setup_files() {
	CreateDirectory("luna", nullptr);
	CreateDirectory("luna\\configs", nullptr);
	CreateDirectory("luna\\scripts", nullptr);
	CreateDirectory("luna\\skyboxes", nullptr);
	CreateDirectory("luna\\hitsounds", nullptr);
}

void hook::setup_netvars()
{
	netvars::get().tables.clear();
	auto client = m_client()->GetAllClasses();

	if (!client)
		return;

	while (client)
	{
		auto recvTable = client->m_pRecvTable;

		if (recvTable)
			netvars::get().tables.emplace(std::string(client->m_pNetworkName), recvTable);

		client = client->m_pNext;
	}
}

void hook::setup_skins()
{
	auto items = std::ifstream(XorStr("csgo/scripts/items/items_game_cdn.txt"));
	auto gameItems = std::string(std::istreambuf_iterator <char> { items }, std::istreambuf_iterator <char> { });

	if (!items.is_open())
		return;

	items.close();
	memory.initialize();

	for (auto i = 0; i <= memory.itemSchema()->paintKits.lastElement; i++)
	{
		auto paintKit = memory.itemSchema()->paintKits.memory[i].value;

		if (paintKit->id == 9001)
			continue;

		auto itemName = m_localize()->FindSafe(paintKit->itemName.buffer + 1);
		auto itemNameLength = WideCharToMultiByte(CP_UTF8, 0, itemName, -1, nullptr, 0, nullptr, nullptr);

		if (std::string name(itemNameLength, 0); WideCharToMultiByte(CP_UTF8, 0, itemName, -1, &name[0], itemNameLength, nullptr, nullptr))
		{
			if (paintKit->id < 10000)
			{
				SkinChanger::skinKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);

				//Show names for weapons.
				if (auto pos = gameItems.find('_' + std::string{ paintKit->name.buffer } + '='); pos != std::string::npos && gameItems.substr(pos + paintKit->name.length).find('_' + std::string{ paintKit->name.buffer } + '=') == std::string::npos)
				{
					if (auto weaponName = gameItems.rfind(XorStr("weapon_"), pos); weaponName != std::string::npos)
					{
						name.back() = ' ';
						name += '(' + gameItems.substr(weaponName + 7, pos - weaponName - 7) + ')';
					}
				}

				SkinChanger::skinKits2.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);
			}
			else
			{
				std::string original_name = name;

				std::string_view gloveName{ paintKit->name.buffer };
				//name.back() = ' ';
				name += std::string{ gloveName.substr(0, gloveName.find('_')) };
				SkinChanger::gloveKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);

				std::string_view gloveName2{ paintKit->name.buffer };
				original_name.back() = ' ';
				original_name += '(' + std::string{ gloveName2.substr(0, gloveName2.find('_')) } + ')';
				SkinChanger::gloveKits2.emplace_back(paintKit->id, std::move(original_name), paintKit->name.buffer);
			}
		}
	}

	std::sort(SkinChanger::skinKits.begin(), SkinChanger::skinKits.end());
	std::sort(SkinChanger::skinKits2.begin(), SkinChanger::skinKits2.end());
	std::sort(SkinChanger::gloveKits.begin(), SkinChanger::gloveKits.end());
	std::sort(SkinChanger::gloveKits2.begin(), SkinChanger::gloveKits2.end());
}

using mother_fucker_valve = Vector * (__thiscall*)(void*);
inline mother_fucker_valve _yes_fuck;

void* pBuildTransformationsEntity;
Vector vecBuildTransformationsAngles;

Vector* __fastcall hkGetEyeAngles(void* ecx, void* edx)
{
	static int* WantedReturnAddress1 = (int*)util::FindSignature("client.dll", "8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55 ?"); //Update Animations X/Y
	static int* WantedReturnAddress2 = (int*)util::FindSignature("client.dll", "F3 0F 10 55 ? 51 8B 8E ? ? ? ?");                                    //Update Animations X/Y
	static int* WantedReturnAddress3 = (int*)util::FindSignature("client.dll", "8B 55 0C 8B C8 E8 ? ? ? ? 83 C4 08 5E 8B E5");                       //Retarded valve fix

	static auto oGetEyeAngles = hooks::player_hook->get_func_address <mother_fucker_valve>(170);

	if (_ReturnAddress() == (void*)WantedReturnAddress1 || _ReturnAddress() == (void*)WantedReturnAddress2 || _ReturnAddress() == (void*)WantedReturnAddress3)
		return oGetEyeAngles(ecx);

	if (!ecx || pBuildTransformationsEntity != ecx)
		return oGetEyeAngles(ecx);

	pBuildTransformationsEntity = nullptr;

	return &vecBuildTransformationsAngles;
}

using BuildTransformations = Vector * (__thiscall*)(void*, CStudioHdr* hdr, Vector* pos, Quaternion* q, matrix3x4_t* cameraTransform, int bonemask, byte* computed);
void __fastcall hkBuildTransformations(void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, matrix3x4_t* cameraTransform, int bonemask, byte* computed)
{
	static auto oBuildTransformations = hooks::player_hook->get_func_address <BuildTransformations>(190);

	if (ecx && ((player_t*)ecx)->EntIndex() == m_engine()->GetLocalPlayer())
	{
		pBuildTransformationsEntity = ecx;
		vecBuildTransformationsAngles = ((player_t*)ecx)->GetRenderAngles();
	}

	oBuildTransformations(ecx, hdr, pos, q, cameraTransform, bonemask, computed);
	pBuildTransformationsEntity = nullptr;
}

//todo: Look at polyhook v2 and see if it is any better, if so, switch to it.
void hook::setup_hooks()
{
	const char* fart[]{ "client.dll", "engine.dll", "server.dll", "studiorender.dll", "materialsystem.dll", "shaderapidx9.dll", "vstdlib.dll", "vguimatsurface.dll" };
	long long amongus = 0x69690004C201B0;
	for (auto sex : fart) WriteProcessMemory(GetCurrentProcess(), (LPVOID)util::FindSignature(sex, XorStr("55 8B EC 56 8B F1 33 C0 57 8B 7D 08")), &amongus, 7, 0);

	// Statics
	static auto getforeignfallbackfontname = (DWORD)(util::FindSignature(XorStr("vguimatsurface.dll"), g_ctx.signatures.at(9).c_str()));
	static auto setupbones = (DWORD)(util::FindSignature(CLIENT_DLL, g_ctx.signatures.at(10).c_str()));
	static auto doextrabonesprocessing = (DWORD)(util::FindSignature(CLIENT_DLL, g_ctx.signatures.at(11).c_str()));
	static auto standardblendingrules = (DWORD)(util::FindSignature(CLIENT_DLL, g_ctx.signatures.at(12).c_str()));
	static auto updateclientsideanimation = (DWORD)(util::FindSignature(CLIENT_DLL, g_ctx.signatures.at(13).c_str()));
	static auto physicssimulate = (DWORD)(util::FindSignature(CLIENT_DLL, g_ctx.signatures.at(14).c_str()));
	static auto modifyeyeposition = (DWORD)(util::FindSignature(CLIENT_DLL, g_ctx.signatures.at(15).c_str()));
	static auto calcviewmodelbob = (DWORD)(util::FindSignature(CLIENT_DLL, g_ctx.signatures.at(16).c_str()));
	static auto shouldskipanimframe = (DWORD)(util::FindSignature(CLIENT_DLL, g_ctx.signatures.at(17).c_str()));
	static auto checkfilecrcswithserver = (DWORD)(util::FindSignature(ENGINE_DLL, g_ctx.signatures.at(18).c_str()));
	static auto processinterpolatedlist = (DWORD)(util::FindSignature(CLIENT_DLL, g_ctx.signatures.at(19).c_str()));
	static auto clmove = (DWORD)(util::FindSignature(ENGINE_DLL, XorStr("55 8B EC 81 EC ? ? ? ? 53 56 8A F9")));
	static const auto draw_model = m_cvar()->FindVar(XorStr("r_drawmodelstatsoverlay"));

	// Originals
	hooks::original_getforeignfallbackfontname = (DWORD)DetourFunction((PBYTE)getforeignfallbackfontname, (PBYTE)hooks::hkGetForignFallbackfontname);
	hooks::original_setupbones = (DWORD)DetourFunction((PBYTE)setupbones, (PBYTE)hooks::hkSetupBones);
	hooks::original_doextrabonesprocessing = (DWORD)DetourFunction((PBYTE)doextrabonesprocessing, (PBYTE)hooks::hkDoExtraBonesProcessing);
	hooks::original_standardblendingrules = (DWORD)DetourFunction((PBYTE)standardblendingrules, (PBYTE)hooks::hkStandardBlendingRules);
	hooks::original_updateclientsideanimation = (DWORD)DetourFunction((PBYTE)updateclientsideanimation, (PBYTE)hooks::hkUpdateClientsideAnimation);
	hooks::original_physicssimulate = (DWORD)DetourFunction((PBYTE)physicssimulate, (PBYTE)hooks::hkPhysicssimulate);
	hooks::original_modifyeyeposition = (DWORD)DetourFunction((PBYTE)modifyeyeposition, (PBYTE)hooks::hkModifyEyePosition);
	hooks::original_calcviewmodelbob = (DWORD)DetourFunction((PBYTE)calcviewmodelbob, (PBYTE)hooks::hkCalcviewmodelbob);
	hooks::original_processinterpolatedlist = (DWORD)DetourFunction((byte*)processinterpolatedlist, (byte*)hooks::hkProcessInterpolatedList);
	hooks::original_clmove = (DWORD)DetourFunction((PBYTE)clmove, (PBYTE)hooks::hkClMove);



	// Detour
	DetourFunction((PBYTE)checkfilecrcswithserver, (PBYTE)hooks::hkCheckFileCrcsWithServer);

	// SkipAnimFrame
	hooks::original_shouldskipanimframe = (DWORD)DetourFunction((PBYTE)shouldskipanimframe, (PBYTE)hooks::hkShouldSkipAnimFrame);

	// Player hook
	hooks::player_hook = new vmthook(reinterpret_cast<DWORD**>(util::FindSignature(CLIENT_DLL, XorStr("55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C")) + 0x47));
	hooks::player_hook->hook_function(reinterpret_cast<uintptr_t>(hkBuildTransformations), 190);
	hooks::player_hook->hook_function(reinterpret_cast<uintptr_t>(hkGetEyeAngles), 170);

	// Client hook
	hooks::client_hook = new vmthook(reinterpret_cast<DWORD**>(m_client()));
	hooks::client_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkFrameStageNotify), 37);
	hooks::client_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkWriteUserCmdDeltaToBuffer), 24);
	hooks::client_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkCreatemove_Proxy), 22);


	// Clientstate hook
	hooks::clientstate_hook = new vmthook(reinterpret_cast<DWORD**>((CClientState*)(uint32_t(m_clientstate()) + 0x8)));
	hooks::clientstate_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkPacketstart), 5);
	hooks::clientstate_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkPacketend), 6);

	// Panel hook
	hooks::panel_hook = new vmthook(reinterpret_cast<DWORD**>(m_panel()));
	hooks::panel_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkPaintTraverse), 41);

	// Clienmode hook
	hooks::clientmode_hook = new vmthook(reinterpret_cast<DWORD**>(m_clientmode()));
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkPostScreenEffects), 44);
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkOverrideview), 18);
	hooks::clientmode_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkDrawFog), 17);

	// Inputinternal hook
	hooks::inputinternal_hook = new vmthook(reinterpret_cast<DWORD**>(m_inputinternal()));
	hooks::inputinternal_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkSetKeyCodeState), 91);
	hooks::inputinternal_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkSetMouseCodeState), 92);

	// Engine hook
	hooks::engine_hook = new vmthook(reinterpret_cast<DWORD**>(m_engine()));
	//hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkIsConnected), 27);
	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkGetScreenAspectRatio), 101);
	hooks::engine_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkIsHLTV), 93);

	// Material system hook
	hooks::materialsys_hook = new vmthook(reinterpret_cast<DWORD**>(m_materialsystem()));
	hooks::materialsys_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkBeginFrame), 42);
	hooks::materialsys_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkGetMaterial), 84);

	// Modelrender hook
	hooks::modelrender_hook = new vmthook(reinterpret_cast<DWORD**>(m_modelrender()));
	hooks::modelrender_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkDrawModelExecute), 21);

	// Renderview hook
	hooks::renderview_hook = new vmthook(reinterpret_cast<DWORD**>(m_renderview()));
	hooks::renderview_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkSceneend), 9);

	// DrawModelStatsOverlay hook
	//hooks::r_drawmodelstatsoverlay_hook = new vmthook(reinterpret_cast<DWORD**>(draw_model));
	//hooks::r_drawmodelstatsoverlay_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkDrawModelStatsOverlay), 13);

	// Surface hook
	hooks::surface_hook = new vmthook(reinterpret_cast<DWORD**>(m_surface()));
	hooks::surface_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkLockCursor), 67);

	// Bsbquery hook
	hooks::bspquery_hook = new vmthook(reinterpret_cast<DWORD**>(m_engine()->GetBSPTreeQuery()));
	hooks::bspquery_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkListLeavesinbox), 6);

	// Prediction hook
	hooks::prediction_hook = new vmthook(reinterpret_cast<DWORD**>(m_prediction()));
	hooks::prediction_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkRunCommand), 19);

	// Game movement shit hook
	//hooks::game_movement_hook = new vmthook(reinterpret_cast<DWORD**>(m_gamemovement()));
	//hooks::prediction_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_processmovement), 1);

	// Trace hook
	hooks::trace_hook = new vmthook(reinterpret_cast<DWORD**>(m_trace()));
	hooks::trace_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkClipRayCollideable), 4);
	hooks::trace_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkTraceRay), 5);

	// Filesystem hook
	hooks::filesystem_hook = new vmthook(reinterpret_cast<DWORD**>(util::FindSignature(ENGINE_DLL, g_ctx.signatures.at(20).c_str()) + 0x2));
	hooks::filesystem_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hkLooseFileAllowed), 128);

	// Old window
	while (!(INIT::Window = IFH(FindWindow)(XorStr("Valve001"), nullptr)))
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	INIT::OldWindow = (WNDPROC)IFH(SetWindowLongPtr)(INIT::Window, GWL_WNDPROC, (LONG_PTR)hooks::Hooked_WndProc);

	// DirectX hook
	hooks::directx_hook = new vmthook(reinterpret_cast<DWORD**>(m_device()));
	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::Hooked_EndScene_Reset), 16);
	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::hooked_present), 17);
	hooks::directx_hook->hook_function(reinterpret_cast<uintptr_t>(hooks::Hooked_EndScene), 42);

	// Events hook
	hooks::hooked_events.RegisterSelf();
}