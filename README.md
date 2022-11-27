# Neobox

- Qt6和C++20写的一个插件管理工具。安装相关插件后可获取网速显示、壁纸切换、文本翻译、天气预报等任何功能。

> 感觉用Qt来写是不是太大了？正在思考换到wxWidgets，这样既跨平台又有较小的文件体积。

## 屏幕截图

1. 原生火绒样式

    ![](https://cloud.tsinghua.edu.cn/f/cb162e06a23e4d42a772/?dl=1)
    <!-- ![](./screenshots/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202022-11-13%20234151.png) -->

2. 360卫士样式

    ![](https://cloud.tsinghua.edu.cn/f/42ef9aa2d55444759783/?dl=1)
    <!-- ![](./screenshots/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202022-11-13%20233907.png) -->

3. 电脑管家样式

    ![](https://cloud.tsinghua.edu.cn/f/1688364ff8d8477888b9/?dl=1)
    <!-- ![](./screenshots/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202022-11-13%20233257.png) -->

4. 金山毒霸样式

    ![](https://cloud.tsinghua.edu.cn/f/2ed05e162e12420f83d4/?dl=1)
    <!-- ![](./screenshots/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202022-11-14%20212223.png) -->

5. 壁纸功能

    ![](https://cloud.tsinghua.edu.cn/f/58fdaa71432c49edbc96/?dl=1)
    <!-- ![](./screenshots/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202022-11-13%20235138.png) -->

6. 有道翻译

    ![](https://cloud.tsinghua.edu.cn/f/0eac4655ae34426d9c48/?dl=1)
    <!-- ![](./screenshots/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202022-11-13%20234609.png) -->

> 你可以使用内置的这四种皮肤，也可以自己创建一个皮肤。

## 开发进度

1. 插件化所有模块，完善插件基类和插件管理器；
2. 一定程度上重构壁纸功能，合并壁纸配置文件为一个JSON文件；
3. 完善自定义皮肤功能，考虑使用Lua语言来编写动画；
4. Windows下使用原生网络库取代curl；
4. 逐步增加wxWidgets部分的代码，最终取代qt。

## 编译环境

> 由于经过重构，目前代码不太成熟，编译后不一定能正常运行。

1. Windows x86_64
    - <del>xmake+xrepo</del> **CMake+Vcpkg**
    - MSVC 2022
    - Qt 6.4.1（最新版本）
    - vcpkg (libcurl, leptonica, tesseract)
    - json库 [YJson](https://github.com/yjmthu/YJson)
2. Linux
    - 更换系统后未曾尝试编译（待 GCC13 发布稳定后再考虑写linux部分代码）

## 核心功能

1. 网速、内存、CPU占用显示
2. 壁纸切换
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
3. 系统优化
    - 文件资源管理器右键增加“复制路径”选项
    - 防止息屏、快速关机、快捷重启等选项在右键菜单，让操作更方便
4. 可能会要添加的功能
    - 悬浮窗嵌入任务栏
    - 热键注册管理
    - 右键菜单功能自定义
    - 可查看CPU温度、磁盘使用率
    - U盘助手

## 额外功能

- 文本翻译，单词翻译（富文本渲染）
- 支持拖拽文字翻译
- 支持读取剪切板
- 屏幕文字识别，快捷键打开
- 支持用户自行添加训练数据
    -支持用户自行选择语言，默认为中文简体加英语
- 屏幕颜色拾取
