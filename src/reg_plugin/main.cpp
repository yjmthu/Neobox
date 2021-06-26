#include <Windows.h>
#include <Shlobj.h>
#include <iostream>
#include <string>

void msgbox(std::wstring str)
{
    MessageBox(NULL,str.c_str(),L"title",MB_ICONWARNING);
}

void msgbox(int k)
{
    msgbox(std::to_wstring(k));
}

const std::wstring reg_path[3] = {
    L"*\\shell",
    L"Directory\\shell",
    L"Directory\\Background\\shell"
};
void changeReg(bool, short);

int main(int argc, char *argv[])
{
    if (argc==2)
    {
        for (short i=0; i<3; i++)
        {
            changeReg(argv[1][i]=='1', i);
        }
    }
    return 0;
}

const WCHAR argv_rec[3] = {'1', '1', 'V'};

void changeReg(bool checked, short t)
{
    std::wstring command = L"mshta vbscript:clipboarddata.setdata(\"text\",\"%";
    command += argv_rec[t];
    command += L"\")(close)";
    WCHAR szPath[MAX_PATH];
    SHGetSpecialFolderPath(NULL, szPath, CSIDL_LOCAL_APPDATA, FALSE);
    std::wstring img_path = szPath;
    img_path += L"\\SpeedBox\\Copy.ico";

    HKEY pKey;
    RegOpenKeyEx(HKEY_CLASSES_ROOT, (reg_path[t]+L"\\QCoper").c_str(), 0, KEY_ALL_ACCESS, &pKey);
    if (pKey)
    {
        RegCloseKey(pKey);
        if (!checked)
        {
            HKEY key;
            RegOpenKeyEx(HKEY_CLASSES_ROOT, reg_path[t].c_str(), 0, KEY_ALL_ACCESS, &key);
            if (key)
            {
                RegDeleteKey(key, L"QCoper\\command");
                RegDeleteKey(key, L"QCoper");
                RegCloseKey(key);
            }
        }
    }
    else
    {
        if (checked)
        {
            HKEY key;
            DWORD dispos;
            SECURITY_ATTRIBUTES sa;
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.bInheritHandle = TRUE;
            sa.lpSecurityDescriptor = NULL;
            long iret = RegCreateKeyEx(HKEY_CLASSES_ROOT, (reg_path[t]+L"\\QCoper").c_str(), NULL, NULL,
                                 REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                 &sa, &key, &dispos);
            if(iret == 0)
            {
                const WCHAR *value = L"复制路径";
                RegSetValueEx(key, L"", NULL, REG_SZ, (BYTE*)value, sizeof (value)); // == ERROR_SUCCESS
                RegSetValueEx(key, L"Icon", NULL, REG_SZ, (BYTE*)(img_path.c_str()), img_path.length()*4);
                RegCloseKey(key);
                HKEY _key;
                DWORD _dispos;
                SECURITY_ATTRIBUTES saa;
                saa.nLength = sizeof(SECURITY_ATTRIBUTES);
                saa.bInheritHandle = TRUE;
                saa.lpSecurityDescriptor = NULL;
                long _iret = RegCreateKeyEx(HKEY_CLASSES_ROOT, (reg_path[t]+L"\\QCoper\\command").c_str(), NULL, NULL,
                                     REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                     &saa, &_key, &_dispos);
                if (_iret == 0)
                {
                    RegSetValueEx(_key, L"", NULL, REG_SZ, (BYTE*)(command.c_str()), command.length()*4);
                }
                RegCloseKey(_key);
            }
        }
    }
}
