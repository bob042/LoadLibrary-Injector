/*
	LoadLibrary injection made by _bob04
	Educational purpose use only!
	You can use this without crediting and edit however you want.
*/

#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <TlHelp32.h>

// Paste your DLL here.
const char* DLLPath = "paste-your-path-here.dll";

DWORD GetPid(const wchar_t* ProcessName) {
	/*
		PID: Every program has an exclusive ID to be identified.
		We need to find the PID of our Target executable so we can
		use the OpenProcess function.
	*/
	DWORD pid{ 0 };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32W pe{};
	pe.dwSize = sizeof(PROCESSENTRY32W);

	if (Process32FirstW(hSnapshot, &pe)) {
		do {
			if (_wcsicmp(ProcessName, pe.szExeFile) == 0) {
				pid = pe.th32ProcessID;
				return pid;
			}
		} while (Process32FirstW(hSnapshot, &pe));
	}
}

int main() {
  // Paste your target process here.
	DWORD pid = GetPid(L"your-target.exe");
	HANDLE hProcess = OpenProcess(
		PROCESS_VM_OPERATION |
		PROCESS_VM_READ |
		PROCESS_VM_WRITE |
		PROCESS_CREATE_PROCESS |
		PROCESS_CREATE_THREAD,
		0,
		pid
	);
	if (hProcess == NULL) {
		std::cout << GetLastError() << '\n';
		return -1;
	}

	LPVOID VirtualMemory = VirtualAllocEx(hProcess, NULL, strlen(DLLPath), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!VirtualMemory) {
		std::cout << GetLastError() << '\n';
		CloseHandle(hProcess);
		return -1;
	}

	if (!WriteProcessMemory(hProcess, VirtualMemory, (LPCVOID)DLLPath, strlen(DLLPath), nullptr)) {
		std::cout << GetLastError() << '\n';
		CloseHandle(hProcess);
		return -1;
	}

	LPVOID lpLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

	HANDLE hThread = CreateRemoteThread(
		hProcess,
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)lpLoadLibrary,
		VirtualMemory,
		0,
		NULL
	);

	WaitForSingleObject(hThread, INFINITE);
	VirtualFreeEx(hProcess, VirtualMemory, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);

	return 0;
}
