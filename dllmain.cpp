#include <windows.h>
#include "debug.h"
#include "Interfaces.h"
#include "globals.h"
#include "hooks.h"
#include "c_gmod_player.h"
#include "sigscan.h"
#include "c_playerresource.h"
#include "NetvarManager.h"
#include "tramphook.h"
#include "c_enginevgui.h"
#include "endscene.h"

CInterfaces* Interface = nullptr;
CDebugConsole* g_pDebug = nullptr;
CGlobalVars* g_pGlobals = nullptr;

HMODULE hDll;

bool g_bShutDown = false;
BOOL WINAPI MainThread(HMODULE hThread)
{
	g_pDebug = new CDebugConsole();
	g_pDebug->CreateInstance();
	g_pDebug->Print("Initializing Cheat...\n");

	g_pDebug->Print("ENDSCENE SIG -> %p\n", FindSignature("shaderapidx9.dll", "\x89\x35\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x80\xA7\x00\x00\x00\x00\x00", "xx????x????xx?????"));


	// initializing stuff
	Interface = new CInterfaces();
	Hooks = new CHooks();

	uintptr_t callLocalPlayer = (uintptr_t)((uintptr_t)(*(void***)Interface->ClientModeShared)[21] + 
#ifndef _WIN64
		4
#else
		18
#endif
		);
	int call = *(int*)callLocalPlayer;
	GetLocalPlayer = (C_GMOD_Player*(*)())(callLocalPlayer + call + 4);

#ifndef _WIN64
	g_pGlobals = (CGlobalVars*)(**(DWORD***)(void*)(
		(uintptr_t)(*(void***)Interface->Client)[11] + 5
	));
#else
	{
		uintptr_t mov = (uintptr_t)((*(void***)Interface->Client)[11]) + 9;
		g_pGlobals = *(CGlobalVars**)(
			mov + *(int*)mov + 4
		);
	}
#endif
	Interface->Dump("client.dll");
	Interface->Dump("engine.dll");
	Interface->Dump("materialsystem.dll");

#ifndef _WIN64
	{
		DWORD startupLua = FindSignature("client.dll", "\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x68\x00\x00\x00\x00\x8B\xD9", "xxxxx????xx????xx");
		g_pDebug->Print("Startup lua: %p\n", startupLua);
	}

	CEngineVGui* vgui = EngineVGui();
	g_pDebug->Print("vgui -> %p\n", vgui);

#endif

	g_pDebug->Print("EndScene -> %p\n", Hooks->pD3DDevice->vTable[42]);

	Hooks->EndScene->Enable();
	// Hooks->CreateMove->Hook();

	while (!g_bShutDown)
	{
		Sleep(25);
	}

	g_pDebug->Print("Unloading...\n");
	Sleep(20);
	Hooks->Release();
	Sleep(100);
	delete Interface;
	Sleep(50);
	g_pDebug->Release();
	Sleep(100);
	FreeLibraryAndExitThread(hDll, 0);
	Sleep(100);
	FreeLibraryAndExitThread(hThread, 0);
	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, 0, 0, 0));
		hDll = hModule;
	}
	return TRUE;
}