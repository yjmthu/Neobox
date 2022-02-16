#include <string>
#include <iostream>
#include <io.h>

#include "yjson.h"

void transDirToJson(const std::string& path, _finddata_t* fileData, YJson* file, YJson*folder)
{
    std::string tempPath = path + "\\*";
    auto x = _findfirst(tempPath.c_str(), fileData);
    while (_findnext(x, fileData) == 0) {
        if (!strcmp(fileData->name, ".."))
            continue;
        if (fileData->attrib & _A_SUBDIR) {
            YJson* temp = folder->append(YJson::Object, fileData->name);
            transDirToJson(path+"\\"+fileData->name, fileData, temp->append(YJson::Array, "File"), temp->append(YJson::Object, "Folder"));
        } else {
            file->append(fileData->name);
        }
    }
    _findclose(x);
}

void transDirToJson(const std::string& folder, bool fmt=false)
{
    YJson js(YJson::Object);
    _finddata_t m_dFileData;
    transDirToJson(folder, &m_dFileData, js.append(YJson::Array, "File"), js.append(YJson::Object, "Folder"));
    js.toFile("directory.json", YJson::ANSI, fmt);
}

int main() {
    std::string mainPath;
    std::cout << "Please input the Directory.\n";
    std::cin >> mainPath;
    transDirToJson(mainPath, 0);
    return 0;
}
