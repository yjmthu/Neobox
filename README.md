# <img width=23 src="pluginmgr/icons/neobox.svg"/> Neobox

![GitHub top language](https://img.shields.io/github/languages/top/yjmthu/Neobox)
[![GitHub license](https://img.shields.io/badge/license-MIT-green.svg)](https://raw.githubusercontent.com/yjmthu/Neobox/master/LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/yjmthu/Neobox)](https://github.com/yjmthu/Neobox/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/yjmthu/Neobox.svg)](https://github.com/yjmthu/Neobox/network/members)
![GitHub watchers](https://img.shields.io/github/watchers/yjmthu/Neobox?color=purple)
[![GitHub issues](https://img.shields.io/github/issues/yjmthu/Neobox)](https://github.com/yjmthu/Neobox/issues)
[![GitHub closed issues](https://img.shields.io/github/issues-closed/yjmthu/Neobox)](https://github.com/yjmthu/Neobox/issues)
[![Latest release version](https://img.shields.io/github/v/release/yjmthu/Neobox?color=red)](https://github.com/yjmthu/Neobox/releases/latest)
![GitHub all releases](https://img.shields.io/github/downloads/yjmthu/Neobox/total)

- Qt6和C++20写的一个插件管理工具。安装相关插件后可获取网速显示、壁纸切换、文本翻译、文字识别、天气预报等任何功能。

## Neobox 插件管理


- 插件下载方式
    1. `托盘图标` `右键菜单` `设置中心` `插件管理`，打开 Neobox 插件管理；
    2. 在插件管理窗口里面即可下载、更新、卸载插件。

- 插件下载界面

![本地插件](./screenshots/Screenshot_20230815_183319.png)

## 插件详情

目前 Neobox 共有 **8+１** 款插件。
- 所有插件已经移动到新仓库 <https://github.com/yjmthu/neobox-plugins>
- 插件二进制文件可在 [![Gitlab](https://img.shields.io/badge/Gitlab-yellow.svg?logo=gitlab)](https://gitlab.com/yjmthu1/neoboxplg) 中查看。

## 编译环境

- `Windows 10/11 x86_64`
    - [pixi](https://pixi.sh)（推荐，见下文；提供 cmake / ninja / Qt6，并自动激活 MSVC）
    - [VS 2022+](https://visualstudio.microsoft.com/zh-hans/vs/)（需勾选“使用 C++ 的桌面开发”，pixi 会自动定位）
    - 仅手动编译需要：[Qt 6.5.x](https://www.qt.io/download)（最新版本）、cmake、ninja（可选）
- `Linux x86_64`
    - [pixi](https://pixi.sh)（推荐，见下文；提供 gcc / cmake / ninja / Qt6 / libcurl / libX11）
    - 仅手动编译需要：
        - `Arch Linux`：qt6-base、gcc、curl、cmake、ninja
        - `Fedora Linux`：qt6-base-dev、libcurl、gcc、cmake、ninja

### 使用 pixi 编译（推荐）

仓库根目录的 `pixi.toml` 把编译环境固定下来，Windows 与 Linux 共用同一份 `pixi.lock`：
cmake、ninja、Qt6 全部来自 conda-forge，Windows 上还会自动激活 MSVC（`vs2022_win-64`）。
因此既不需要单独安装 Qt，也不需要打开“VS 开发者命令提示符”。

```powershell
git clone https://github.com/yjmthu/Neobox.git
cd Neobox
pixi run init      # 拉取 thirdlib 子模块
pixi run install   # 配置 + 编译 + 安装，产物在 build/Debug 与 install/
pixi run run       # 运行刚编译好的 Neobox
```

Linux 上的命令完全相同。

- `pixi run configure` / `pixi run build` 只做配置 / 编译，`pixi run clean` 清理 `build/` 与 `install/`。
- 编译后 `compile_commands.json` 会同步到 `build/compile_commands.json`（clangd、VS Code 的默认查找位置）；
  只想刷新它而不重新编译时用 `pixi run compdb`。
- 默认构建 `Debug`，可切换构建类型：`pixi run install Release`（产物在 `build/Release`）。
- `pixi shell` 可进入已激活的交互式环境（VS Code 里的终端同样适用）。
- `pixi run init` 需要访问 GitHub，直连失败时请先配置代理。

### 手动编译【windows】

```powershell
git clone https://github.com/yjmthu/Neobox.git
cd Neobox
git submodule update --init --recursive
cmake -S . -B build/Debug -GNinja -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_INSTALL_PREFIX=install
cmake --build "./build/Debug"
cmake -P "./build/Debug/cmake_install.cmake"
```

> 需要在VS的提供命令行环境下执行上述命令。

- CMake 命令【archlinux】

```sh
sudo pacman -S qt6-base curl cmake ninja
git clone https://github.com/yjmthu/Neobox.git
cd Neobox
git submodule update --init --recursive
cmake -GNinja -B build/Debug -S . -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_INSTALL_PREFIX=install
cmake --build build/Debug
cmake -P build/Debug/cmake_install.cmake
```

> 软件在别人电脑上可能会缺失 `msvcp140.dll` 等文件，需要在 `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Redist\MSVC\xx.xx.xxxx\x64\Microsoft.VCxx.CRT` 下面拷贝所有必要的 *dll* 文件一起发布，或者将该位置添加到环境变量后在运行 `windeployqt.exe`。对于插件的打包也应将 **vcpkg** 的库文件目录添加至环境变量。


## 现有功能增强方向

<!-- > 预计在 `v2.3.x` 的版本内完成。 -->

<!-- 1. 完善自定义皮肤功能，考虑使用 `Lua` 语言来编写动画；
2. 逐步增加wxWidgets部分的代码，最终取代qt。 -->

- [ ] <del>文字识别模仿微信加强，可在图片上选中文字（tesseract难以实现）</del>；
- [x] 网速悬浮窗嵌入任务栏。
- [x] 网速悬浮窗可滚轮查看每个进程内存；
- [x] 尽量解决网速悬浮窗的闪退情况；
- [x] 插件更新功能完善，可离线管理插件、调整插件加载顺序；
- [x] 热键管理加强，增加注册失败提示，增加热键执行命令功能；
- [x] 翻译功能记住from，to语言；
- [x] 翻译功能可调节文本区域高度；
- [x] 支持设置网络代理。
- [x] 壁纸下载方式切换为异步。

## 插件开发计划

> 有什么好的想法欢迎提 issue。

- [x] 颜色拾取（PowerToys已具备，但为了跨平台还是先简单写一个吧）
- [x] 天气预报
- [ ] <del>动态壁纸（不写了）</del>
- [x] U盘助手
- [x] 后台自动更新程序
