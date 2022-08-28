#include <array>
#include <filesystem>
#include <iostream>
#include <queue>
#include <regex>
#include <string>

int main()
{
    using namespace std::literals;
    std::string _folderStr;
    std::cout << "Please enter the folder path:\n";
    std::getline(std::cin, _folderStr);
    const std::filesystem::path _folderPath = _folderStr;
    if (!std::filesystem::exists(_folderStr))
    {
        std::cout << _folderPath << " is not a folder.\n";
        return 0;
    }
    if (!std::filesystem::is_directory(_folderStr))
    {
        std::cout << _folderPath << " is a file.\n";
        return 0;
    }
    auto _isImageFile = [](const std::filesystem::path &_path) {
        const std::regex _pattern("^.*\\.(cpp|c|h|hpp|cc)$");
        return std::regex_match(_path.string(), _pattern);
    };
    std::array _patterns = {"build-rel"s, "build"s, "build-dbg"s, "out"s,     "\\.git"s,
                            "env"s,       "glad"s,  "glfw"s,      "freetype"s};
    for (auto &i : _patterns)
    {
        i = "^.*" + i + "$";
    }
    auto _isInWhiteList = [&_patterns](const std::filesystem::path &_path) {
        std::string str = _path.string();
        for (auto &i : _patterns)
        {
            if (std::regex_match(str, std::regex(i)))
                return true;
        }
        return false;
    };
    std::queue<std::filesystem::path> _stack;
    _stack.push(std::move(_folderPath));
    while (!_stack.empty())
    {
        auto &_tmpFolderPath = _stack.front();
        std::cout << "Cd into " << _tmpFolderPath << std::endl;
        for (auto &iter : std::filesystem::directory_iterator(_tmpFolderPath))
        {
            const auto &_path = iter.path();
            if (std::filesystem::is_directory(iter.status()))
            {
                if (!_isInWhiteList(_path))
                    _stack.push(_path);
                continue;
            }
            else if (std::filesystem::is_regular_file(iter.status()))
            {
                if (_isImageFile(_path))
                {
                    std::cout << "Format file " << _path << std::endl;
                    std::string _command = "clang-format --style=microsoft -i ";
                    _command.append(_path.string());
                    system(_command.c_str());
                }
            }
        }
        std::cout << "Cd out " << _tmpFolderPath << std::endl;
        _stack.pop();
    }
    return 0;
}
