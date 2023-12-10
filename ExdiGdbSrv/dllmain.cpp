// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "ExdiGdbSrv.h"
#include "dllmain.h"

CExdiGdbSrvModule _AtlModule;
VOID
ShowDebugConsole()
{
	if (AllocConsole())
	{
		char szbuffer[MAX_PATH]{};
		GetModuleFileNameA(nullptr, szbuffer, MAX_PATH);
		SetConsoleTitleA(szbuffer);
		AttachConsole(GetCurrentProcessId());
		FILE* pFile = nullptr;
		freopen_s(&pFile, "CON", "r", stdin);
		freopen_s(&pFile, "CON", "w", stdout);
		freopen_s(&pFile, "CON", "w", stderr);
		//setlocale(0, "chs");
	}
}
// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	ShowDebugConsole();
	hInstance;
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
