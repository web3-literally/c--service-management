#include "pch.h"
#include "fileutil.h"

#include <chrono>
#include <inttypes.h>
#include <tchar.h>
#include <Windows.h>

using namespace std;

std::wstring GenerateServiceTargetFile(std::wstring file_name)
{
	std::wstring target_path;

#define INFO_BUFFER_SIZE 32767
	TCHAR  bufDirPath[INFO_BUFFER_SIZE];
	TCHAR  bufFilePath[INFO_BUFFER_SIZE];

	// Try to get system directory
	//if (!GetSystemDirectory(bufDirPath, INFO_BUFFER_SIZE)) {
	//	// If getting system directory failed, try to get temp directory
	//	DWORD dwRetVal = GetTempPath(INFO_BUFFER_SIZE,          // length of the buffer
	//		bufDirPath); // buffer for path 

	//	if (dwRetVal > INFO_BUFFER_SIZE || (dwRetVal == 0)) {
	//		// Failed to get target path, return empty string
	//		return target_path;
	//	}
	//}

	//  Generates a temporary file name. 
	//UINT uRetVal = GetTempFileName(bufDirPath,					// directory for tmp files
	//	TEXT("svc"),			// temp file name prefix 
	//	0,							// create unique name 
	//	bufFilePath);				// buffer for name 

	//if (uRetVal == 0)
	//{	// Generate temporary file name failed, use random file name
	//	using namespace std::chrono;
	//	unsigned __int64 now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	//	_stprintf_s(bufFilePath, INFO_BUFFER_SIZE, _T("%s\\svc%" PRIu64 ".tmp"), bufDirPath, now);
	//}

	// _stprintf_s(bufFilePath, INFO_BUFFER_SIZE, _T("%s.exe"), bufFilePath);

	// If getting system directory failed, try to get temp directory
	DWORD dwRetVal = GetTempPath(INFO_BUFFER_SIZE,          // length of the buffer
									bufDirPath); // buffer for path 

	if (dwRetVal > INFO_BUFFER_SIZE || (dwRetVal == 0)) {
		// Failed to get target path, return empty string
		return target_path;
	}

	// Generate service file name
	_stprintf_s(bufFilePath, INFO_BUFFER_SIZE, _T("%s\\%s"), bufDirPath, file_name.c_str());
	
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
