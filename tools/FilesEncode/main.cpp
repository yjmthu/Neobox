#include <bits/stdc++.h>
#include <io.h>
using namespace std;

inline bool isMatchSuffix(const string& name, const string& suffix) {
    if (name.size() <= suffix.size()) return false;
    auto iter = find(name.rbegin(), name.rend(), '.').base();
    /* 由于前面比较了字符串的大小，这里不需要加上 iter != name.begin() &&  */
    return name.end() - iter == suffix.size() && equal(suffix.begin(), suffix.end(), iter);
}

void removeFileUtf8Bom(const string& m_sFilePath) {
    static unsigned const char utf8bom[] { 0xEF, 0xBB, 0xBF };
    static unsigned char tempbom[3];
    ifstream file_old(m_sFilePath, ios::in | ios::binary);
    if (file_old.is_open()) {
        memset(tempbom, 0, 3*sizeof(unsigned char));
        if (file_old.read(reinterpret_cast<char*>(tempbom), 3)) {
            if (equal(utf8bom, utf8bom+3, tempbom)) {
                ofstream file_new("./file.temp", ios::out | ios::binary);
                if (file_new.is_open()) {
                    file_new << file_old.rdbuf();
                    file_new.close();
                    file_old.close();
                    remove(m_sFilePath.c_str());
                    rename("./file.temp", m_sFilePath.c_str());
                    cout << "Remove file " << m_sFilePath << "'s utf8-bom successfully!\n";
                    return;
                } else {
                    cout << "Can't create temp file!\n";
                }
            } else {
                cout << "File " << m_sFilePath << " dosen't have utf8-bom\n";
            }
        } else {
            cout << "File is empty!\n";
        }
        file_old.close();
    } else {
        cout << "Can't open file: " << m_sFilePath << endl;
    }
}

void addFileUtf8Bom(const string& m_sFilePath) {
    static unsigned const char utf8bom[] { 0xEF, 0xBB, 0xBF };
    static unsigned char tempbom[3];
    fstream file(m_sFilePath, ios::in | ios::out | ios::binary);
    if (file.is_open()) {
        memset(tempbom, 0, 3*sizeof(unsigned char));
        if (file.read(reinterpret_cast<char*>(tempbom), 3)) {
            if (!equal(utf8bom, utf8bom+3, tempbom)) {
                file.seekg(0, ios::end);
                file.write(reinterpret_cast<const char*>(utf8bom), 3);
                streamoff fPtr = file.tellg() - streamoff(3);
                while ((fPtr -= 1) >= 0) {
                    file.seekg(fPtr);
                    char get = file.get();
                    file.seekg(fPtr + streamoff(3));
                    file.put(get);
                }
                file.seekg(0, ios::beg);
                file.write(reinterpret_cast<const char*>(utf8bom), 3);
                cout << "Change " << m_sFilePath << " to utf-8-bom successfully!\n";
            } else {
                cout << "File " << m_sFilePath << "is allready utf8-bom file\n";
            }
        } else {
            cout << "File id empty!\n";
            file.seekg(0, ios::beg);
            file.write(reinterpret_cast<const char*>(utf8bom), 3);
        }
        file.close();
    } else {
        cout << "Can't open file: " << m_sFilePath << endl;
    }
}

void HandleFile(const std::string& folder, _finddata_t* fileData, const string& suffix, void(*pf)(const string&))
{
    std::string tempPath = folder + "\\*";
    auto x = _findfirst(tempPath.c_str(), fileData);
    while (_findnext(x, fileData) == 0) {
        if (!strcmp(fileData->name, ".."))
            continue;
        string m_sFileName(folder + "\\" + fileData->name);
        if (fileData->attrib & _A_SUBDIR) {
            HandleFile(m_sFileName, fileData, suffix, pf);
        } else {
            if (isMatchSuffix(m_sFileName, suffix)) pf(m_sFileName);
        }
    }
    _findclose(x);
}

void HandleFile(const std::string& folder, const vector<string>& suffix, void(*pf)(const string&))
{
    _finddata_t m_dFileData;
    for (auto& str: suffix) {
        cout << "Change suffix " << str << "...\n";
        HandleFile(folder, &m_dFileData, str, pf);
    }
}

#if 1
int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::string m_sMainPath;
        cout << "Please input the main Directory: \n";
        std::cin >> m_sMainPath;
        cout << "Input the number of suffix.\n";
        int count = 0;
        cin >> count;
        cout << "Please input the suffix, without \".\", and separated by a space between them: \n";
        vector<string> m_sStrList;
        for (size_t i = 0; i < count; i++)
        {
            m_sStrList.emplace_back();
            cin >> m_sStrList.back();
        }
        cout << "Add or remove?(1 or 2)\n";
        int a;
        cin >> a;
        if (a == 1) {
            HandleFile(m_sMainPath, m_sStrList, &addFileUtf8Bom);
        } else if (a == 2) {
            HandleFile(m_sMainPath, m_sStrList, &removeFileUtf8Bom);
        } else {
            cout << "Nothing was changed.\n";
        }
        system("pause");
        return 0;
    }
    if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
        cout << "UTF8-BOM.exe /your/directory -a -e md,cpp,py,txt"
                "UTF8-BOM.exe /your/directory --remove --extension=md,cpp,py,txt";
    } else if (argc == 5 || argc == 4) {
        bool m_bAddRemove = false;
        string m_sMainPath;
        vector<string> m_sStrList;
        for (int i=1; i < argc; ++i)
        {
            auto str = argv[i];
            cout << "str" << i << " is " << str << endl;
            if (str[0] != '-') {
                m_sMainPath = str;
            } else if (!strcmp(str, "--help") || !strcmp(str, "-h")) {
                return 0;
            } else if (!strcmp(str, "--add") || !strcmp(str, "-a")) {
                m_bAddRemove = true;
            } else if (!strcmp(str, "--remove") || !strcmp(str, "-r")) {
                m_bAddRemove = false;
            } else if (!strcmp(str, "-e")) {
                if (++i >= argc) return 0;
                char* ptr1 = argv[i], *ptr2 = ptr1;
                while (true) {
                    while (*ptr2 && *ptr2 != ',') ++ptr2;
                    if (ptr1 != ptr2) {
                        m_sStrList.emplace_back();
                        m_sStrList.back().resize(ptr2 - ptr1);
                        copy(ptr1, ptr2, m_sStrList.back().begin());
                    }
                    if (!*ptr2) break;
                    ptr1 = ++ptr2;
                    if (!*ptr1) break;
                }
            }  else if (!strncmp(str, "--extension=", 12)) {
                char* ptr1 = str+12, *ptr2 = ptr1;
                while (*ptr1) {
                    while (*ptr2 && *ptr2 != ',') ++ptr2;
                    if (ptr1 != ptr2) {
                        m_sStrList.emplace_back();
                        m_sStrList.back().resize(ptr2 - ptr1);
                        copy(ptr1, ptr2, m_sStrList.back().begin());
                    }
                    if (!*ptr2) break;
                    if (!*ptr2) break;
                    ptr1 = ++ptr2;
                    if (!*ptr1) break;
                }
            }  else {
                cout << "error.\n";
                return 0;
            }
        }
        if (m_sStrList.empty()) return 0;
        if (m_bAddRemove) {
            HandleFile(m_sMainPath, m_sStrList, &addFileUtf8Bom);
        } else {
            HandleFile(m_sMainPath, m_sStrList, &removeFileUtf8Bom);
        }
    } else {
        //
    }
    system("pause");
    return 0;
}
#else
int main() {
    std::string m_sMainPath = ".\\test";
    vector<string> m_sStrList {"txt", "md"};
#if 1
    HandleFile(m_sMainPath, m_sStrList, &removeFileUtf8Bom);
#else
    HandleFile(m_sMainPath, m_sStrList, &addFileUtf8Bom);
#endif
    return 0;
}
#endif

