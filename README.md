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

目前 Neobox 共有 **8** 款插件，源代码在 `plugins` 目录下。插件二进制文件可在 [![Gitlab](https://img.shields.io/badge/Gitlab-yellow.svg?logo=gitlab)](https://gitlab.com/yjmthu1/neoboxplg) 中查看。

- 插件下载方式
    1. `托盘图标` `右键菜单` `设置中心` `插件管理`，打开 Neobox 插件管理；
    2. 在插件管理窗口里面即可下载、更新、卸载插件。

- 插件下载界面

<img width="400" alt="Neobox 本地插件" src="https://cloud.tsinghua.edu.cn/f/c5b662d65cf2474d94c5/?dl=1"><img width="397" alt="Neobox 网络插件" src="https://cloud.tsinghua.edu.cn/f/d91c6a5a26314764825f/?dl=1">

## 插件详情

- 所有插件已经移动到新仓库 <https://github.com/yjmthu/neobox-plugins>

## 编译环境

- `Windows 10/11 x86_64`
    - <del>xmake+xrepo</del> **[CMake](https://cmake.org/download/)+Vcpkg**
    - [VS 2022](https://visualstudio.microsoft.com/zh-hans/vs/)
    - [Qt 6.5.1](https://www.qt.io/download)（最新版本）
    - c++20 JSON库 [YJson](https://github.com/yjmthu/YJson)
- `Arch Linux x86_64`
    - 主程序库依赖: `kde` `curl` `qt6-base`
    - 目前主程序编译成功，但无可用插件
    - 正在使用clang补写linux部分功能
    - 待 GCC13 发布后再切换到 GCC

- CMake 命令【windows】

```powershell
vcpkg install leptonica:x64-windows Tesseract:x64-windows
vcpkg integrate install
git clone https://github.com/yjmthu/Neobox.git
cd Neobox
git submodule update --init --recursive
cmake -S . -GNinja -DCMAKE_BUILD_TYPE="Debug" -B build/Debug -DCMAKE_TOOLCHAIN_FILE=${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake
cmake --build "./build/Debug"
cmake -P "./build/Debug/cmake_install.cmake"
```

> 需要在VS的提供命令行环境下执行上述命令。

- CMake 命令【linux】

```shell
sudo pacman -S qt6-base curl cmake ninja clang libc++
git clone https://github.com/yjmthu/Neobox.git
cd Neobox
git submodule update --init --recursive
cmake -GNinja -B build/Debug -S . -DCMAKE_BUILD_TYPE="Debug"
cmake --build build/Debug
cmake -P build/Debug/cmake_install.cmake
```


## 现有功能增强方向

<!-- > 预计在 `v2.3.x` 的版本内完成。 -->

<!-- 1. 完善自定义皮肤功能，考虑使用 `Lua` 语言来编写动画；
2. 逐步增加wxWidgets部分的代码，最终取代qt。 -->

- [ ] 文字识别模仿微信加强，可在图片上选中文字；
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

> 这是我目前打算新开发的插件，如果有什么建议或者想参与插件开发，可以联系我。

- [x] 颜色拾取（PowerToys已具备，但为了跨平台还是先简单写一个吧）
- [x] 天气预报
- [ ] 动态壁纸
- [x] U盘助手
- [x] 后台自动更新程序
