#pragma once

#include <Windows.h>
#include <TlHelp32.h>

class DLLInjector
{
public:
	bool inject(const WCHAR* dllName, DWORD processId)
	{
		HANDLE hProcess = NULL;
		HANDLE hThread = NULL;
		int length = 0;
		WCHAR* remoteDLLName = nullptr;
		bool result = false;
		__try
		{
			hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_WRITE | PROCESS_VM_OPERATION,
				FALSE, processId);
			if (hProcess == NULL)
			{
				result = false;
				__leave;
			}

			length = (lstrlenW(dllName) + 1) * sizeof(WCHAR);
			WCHAR* remoteDLLName = (WCHAR*)VirtualAllocEx(hProcess, NULL, length,
				MEM_COMMIT, PAGE_READWRITE);
			if (remoteDLLName == NULL)
			{
				result = false;
				__leave;
			}

			if (!WriteProcessMemory(hProcess, remoteDLLName, dllName, length, NULL))
			{
				result = false;
				__leave;
			}

			LPTHREAD_START_ROUTINE loadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "LoadLibraryW");
			if (loadLibrary == NULL)
			{
				result = false;
				__leave;
			}

			hThread = CreateRemoteThread(hProcess, NULL, 0, loadLibrary, remoteDLLName, 0, NULL);
			if (hThread == NULL)
			{
				result = false;
				__leave;
			}

			WaitForSingleObject(hThread, INFINITE);
			result = true;
		}
		__finally
		{
			if (hThread)
			{
				CloseHandle(hThread);
			}
			if (remoteDLLName)
			{
				VirtualFreeEx(hProcess, remoteDLLName, 0, MEM_RELEASE);
			}
			if (hProcess)
			{
				CloseHandle(hProcess);
			}

			if (result == false)
			{
				this->errorCode = GetLastError();
				return false;
			}
			return true;
		}
	}

	bool inject(const WCHAR* dllName, const WCHAR* processName)
	{
		return inject(dllName, GetProcessIdByName(processName));
	}

	DWORD GetErrorCode() const { return this->errorCode; }

private:
	DWORD GetProcessIdByName(const WCHAR* processName)
	{
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if (Process32First(snapshot, &entry) == TRUE)
		{
			while (Process32Next(snapshot, &entry) == TRUE)
			{
				if (lstrcmp(entry.szExeFile, processName) == 0)
				{
					CloseHandle(snapshot);
					return entry.th32ProcessID;
				}
			}
		}
		CloseHandle(snapshot);
		return NULL;
	}

private:
	DWORD errorCode;
};