#include <Windows.h>


template<typename T>
void Patch(int address, T data)
{
	unsigned long vp[2];
	unsigned int size = sizeof(T);
	VirtualProtect((void *)address, size, PAGE_EXECUTE_READWRITE, &vp[0]);
	memcpy((void *)address, &data, size);
	VirtualProtect((void *)address, size, vp[0], &vp[1]);
}

void GetFileVersion(LPTSTR lptstrFilename, LPDWORD lpdwVersion) 
{
	DWORD dwHandle;
	DWORD dwFileVersionSize;
	VS_FIXEDFILEINFO *pFileInfo;
	UINT dwFileInfoBytes;
	BYTE *pVerBuffer;
	DWORD dwVersion;
	
	*lpdwVersion = 0xFFFFFFFF;
	
	dwFileVersionSize = GetFileVersionInfoSize(lptstrFilename, &dwHandle);
		
	if ( dwFileVersionSize != 0 )
	{
		pVerBuffer = (BYTE *)LocalAlloc(LPTR, dwFileVersionSize);
		if ( pVerBuffer )
		{
			if (   GetFileVersionInfo(lptstrFilename, 0, dwFileVersionSize, pVerBuffer)
				&& VerQueryValue(pVerBuffer, "\\", (LPVOID *)&pFileInfo, &dwFileInfoBytes)
				&& dwFileInfoBytes == sizeof(VS_FIXEDFILEINFO) )
			{
				*lpdwVersion = pFileInfo->dwFileVersionMS;
			}
			
			LocalFree(pVerBuffer);
		}
	}
}

typedef HRESULT(WINAPI *DirectInput8Create_t)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPVOID punkOuter);
DirectInput8Create_t _DirectInput8Create;
HMODULE hdinput8;

extern "C"
{
	__declspec(dllexport) HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPVOID punkOuter)
	{
		return _DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
	}
}

#ifdef __MINGW32__
extern "C" __declspec(dllexport) // MinGW
#endif
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	if( reason == DLL_PROCESS_ATTACH )
	{
		char path[MAX_PATH];
		char moduleName[MAX_PATH];
		DWORD version;
		
		
		CopyMemory(path + GetSystemDirectory(path, MAX_PATH-12), "\\dinput8.dll", 13);
		hdinput8 = LoadLibrary(path);
		if ( hdinput8 == NULL )
		{
			MessageBox(0, "Can't load dinput8.dll", "ReefFix", MB_ICONERROR);
			ExitProcess(0);
		}
		
		_DirectInput8Create = (DirectInput8Create_t)GetProcAddress(hdinput8, "DirectInput8Create");
		
		GetModuleFileName(NULL, moduleName, MAX_PATH);
		GetFileVersion(moduleName, &version);
		
		if ( version == 0x00010000 )
		{
			// "Unable to initialise graphics"
			// D3DFMT_D24X4S4 -> D3DFMT_D24S8
			Patch<unsigned char>(0x474DC4+1, 0x4B);
			
			// "This demo can only run on Matrox Parhelia Graphics cards"
			// jz     short loc_403A7D -> jmp     short loc_403A7D
			Patch<unsigned char>(0x403A08, 0xEB);
		}
		else if ( version == 0x00010001 )
		{	
			// "Unable to initialise graphics"
			// D3DFMT_D24X4S4 -> D3DFMT_D24S8
			Patch<unsigned char>(0x479934+1, 0x4B);
			
			// "This demo can only run on Matrox Parhelia Graphics cards"
			// jz     short loc_403FCC -> jmp     short loc_403FCC
			Patch<unsigned char>(0x403F57, 0xEB);
		}
		else
		{
			MessageBox(0, "Unsupported version", "ReefFix", MB_ICONERROR);
			ExitProcess(0);
		}
	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		FreeLibrary(hdinput8);
	}
	
	return TRUE;
}