#pragma once

#include <Windows.h>

int SvcInstall(const wchar_t *szSvcPath, const wchar_t *szSvcName);

int __stdcall DoStartSvc(const wchar_t *szSvcName);

VOID __stdcall DoStopSvc(const wchar_t *szSvcName);

VOID __stdcall DoDisableSvc(const wchar_t *szSvcName);

VOID __stdcall DoEnableSvc(const wchar_t *szSvcName);

VOID __stdcall DoUpdateSvcDesc(const wchar_t *szSvcName);

VOID __stdcall DoDeleteSvc(const wchar_t *szSvcName);
