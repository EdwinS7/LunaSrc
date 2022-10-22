#include "Features\HttpRequest.h"
#include "Features\hook.h"
#include <wincrypt.h>

#define CheatName = "Luna";
#define Version = 3;

void AttachConsole() {
    if (!L::Attach(XorStr("LUNA V3 PRE-RELEASE")))
        throw std::runtime_error(XorStr("failed to attach console"));
}

void Instance(PVOID base) {
	AttachConsole();
	L::PushConsoleColor(FOREGROUND_BLUE);

#if BETA
    g_ctx.username = "Developer";
    g_ctx.uid = 1;
#else
    //Setup loader shit here so we know the users information.
	g_ctx.username = "Public user";
    g_ctx.uid = 1;
#endif
    hook::get().Setup();
  
    return ExitThread( EXIT_SUCCESS );
}

BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwCallReason, LPVOID pReserved ) {
	DisableThreadLibraryCalls(hInstance);
	if (dwCallReason != DLL_PROCESS_ATTACH)
		return FALSE;

	HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)(Instance), hInstance, NULL, NULL);
	if (hThread)
		CloseHandle(hThread);

    return TRUE;
}