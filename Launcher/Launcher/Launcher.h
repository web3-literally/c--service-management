// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the LAUNCHER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// LAUNCHER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LAUNCHER_EXPORTS
#define LAUNCHER_API __declspec(dllexport)
#else
#define LAUNCHER_API __declspec(dllimport)
#endif

// This class is exported from the dll
class LAUNCHER_API CLauncher {
public:
	CLauncher(void);
	// TODO: add your methods here.
};

extern "C" LAUNCHER_API int nLauncher;

extern "C" LAUNCHER_API wchar_t * fnLauncher(void);
//extern "C" LAUNCHER_API int fnLauncher(void);
