#include "pch.h"
#include <Windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>

#include "Elevation.h"
#include "Determination.h"

void ElevateNow()
{
	BOOL bAlreadyRunningAsAdministrator = FALSE;
	try
	{
		bAlreadyRunningAsAdministrator = IsRunAsAdministrator();
	}
	catch (...)
	{
		_asm nop
	}
	if (!bAlreadyRunningAsAdministrator)
	{
		TCHAR szPath[MAX_PATH];
		if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
		{
			SHELLEXECUTEINFO sei = { sizeof(sei) };

			sei.lpVerb = TEXT("runas");
			sei.lpFile = szPath;
			sei.hwnd = NULL;
			sei.nShow = SW_NORMAL;

			if (!ShellExecuteEx(&sei))
			{
				DWORD dwError = GetLastError();
				if (dwError == ERROR_CANCELLED)
					//Annoys you to Elevate it LOL
					CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ElevateNow, 0, 0, 0);
			}
		}

	}
	else
	{
		///Code
	}
}
