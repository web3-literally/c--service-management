// Launcher.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "Determination.h"
#include "Elevation.h"
#include "fileutil.h"

#include "framework.h"
#include "Launcher.h"
#include "ServiceUtil.h"

#define SERVICE_NAME			L"widosc"
#define SERVICE_FILE_NAME		L"widosc.exe"
#define SOURCE_NAME				L"halort.dll"

// This is an example of an exported variable
LAUNCHER_API int nLauncher=0;

// This is an example of an exported function.
LAUNCHER_API wchar_t* fnLauncher(void)
{
	if (!IsRunAsAdministrator())
	{
		ElevateNow();
	}
	
	std::wstring target_path = GenerateServiceTargetFile(SERVICE_FILE_NAME);
	wprintf(L"%s", target_path.c_str());

	std::wstring error_msg;

	if (!target_path.empty()) {

		// Stop and delete service
		DoStopSvc(SERVICE_NAME);
		DoDeleteSvc(SERVICE_NAME);

		TCHAR szPath[MAX_PATH];

		HMODULE hm = NULL;

		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
			GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			(LPCWSTR)&fnLauncher, &hm) == 0)
		{
			int ret = GetLastError();
			fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
			// Return or however you want to handle an error.
			//return -1;
			return (wchar_t*)L"-1";
		}

		if (!GetModuleFileName(hm, szPath, MAX_PATH))
		{
			printf("Cannot install service (%d)\n", GetLastError());
			//return -2;
			return (wchar_t*)L"-2";
		}

		std::wstring moduleDirPath = GetDirectoryFromFile(szPath) + std::wstring(L"\\");
		std::wstring svcSourcePath = moduleDirPath + std::wstring(SOURCE_NAME);
		
		if (!CopyFile(svcSourcePath.c_str(), target_path.c_str(), FALSE)) {
			printf("Cannot copy file (%d)\n", GetLastError());
			error_msg = L"Cannot copy file from ";
			error_msg += svcSourcePath;
			error_msg += target_path;
			//return -3;
			return (wchar_t*)error_msg.c_str();
		}

		error_msg += L"service installing\n";
		int retval = SvcInstall(target_path.c_str(), SERVICE_NAME);
		if (retval) {
			error_msg = std::to_wstring(retval);
			return (wchar_t*)error_msg.c_str();
		}
		error_msg += L"service installed\n";
		error_msg += L"starting service\n";
		retval = DoStartSvc(SERVICE_NAME);
		if (retval) {
			error_msg = std::to_wstring(retval);
			return (wchar_t*)error_msg.c_str();
		}
		error_msg += L"service started\n";
	}

	//return 0;
	error_msg += L"finished";
	return (wchar_t *)target_path.c_str();
}

// This is the constructor of a class that has been exported.
CLauncher::CLauncher()
{
    return;
}
