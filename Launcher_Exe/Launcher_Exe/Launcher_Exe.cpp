// Launcher_Exe.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <string>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <tchar.h>
#include <Windows.h>

TCHAR szSvcName[256] = _T("Sample Service");
//TCHAR szSvcPath[256] = _T("C:\\SampleService.exe");

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID SvcInstall(const wchar_t *szSvcPath)
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];

	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		printf("Cannot install service (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Create the service

	schService = CreateService(
		schSCManager,						// SCM database 
		szSvcName,							// name of service 
		szSvcName,							// service name to display 
		SERVICE_ALL_ACCESS,					// desired access 
		SERVICE_WIN32_OWN_PROCESS,			// service type 
		SERVICE_DEMAND_START,				// start type 
		SERVICE_ERROR_NORMAL,				// error control type 
		szSvcPath,							// path to service's binary 
		NULL,								// no load ordering group 
		NULL,								// no tag identifier 
		NULL,								// no dependencies 
		NULL,								// LocalSystem account 
		NULL);								// no password 

	if (schService == NULL)
	{
		printf("CreateService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	else printf("Service installed successfully\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

//
// Purpose: 
//   Starts the service if possible.
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID __stdcall DoStartSvc()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwOldCheckPoint;
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,         // SCM database 
		szSvcName,            // name of service 
		SERVICE_ALL_ACCESS);  // full access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Check the status in case the service is not stopped. 

	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // information level
		(LPBYTE)&ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // size needed if buffer is too small
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// Check if the service is already running. It would be possible 
	// to stop the service here, but for simplicity this example just returns. 

	if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		printf("Cannot start the service because it is already running\n");
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	// Wait for the service to stop before attempting to start it.

	while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status until the service is no longer stop pending. 

		if (!QueryServiceStatusEx(
			schService,                     // handle to service 
			SC_STATUS_PROCESS_INFO,         // information level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // size needed if buffer is too small
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				printf("Timeout waiting for service to stop\n");
				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return;
			}
		}
	}

	// Attempt to start the service.

	if (!StartService(
		schService,  // handle to service 
		0,           // number of arguments 
		NULL))      // no arguments 
	{
		printf("StartService failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}
	else printf("Service start pending...\n");

	// Check the status until the service is no longer start pending. 

	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // info level
		(LPBYTE)&ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // if buffer too small
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status again. 

		if (!QueryServiceStatusEx(
			schService,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // if buffer too small
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			break;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				// No progress made within the wait hint.
				break;
			}
		}
	}

	// Determine whether the service is running.

	if (ssStatus.dwCurrentState == SERVICE_RUNNING)
	{
		printf("Service started successfully.\n");
	}
	else
	{
		printf("Service not started. \n");
		printf("  Current State: %d\n", ssStatus.dwCurrentState);
		printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
		printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
		printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

//
// Purpose: 
//   Disables the service.
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID __stdcall DoDisableSvc()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,            // SCM database 
		szSvcName,               // name of service 
		SERVICE_CHANGE_CONFIG);  // need change config access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Change the service start type.

	if (!ChangeServiceConfig(
		schService,        // handle of service 
		SERVICE_NO_CHANGE, // service type: no change 
		SERVICE_DISABLED,  // service start type 
		SERVICE_NO_CHANGE, // error control: no change 
		NULL,              // binary path: no change 
		NULL,              // load order group: no change 
		NULL,              // tag ID: no change 
		NULL,              // dependencies: no change 
		NULL,              // account name: no change 
		NULL,              // password: no change 
		NULL))            // display name: no change
	{
		printf("ChangeServiceConfig failed (%d)\n", GetLastError());
	}
	else printf("Service disabled successfully.\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

//
// Purpose: 
//   Enables the service.
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID __stdcall DoEnableSvc()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,            // SCM database 
		szSvcName,               // name of service 
		SERVICE_CHANGE_CONFIG);  // need change config access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Change the service start type.

	if (!ChangeServiceConfig(
		schService,            // handle of service 
		SERVICE_NO_CHANGE,     // service type: no change 
		SERVICE_DEMAND_START,  // service start type 
		SERVICE_NO_CHANGE,     // error control: no change 
		NULL,                  // binary path: no change 
		NULL,                  // load order group: no change 
		NULL,                  // tag ID: no change 
		NULL,                  // dependencies: no change 
		NULL,                  // account name: no change 
		NULL,                  // password: no change 
		NULL))                // display name: no change
	{
		printf("ChangeServiceConfig failed (%d)\n", GetLastError());
	}
	else printf("Service enabled successfully.\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

//
// Purpose: 
//   Updates the service description to "This is a test description".
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID __stdcall DoUpdateSvcDesc()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_DESCRIPTION sd;
	TCHAR szDesc[256] = _T("This is a test description");

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,            // SCM database 
		szSvcName,               // name of service 
		SERVICE_CHANGE_CONFIG);  // need change config access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Change the service description.

	sd.lpDescription = szDesc;

	if (!ChangeServiceConfig2(
		schService,                 // handle to service
		SERVICE_CONFIG_DESCRIPTION, // change: description
		&sd))                      // new description
	{
		printf("ChangeServiceConfig2 failed\n");
	}
	else printf("Service description updated successfully.\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

//
// Purpose: 
//   Deletes a service from the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID __stdcall DoDeleteSvc()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	SERVICE_STATUS ssStatus;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,       // SCM database 
		szSvcName,          // name of service 
		DELETE);            // need delete access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Delete the service.

	if (!DeleteService(schService))
	{
		printf("DeleteService failed (%d)\n", GetLastError());
	}
	else printf("Service deleted successfully\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

std::wstring GenerateServiceTargetFile()
{
	std::wstring target_path;

#define INFO_BUFFER_SIZE 32767
	TCHAR  bufDirPath[INFO_BUFFER_SIZE];
	TCHAR  bufFilePath[INFO_BUFFER_SIZE];

	// Try to get system directory
	//if (!GetSystemDirectory(bufDirPath, INFO_BUFFER_SIZE)) {
	//	// If getting system directory failed, try to get temp directory
	//	DWORD dwRetVal = GetTempPath(INFO_BUFFER_SIZE,          // length of the buffer
	//									bufDirPath); // buffer for path 
	//	
	//	if (dwRetVal > INFO_BUFFER_SIZE || (dwRetVal == 0)) {
	//		// Failed to get target path, return empty string
	//		return target_path;
	//	}
	//}

	DWORD dwRetVal = GetTempPath(INFO_BUFFER_SIZE,          // length of the buffer
									bufDirPath); // buffer for path 
		
	if (dwRetVal > INFO_BUFFER_SIZE || (dwRetVal == 0)) {
		// Failed to get target path, return empty string
		return target_path;
	}

	//  Generates a temporary file name. 
	UINT uRetVal = GetTempFileName(bufDirPath,					// directory for tmp files
									TEXT("svc"),			// temp file name prefix 
									0,							// create unique name 
									bufFilePath);				// buffer for name 

	if (uRetVal == 0)
	{	// Generate temporary file name failed, use random file name
		using namespace std::chrono;
		unsigned __int64 now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		_stprintf_s(bufFilePath, INFO_BUFFER_SIZE, _T("%s\\svc%" PRIu64 ".tmp"), bufDirPath, now);
	}

	_stprintf_s(bufFilePath, INFO_BUFFER_SIZE, _T("%s.exe"), bufFilePath);

	target_path = std::wstring(bufFilePath);
	return target_path;
}

std::wstring GetDirectoryFromFile(std::wstring file_path)
{
	std::wstring directory;
	const size_t last_slash_idx = file_path.rfind('\\');
	if (std::wstring::npos != last_slash_idx)
	{
		directory = file_path.substr(0, last_slash_idx);
	}

	return directory;
}

#define SOURCE_NAME		L"halort.dll"
typedef int (__cdecl *fnLauncher)(void);

int main()
{
    std::cout << "Hello World!\n";

	fnLauncher ProcLaunch;
	HINSTANCE hinstLib = LoadLibrary(L"Launcher.dll");
	if (hinstLib != NULL) {
		ProcLaunch = (fnLauncher)GetProcAddress(hinstLib, "fnLauncher");
		if (NULL != ProcLaunch) {
			int retval = (ProcLaunch)();
			wprintf(L"%d", retval);
		}
		FreeLibrary(hinstLib);
	}

	return 0;

	/*ShellExecute(NULL, L"sc", L"stop 'Sample Service'", NULL, NULL, 0);
	ShellExecute(NULL, L"sc", L"delete 'Sample Service'", NULL, NULL, 0);*/

	std::wstring target_path = GenerateServiceTargetFile();
	if (!target_path.empty()) {

		TCHAR szPath[MAX_PATH];

		if (!GetModuleFileName(NULL, szPath, MAX_PATH))
		{
			printf("Cannot install service (%d)\n", GetLastError());
			return -1;
		}

		std::wstring moduleDirPath = GetDirectoryFromFile(szPath) + std::wstring(L"\\");
		std::wstring svcSourcePath = moduleDirPath + std::wstring(SOURCE_NAME);

		if (!CopyFile(svcSourcePath.c_str(), target_path.c_str(), FALSE)) {
			printf("Cannot copy file (%d)\n", GetLastError());
			return -1;
		}

		DoDisableSvc();
		DoDeleteSvc();
		SvcInstall(target_path.c_str());
		DoStartSvc();
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
