#include <string>
#include <vector>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>

template<typename _Char, typename _Ty>
void GetCmdOutput(const char* cmd, _Ty& result, int rows = -1)
{
	STARTUPINFOA si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	char m_buffer[1024] = { 0 };
	DWORD ReadNum = 0;
	HANDLE hRead = NULL;
	HANDLE hWrite = NULL;
	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = 0;
	BOOL bRet = CreatePipe(&hRead, &hWrite, &sa, 0);
	if (bRet == TRUE) {
	} else {
		return;
	}
	HANDLE hTemp = GetStdHandle(STD_OUTPUT_HANDLE);
	SetStdHandle(STD_OUTPUT_HANDLE, hWrite);

	GetStartupInfoA(&si);
	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	//si.hStdInput = hRead;
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;

	bRet = CreateProcessA(NULL, (LPSTR)cmd, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi);
	SetStdHandle(STD_OUTPUT_HANDLE, hTemp);
	if (bRet == TRUE) {
	} else {
		// std::cout << "create pop failed: " << GetLastError() << std::endl;
	}
	CloseHandle(hWrite);
	std::ostringstream ssi;
	std::istringstream sso;
	sso.tie(&ssi);
	while (ReadFile(hRead, m_buffer, 1024, &ReadNum, NULL))
	{
		m_buffer[ReadNum] = '\0';
		ssi.write(m_buffer, ReadNum);
	}
	std::string temp;
	while (std::getline(sso, temp)) {
		result.emplace_back(temp.begin(), temp.end());
	}
	return;
}
#elif defined __linux__

template<typename _Char, typename _Ty>
void GetCmdOutput(const char* cmd, _Ty& result, int rows = -1)
{
	_Char m_buffer[1024];
	FILE* ptr;
	result.emplace_back();
	if ((ptr = popen(cmd, "r")))
	{
		while (fgets((char*)m_buffer, 1024, ptr)) {
			if (!result.back().empty() && result.back().back() == '\n') {
				if (!--rows) return;
				result.emplace_back(m_buffer);
			}
			else {
				result.back().append(m_buffer);
			}
		}
		pclose(ptr);
	}
	else {
		// std::cout << "cmd error" << std::endl;
	}
}
#endif

inline std::string GetStdString(const std::u8string& str) {
    return std::string(str.begin(), str.end());
}

template <typename _Ty>
inline std::u8string GetU8String(const _Ty& str) {
    return std::u8string(str.begin(), str.end());
}
