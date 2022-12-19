# <img width=23 src="widgets/icons/neobox.svg"/> Neobox

[![GitHub issues](https://img.shields.io/github/issues/yjmthu/Neobox.svg)](https://github.com/yjmthu/Neobox/issues) 
[![GitHub stars](https://img.shields.io/github/stars/yjmthu/Neobox.svg)](https://github.com/yjmthu/Neobox/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/yjmthu/Neobox.svg)](https://github.com/yjmthu/Neobox/network/members)
![GitHub watchers](https://img.shields.io/github/watchers/yjmthu/Neobox.svg)
[![GitHub license](https://img.shields.io/badge/license-MIT-green.svg)](https://raw.githubusercontent.com/yjmthu/Neobox/master/LICENSE)
[![Latest release version](https://img.shields.io/badge/version-2.1-red.svg)](https://github.com/yjmthu/Neobox/releases/latest)

- Qt6和C++20写的一个插件管理工具。安装相关插件后可获取网速显示、壁纸切换、文本翻译、文字识别、天气预报等任何功能。

## Neobox 插件

目前 Neobox 共有6款插件，可在 [![Gitlab](https://img.shields.io/badge/Gitlab-yellow.svg?logo=gitlab)](https://gitlab.com/yjmthu1/neoboxplg) 中查看。

- 插件下载方式
    1. `托盘图标` `右键菜单` `设置中心` `插件管理`，打开 Neobox 插件管理；
    2. 在插件管理窗口里面即可下载、更新、卸载插件。

- 插件下载界面

![Neobox 插件](https://cloud.tsinghua.edu.cn/f/c5b662d65cf2474d94c5/?dl=1)

<details>
<summary>网速悬浮插件</summary>

功能：网速、内存、CPU占用显示。

1. 原生样式

    ![](https://cloud.tsinghua.edu.cn/f/cb162e06a23e4d42a772/?dl=1)

2. 卫士样式

    ![](https://cloud.tsinghua.edu.cn/f/42ef9aa2d55444759783/?dl=1)

3. 管家样式

    ![](https://cloud.tsinghua.edu.cn/f/1688364ff8d8477888b9/?dl=1)


4. 毒霸样式

    ![](https://cloud.tsinghua.edu.cn/f/2ed05e162e12420f83d4/?dl=1)

> 你可以使用内置的这四种皮肤，也可以自己创建一个皮肤。

</details>

<details>
<summary>壁纸引擎插件</summary>

+ 手动切换、定时切换、收藏夹、黑名单
+ 网络壁纸源
    - Awesome Wallpapers: <https://wallhaven.cc/>
    - Bing: <https://www.bing.com/>
    - Unsplash: <https://unsplash.com/>
    - 小歪: <https://api.ixiaowai.cn/>
    - 其他壁纸Api链接（必须是直接在浏览器打开就能看到图片的链接，例如<https://source.unsplash.com/random/2500x1600>）
+ 本地壁纸源
    - 可遍历壁纸文件夹
    - 可调用脚本获取本地壁纸路径
    - 用户收藏夹内的壁纸
+ 拖拽壁纸源
    - 如果安装了网速悬浮插件的话，可以拖拽网页或者本地的图片到悬浮窗，也是可以设置壁纸的。
+ 屏幕截图

![](https://cloud.tsinghua.edu.cn/f/f1bec3fe13a94a9794a5/?dl=1)

![](https://cloud.tsinghua.edu.cn/f/7db62f1da80f4374b742/?dl=1)

</details>


<details>
<summary>极简翻译插件</summary>

- 简介：普通模式调用百度翻译Api，查词模式调用有道翻译Api。

![极简翻译](https://cloud.tsinghua.edu.cn/f/ad12e8d1452549789dc1/?dl=1)

</details>

<details>
<summary>文字识别插件</summary>

- 简介：截图识别多种语言文字，需要自行下载相应语言的训练数据。目前依赖于极简翻译插件来输出识别结果。

![文字识别](https://cloud.tsinghua.edu.cn/f/612106e8c64c49c393c8/?dl=1)

![文字识别](https://cloud.tsinghua.edu.cn/f/42e2e76532a2416aa9fa/?dl=1)

> 技巧：按住鼠标中键可移动选框，按ESC键退出框选。

</details>

<details>
<summary>系统控制插件</summary>

- 简介：提供防止息屏、右键复制、快速关机、重启、睡眠等功能。

![系统控制](https://cloud.tsinghua.edu.cn/f/c27ae43c1ca242419ad6/?dl=1)

</details>

<details>
<summary>热键管理插件</summary>

- 简介：注册并捕获系统全局热键，并将结果发送至相应插件。至于插件具体作何反应与此插件无关。

![热键管理](https://cloud.tsinghua.edu.cn/f/11eae0e195d6402685d9/?dl=1)

</details>

## 插件开发计划

> 这是我目前打算新开发的插件，如果有什么建议或者想参与插件开发，可以联系我。

1. 颜色拾取
2. 天气预报
3. 动态壁纸
4. U盘助手
5. 任务栏网速

## 编译环境

- `Windows10+ x86_64`
    - <del>xmake+xrepo</del> **[CMake](https://cmake.org/download/)+Vcpkg**
    - [MSVC 2022](https://visualstudio.microsoft.com/zh-hans/vs/)
    - [Qt 6.4.1](https://www.qt.io/download)（最新版本）
    - [vcpkg](https://github.com/microsoft/vcpkg) (libcurl[linux], leptonica, tesseract)
    - c++20 JSON库 [YJson](https://github.com/yjmthu/YJson)
- `Linux x86_64`
    - 更换系统后未曾尝试编译（待 GCC13 发布稳定后再考虑写linux部分代码）


## 开发进度

1. 完善自定义皮肤功能，考虑使用 `Lua` 语言来编写动画；
2. 逐步增加wxWidgets部分的代码，最终取代qt。
