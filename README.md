<head>
<title>Speed Box</title>
<style>
	img {
		background-color: rgba(0,0,0,0);
	}
</style>
</head>

# &nbsp;<img src="https://gitee.com/yjmthu/Speed-Box/raw/main/src/icons/speedbox.ico" width=32 />&nbsp; Speed-Box 简介
Speed Box 是一款集众多常用功能于一身的网速显示软件，单界面内存占用6-15M，目前支持
- 实时网速——悬浮窗显示网速和内存占用率。
- 智能隐藏——全屏时自动隐藏、贴边时自动隐藏。
- 划词翻译——手动输入中英文翻译、选中文本后通过快捷键"Ctrl+C"和“Shift+Z”实现划词翻译。
- 切换壁纸——后台/手动切换[Wallhaven](https://wallhaven.cc/)、[Bing](https://cn.bing.com/)、[Unsplash](https://unsplash.com/)、电脑本地 以及所有其它网站的图片，将不喜欢的壁纸直接删除或加入黑名单。
- 任务栏美化——改变颜色、图标居中、靠右、透明效果、磨砂效果、玻璃效果，桌面右下角时间格式精确到秒。（ **借鉴了坛友cgbsmy发布的 [ TrayS ](https://www.52pojie.cn/forum.php?mod=viewthread&tid=1182669&highlight=tray)** ）
- 快捷功能——悬浮窗右键关机、切换壁纸、系统右键增加“复制路径”选项。
- 防止息屏——防止电脑屏幕自动关闭。
- 隐藏图标——通过鼠标滑动隐藏/显示桌面上的所有图标。
- 自动更新——点击按钮自动下载更新。

**源代码** ：[![](https://img.shields.io/badge/Github-367aff?color=black&logo=github)](https://github.com/yjmthu/Speed-Box)、[![](https://img.shields.io/badge/Gitee-367aff?color=red&logo=gitee)](https://gitee.com/yjmthu/Speed-Box)

> **提示：github上的代码才是最新的哦**

# 详细功能介绍
在Speed Box的网速悬浮窗处单击右键，即可看到有许多快捷功能，这些功能看文字就很容易知道是什么意思了。再点击其中的“软件设置”可以打开设置界面，可以看到这里有 *壁纸设置* 、 *桌面美化* 、 *路径设置* 、 *其它设置*  等。
**大多数设置都有Tool Tip提示**，你只需要把鼠标放在按钮上面悬停一下就可以看到它的功能解释。这里需要详细说明的功能只有三个个——翻译功能、路径设置、壁纸设置中的高级命令。

## \$ 划词翻译
* 如果你在启用了翻译功能，就需要输入百度翻译的app id和密码。我这里暂时提供一个APP ID：**<u>20210503000812254</u>**，密钥：**<u>Q_2PPxmCr66r6B2hi0ts</u>**。~~注册一个ID是免费的，如果不嫌麻烦的话可以自己去[注册](http://api.fanyi.baidu.com/)~~。
* 在右键菜单**启用**划词翻译后，你可以鼠标**左键双击**悬浮窗，“极简翻译”的窗口就会自己弹出来了，你还可以按下 <kbd>Shift</kbd>+<kbd>Z</kbd>键，直接呼唤出划词翻译，你也可以在 *选中* 一段文本后，按下<kbd>Ctrl</kbd>+<kbd>C</kbd>键将其复制，再按下<kbd>Shift</kbd>+<kbd>Z</kbd>键将其直接翻译。如果启用了<img src="https://gitee.com/yjmthu/Speed-Box/raw/main/src/icons/drip_pin.ico" width=14 />，“极简翻译”窗口就会不自动消失，如果取消选中<img src="https://gitee.com/yjmthu/Speed-Box/raw/main/src/icons/drip_blue_pin.ico" width=14 />，“极简翻译”就会在10秒钟之后自动消失。此外，你还可以用快捷键<kbd>Shift</kbd>+<kbd>A</kbd>关闭“极简翻译”窗口。当输入焦点在“极简翻译”内的时候，你可以 **回车直接翻译，按下<kbd>ALt</kbd>切换中英文结果，按下<kbd>Delete</kbd>键清空输入，按Esc关闭窗口** 。


## \$ 路径设置
* 打开**设置窗口-路径设置**后，这里有“标准名称”和“新名称”之分，标准名称就是软件默认使用存放壁纸文件的文件夹的名称，新名称就是你想把它改成的名称。
* 壁纸文件夹默认在“用户/图片”里面的“Wallpapers”目录下，你可以点击“壁纸下载目录”后面的“打开”按钮进行查看。当你想要更换壁纸下载路径时，直接输入路径回车即可。
* 还有一个就是右键菜单的“打开目录功能”，你可以在这里进行自定义每次点击 **“右键菜单-打开目录“** 时要打开的目录。


## \$ 高级命令
这里举个例子，假设我们会![](https://img.shields.io/badge/Python-367aff?color=brown&logo=python)，在`D:\hello.py`里面写了爬虫代码并且成功下载了一张图片保存在`D:\hello.jpg`，在`D:\hello.py`里面注释掉所有的print输出，只保留一句
```python
print("D:\\hello.jpg")
```
这时候我们就可以把Speed Box里面的高级命令设置为
`python D:\hello.py`
然后Speed Box就可以自动调用这个脚本了。其它语言的脚本或者程序也一样，只要输出图片路径，Speed Box就能自动识别，并且能定时调用这个脚本，也可以点击在右键菜单里面点击 *“下一张图”* 调用。

# 运行效果

### 1\. 悬浮窗界面、壁纸设置界面和翻译界面
![加载失败](https://gitee.com/yjmthu/Speed-Box/raw/main/img/img_08.png)


### 2\. 悬浮窗界面、桌面美化界面和右键菜单界面
![加载失败](https://gitee.com/yjmthu/Speed-Box/raw/main/img/img_09.jpg)


### 3\. 悬浮窗界面、Tooltip、任务栏美化、图标居中
![加载失败](https://gitee.com/yjmthu/Speed-Box/raw/main/img/img_06.jpg)


### 4\. 悬浮窗美化之圆角效果、玻璃效果、透明效果
![加载失败](https://gitee.com/yjmthu/Speed-Box/raw/main/img/img_11.jpg)


# 下载方式

> **提示：21.9.10及以上版本直接在"右键菜单->软件设置->其他设置->关于"中点击"下载更新"按钮即可完成更新**

### 版本：v21.10.1（最新版本） ![](https://img.shields.io/badge/热烈庆祝中华人民共和国成立72周年！-Megix?color=fff&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAgCAYAAABU1PscAAAAAXNSR0IArs4c6QAAAORlWElmTU0AKgAAAAgABgESAAMAAAABAAEAAAEaAAUAAAABAAAAVgEbAAUAAAABAAAAXgExAAIAAAAfAAAAZgEyAAIAAAAUAAAAhodpAAQAAAABAAAAmgAAAAAAAABIAAAAAQAAAEgAAAABQWRvYmUgUGhvdG9zaG9wIDIxLjAgKFdpbmRvd3MpAAAyMDIwOjEyOjI4IDA5OjI2OjU1AAAEkAQAAgAAABQAAADQoAEAAwAAAAEAAQAAoAIABAAAAAEAAAAwoAMABAAAAAEAAAAgAAAAADIwMjE6MDE6MDkgMTY6MzQ6NDAA5X3tpAAAAAlwSFlzAAALEwAACxMBAJqcGAAACtJpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IlhNUCBDb3JlIDYuMC4wIj4KICAgPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICAgICAgPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIKICAgICAgICAgICAgeG1sbnM6ZGM9Imh0dHA6Ly9wdXJsLm9yZy9kYy9lbGVtZW50cy8xLjEvIgogICAgICAgICAgICB4bWxuczp4bXA9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC8iCiAgICAgICAgICAgIHhtbG5zOnhtcE1NPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvbW0vIgogICAgICAgICAgICB4bWxuczpzdEV2dD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL3NUeXBlL1Jlc291cmNlRXZlbnQjIgogICAgICAgICAgICB4bWxuczpwaG90b3Nob3A9Imh0dHA6Ly9ucy5hZG9iZS5jb20vcGhvdG9zaG9wLzEuMC8iCiAgICAgICAgICAgIHhtbG5zOnRpZmY9Imh0dHA6Ly9ucy5hZG9iZS5jb20vdGlmZi8xLjAvIj4KICAgICAgICAgPGRjOmZvcm1hdD5pbWFnZS9wbmc8L2RjOmZvcm1hdD4KICAgICAgICAgPHhtcDpNb2RpZnlEYXRlPjIwMjAtMTItMjhUMDk6MjY6NTUrMDg6MDA8L3htcDpNb2RpZnlEYXRlPgogICAgICAgICA8eG1wOkNyZWF0b3JUb29sPkFkb2JlIFBob3Rvc2hvcCAyMS4wIChXaW5kb3dzKTwveG1wOkNyZWF0b3JUb29sPgogICAgICAgICA8eG1wOkNyZWF0ZURhdGU+MjAyMS0wMS0wOVQxNjozNDo0MCswODowMDwveG1wOkNyZWF0ZURhdGU+CiAgICAgICAgIDx4bXA6TWV0YWRhdGFEYXRlPjIwMjAtMTItMjhUMDk6MjY6NTUrMDg6MDA8L3htcDpNZXRhZGF0YURhdGU+CiAgICAgICAgIDx4bXBNTTpIaXN0b3J5PgogICAgICAgICAgICA8cmRmOlNlcT4KICAgICAgICAgICAgICAgPHJkZjpsaSByZGY6cGFyc2VUeXBlPSJSZXNvdXJjZSI+CiAgICAgICAgICAgICAgICAgIDxzdEV2dDpzb2Z0d2FyZUFnZW50PkFkb2JlIFBob3Rvc2hvcCAyMS4wIChXaW5kb3dzKTwvc3RFdnQ6c29mdHdhcmVBZ2VudD4KICAgICAgICAgICAgICAgICAgPHN0RXZ0OndoZW4+MjAyMS0wMS0wOVQxNjozNDo0MCswODowMDwvc3RFdnQ6d2hlbj4KICAgICAgICAgICAgICAgICAgPHN0RXZ0Omluc3RhbmNlSUQ+eG1wLmlpZDplMzkyNjg5OS1iMGI4LWNhNDctYWJkMC0yODkwMTZkNWNkNDc8L3N0RXZ0Omluc3RhbmNlSUQ+CiAgICAgICAgICAgICAgICAgIDxzdEV2dDphY3Rpb24+Y3JlYXRlZDwvc3RFdnQ6YWN0aW9uPgogICAgICAgICAgICAgICA8L3JkZjpsaT4KICAgICAgICAgICAgICAgPHJkZjpsaSByZGY6cGFyc2VUeXBlPSJSZXNvdXJjZSI+CiAgICAgICAgICAgICAgICAgIDxzdEV2dDpzb2Z0d2FyZUFnZW50PkFkb2JlIFBob3Rvc2hvcCAyMS4wIChXaW5kb3dzKTwvc3RFdnQ6c29mdHdhcmVBZ2VudD4KICAgICAgICAgICAgICAgICAgPHN0RXZ0OmNoYW5nZWQ+Lzwvc3RFdnQ6Y2hhbmdlZD4KICAgICAgICAgICAgICAgICAgPHN0RXZ0OndoZW4+MjAyMS0wMS0wOVQxNjozNzowNyswODowMDwvc3RFdnQ6d2hlbj4KICAgICAgICAgICAgICAgICAgPHN0RXZ0Omluc3RhbmNlSUQ+eG1wLmlpZDoyOGQ3ZjhmNi04ZmFkLWY0NDQtYjdiMS05NDhlNmEwYzVlMjg8L3N0RXZ0Omluc3RhbmNlSUQ+CiAgICAgICAgICAgICAgICAgIDxzdEV2dDphY3Rpb24+c2F2ZWQ8L3N0RXZ0OmFjdGlvbj4KICAgICAgICAgICAgICAgPC9yZGY6bGk+CiAgICAgICAgICAgICAgIDxyZGY6bGkgcmRmOnBhcnNlVHlwZT0iUmVzb3VyY2UiPgogICAgICAgICAgICAgICAgICA8c3RFdnQ6c29mdHdhcmVBZ2VudD5BZG9iZSBQaG90b3Nob3AgMjEuMCAoV2luZG93cyk8L3N0RXZ0OnNvZnR3YXJlQWdlbnQ+CiAgICAgICAgICAgICAgICAgIDxzdEV2dDpjaGFuZ2VkPi88L3N0RXZ0OmNoYW5nZWQ+CiAgICAgICAgICAgICAgICAgIDxzdEV2dDp3aGVuPjIwMjAtMTItMjhUMDk6MjY6NTUrMDg6MDA8L3N0RXZ0OndoZW4+CiAgICAgICAgICAgICAgICAgIDxzdEV2dDppbnN0YW5jZUlEPnhtcC5paWQ6MGI4M2Y4ZTAtYTEwMS1kZDRhLTgwZGMtZjE1ODMyYmQ2OTNlPC9zdEV2dDppbnN0YW5jZUlEPgogICAgICAgICAgICAgICAgICA8c3RFdnQ6YWN0aW9uPnNhdmVkPC9zdEV2dDphY3Rpb24+CiAgICAgICAgICAgICAgIDwvcmRmOmxpPgogICAgICAgICAgICA8L3JkZjpTZXE+CiAgICAgICAgIDwveG1wTU06SGlzdG9yeT4KICAgICAgICAgPHhtcE1NOk9yaWdpbmFsRG9jdW1lbnRJRD54bXAuZGlkOmUzOTI2ODk5LWIwYjgtY2E0Ny1hYmQwLTI4OTAxNmQ1Y2Q0NzwveG1wTU06T3JpZ2luYWxEb2N1bWVudElEPgogICAgICAgICA8eG1wTU06RG9jdW1lbnRJRD5hZG9iZTpkb2Np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&style=for-the-badge)

- 更新内容
  * 改变了官方网站，去除了github和gitee。
  * 增加了语音朗读功能，翻译速度过快不会提示失败而是等待一秒以内自动重试。
  * 优化内存占用，修复了几个细节上的bug。
  * 皮肤颜色增加暗夜黑，悬浮窗效果增加了默认、透明、玻璃、亚克力四种效果。
  * 支持翻译多行。普通回车直接触发翻译功能，如需换行，请先打一个空格然后按下回车。
  * 增加了隐藏桌面图标功能，在"右键菜单->软件设置->桌面美化->桌面图标控制"中勾选"开启"，即可在桌面最左边上/下滑动显示桌面图标，在桌面最右边上/下滑动隐藏桌面图标。
- 下载方式
  * [![](https://img.shields.io/badge/Windows%20x64%2021.10.1-蓝奏云-367aff?logo=windows)](https://wws.lanzoui.com/b020l1esh)
  * 密码:  <input type="image" onclick="mCopy('52pojie')" src="https://img.shields.io/badge/52pojie-Megix?color=fff&amp;logo=data:image/png;base64,AAABAAEAICAAAAAAAACoDAAAFgAAACgAAAAgAAAAQAAAAAEAGAAAAAAAgAwAAAAAAAAAAAAAAAAAAAAAAAD////////BwfdQUOr8/P7////Dw/ewsPWWlvKoqPTk5PvU1Pnt7fy0tPWFhfBxce7d3fu6uvapqfWnp/RZWev///+UlPJqau3T0/m8vPf09P3////j4/vLy/n///////////8JCeIKCuIFBeEAAOAAAOAAAOAAAOAAAOEAAOAAAOAAAOAAAOAAAOADA+EEBOEAAOAAAOEAAOAAAOAFBeEAAN8AAOAGBuEAAOAAAOAAAOAAAN8AAOAAAN+oqPX///+rq/QAAOAREeMAAOEAAOEAAOEDA+EHB+ILC+IPD+IQEOIAAOEAAOAQEOIREeMREeMQEOILC+JkZOwREeMbG+RfX+wMDOIAAOACAuEAAOAuLuZcXOsUFOMODuIAAN7JyfhOTuoNDeIAAODd3fqZmfKdnfKBgfCRkfHOzvkAAN4AAN+rq/SysvYAAN8ODuIREeMQEOIAAOD+/v6UlPIuLubr6/z////V1fp1de6/v/f///+EhPABAeEWFuMAAOCurvVzc+4GBuEuLubX1/plZe2SkvFycu5WVuvu7vybm/OlpfTMzPnNzflVVesSEuMODuIREeMJCeI2Nuf///8EBOEAAN5LS+n///////////+BgfAAAN8dHeQPD+IBAeGcnPOvr/UAAOEAAN/MzPj4+P3j4/v///+dnfP4+P5bW+tubu25ufbp6fzc3PqpqfUCAuEREeMQEOIAAN/U1PmMjPE/P+jAwPdubu0AAN4PD+L6+v5sbO0HB+EQEOIAAOC/v/fLy/kAAOAAAODf3/uurvWtrfWBgfAnJ+X///8nJ+UAAN+IiPGhofQAANsAAOAQEOIREeMREeMAAOAYGOT////s7P38/P7o6Py7u/eXl/Lx8f2amvMBAeEZGeQHB+FHR+n///8ICOIAAODR0fm/v/fR0fnNzflMTOr///7u7v339/7///////////+CgvAFBeEREeMCAuGhofOGhvD///+VlfI1NedVVep2du6NjfFkZOwAAOAJCeIPD+IAAOH///+5ufYQEOIAAODX1/qKivHV1fn7+/5XV+v4+P18fO+oqPWtrfTBwfgdHeQcHOQODuIQEOIHB+GMjPGiovOHh/D////09P3+/v79/f7+/v/5+f7m5vwuLuYLC+IBAeHh4fv///8ICOEAAODe3vovL+adnfJ5ee4wMOb7+/4/P+hWVusxMeZOTuoAAOAJCeIREeMREeMAAOGQkPHFxfcAANq+vvejo/MAAN0NDeIoKOVXV+v///8vL+YMDOIAAOGXl/Pp6fwwMOYAAODZ2fra2vr///////////95ee+1tfaMjPFJSen////8/P4TE+MNDeIQEOIAAOD////////t7fzPz/n///8pKeUHB+EAAN8AAN77+/6Hh/AJCeIBAeGDg/BhYewAAOAPD+KMjPEAAOAAAN54eO////8HB+IkJOX///8AAOAAAN3MzPmgoPMBAeEPD+IREeNxce4CAuHFxfjCwvfc3Pr////////j4/vk5Pz///+GhvAEBOEZGeSDg/CcnPMAAOAjI+Tz8/3W1vmvr/VlZeza2vrx8f0AANzZ2fqJifAAAN2dnfPGxvgAAOAREeMNDeINDeIFBeH///8AAOClpfRkZOxAQOj///9qau0rK+YAAOEODuIcHORpae1qau0EBOEUFOMAAN/f3/v///+qqvW1tfZSUurDw/f09P3///9tbe3///94eO4EBOEREeMDA+GQkPH///////+urvX///+Dg/ALC+JbW+u6uvcAAN8QEOIQEOMAAN////+5ufYKCuIPD+IMDOIBAeH///+goPMAAN8AAOBJSemNjfGsrPXCwvd0dO4AAOEQEOIREeMQEOIEBOESEuMzM+deXux+fu+7u/fo6Pz///////8lJeUMDOIREeMAAODCwvfi4vsAAOAQEOIREeMLC+IfH+QhIeQNDeIQEOMHB+EWFuMAAOAAAOADA+EQEOIREeMREeMREeMQEOINDeIJCeIGBuEDA+EAAOAAAOAJCeEAAOAPD+IQEOIQEOIAAOC6uvdtbe0ICOEQEOMQEOIREeMNDeIMDOIODuIGBuEPD+IPD+IREeMPD+ImJuUPD+IREeMREeMREeMREeMPD+IWFuMGBuIAAOAAAOAWFuMMDOJQUOoKCuISEuIYGOMAAOD////NzfkAAOAQEOIVFeMQEOIPD+IjI+QcHOR0dO4JCeIAAOANDeIQEOMHB+EGBuEREeMMDOIQEOMQEOIAAOBPT+qkpPTExPe9vfeenvNRUeoAAOAMDOIQEOIQEOIAAOBeXuvg4PsAAOAQEOIREeMJCeIAAOEAAOEwMOb///9vb+3BwfcQEOIAAN9ISOl2du8CAuFFRekLC+IAAOC2tvb///+RkfF0dO6AgPCnp/T////29v4QEOINDeIKCuJISOmysvbr6/wDA+EPD+IGBuFBQei1tfZZWesCAuH///8iIuS+vvf///+fn/P///98fO8EBOEKCuIzM+YAAN////8bG+QAAN8AAOECAuEAAOAICOH///+QkPECAuEQEOICAuGPj/Hl5fwAAOAQEOMVFeP///+WlvL///9ZWev4+P0bG+QDA+H///////9ISOkAAOAREeMPD+IgIOQAAN////+pqfREROkcHOQDA+EAAOAAAOD///+envMBAeEODuIYGOOqqvV9ffAjI+UAAOFBQej39/4AAN5lZezT0/nf3/siIuTi4vuVlfJnZ+z///8AAOAPD+IREeMLC+IAAOCiovPn5/zm5vv////////////////s7PwVFeMHB+EPD+IAAOD///+NjfEwMOY/P+hra+3///4AAN8pKeX09P7MzPg/P+gAAN8AAN8YGOP+/v5LS+kJCeIMDOInJ+UzM+cuLuYgIOQXF+MsLOYyMudlZewxMeYeHuQoKOUuLuYYGOMAAOCgoPTt7fwAAN9pae3q6vz///8/P+iBgfDn5/yqqvTS0vn09P1cXOs1Nef///9ZWesHB+ENDeKoqPT9/f7w8P38/P7////7+/7///////////////////////8AAOABAeGcnPO7u/YCAuEAAN/l5fzCwvfm5vv///9DQ+ilpfSjo/RRUer///////+8vPcFBeEODuIcHOQBAeEAAN8AAOAXF+P4+P4AANwAAN4AAN2oqPSZmfILC+IjI+QQEOMBAeGXl/J9fe8YGOMICOInJ+X7+/4AAN0AAN8AAN+np/TDw/cAAOD///8AAN0ICOEFBeEREeMODuIREeMAAODx8f3///////+8vPeYmPJ7e+/NzfmurvUAAOAMDOIQEOIDA+GNjfLS0vkAAOAQEOIAAN+goPOhofMAAN4AAN+Xl/Lx8f0AAN7///8AAN////85OecICOIeHuQPD+IMDOIEBOEoKOX////IyPja2vr+/v7q6vwrK+YLC+IREeIVFeMBAeGlpfX///8ZGeQDA+F4eO/5+f7////w8P3+/v719f7////v7/3///9lZe3w8P28vPcAAOAXF+MNDeIcHOQLC+ItLebl5fxoaO0AAN0AAN0AAN0AAN8AAOAQEOMQEOIAAODT0/l3d+8BAeENDeIlJeVISOk/P+hEROlISOkaGuMkJOVpae3///+qqvWjo/MrK+ULC+IQEOIAAODIyPj////////////////////h4fvm5vzn5/zm5vwAAOAQEOMAAODMzPldXewEBOEQEOMLC+IICOEVFeIPD+IICOIMDOIHB+EcHOT///8AAN4AAOALC+IQEOINDeILC+IzM+ZISOlEROlFRelISOlMTOpMTOlMTOpSUupLS+oICOEODuILC+L///////8AAN8EBOEmJuUAAOAAAOAICOIAAOACAuEAAOAEBOFGRukZGeQPD+ILC+IAAOElJeUCAuEAAOAAAN8dHeQAAN8tLeYAAN8AAOAAAOAAAN8PD+MODuIAAOAAAOD+/v7////q6vxiYuzOzvnT0/r39/6hofPb2/qvr/W9vfaUlPODg/Dc3Ps5Oed3d++vr/X09P1SUurn5/ympvRqau2BgfBUVOrs7PzKyvh1de/i4vvi4vuenvSCgvDn5/z///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==" />


### 版本：v21.9.10
- 更新内容
  * 修复一些比较影响使用的问题
  * 将开机自启移动到了软件设置-其他设置下，移除了快捷重启功能。
  * 增加了壁纸历史记录功能，可以通过右键菜单来切换上一张图。
  * 增加了壁纸黑名单功能，可以将不喜欢的wallhaven壁纸删除并加入黑名单，将其他类型的壁纸直接删除。
  * 完善tooltip提示，前面带* 号的表示点击后会保存
  * 移动控件的位置，改变了部分颜色
  * 改善了上下壁纸切换逻辑，当列表中的壁纸不存在自动时，将其从列表中移除并跳过。
  * 必应壁纸更加精彩，弃用了从必应首页爬取壁纸的方法，改用必应官方的api，可以设置循环切换最近八天壁纸，可以使用壁纸信息作为壁纸名称，可以自定义每次启动是否后台保存必应壁纸。
  * 路径设置-壁纸下载目录增加了选择文件夹按钮。
  * 增加了刷新功能，可以刷新用了很久的图片集合。
  * 将首次更换壁纸和定时更换壁纸区别开来，可选择软件启动时更换一次壁纸。
- 下载地址(64位)
  * [![](https://img.shields.io/badge/Windows%20x64%2021.9.10-蓝奏云-367aff?logo=windows)](https://wws.lanzoui.com/iDmDOtvsugj)
  * 密码：`v910`
***

# 软件问题说明

  * 部分电脑右键无法正常添加“复制路径”功能。
    + 右键以管理员身份运行Speed Box即可正常设置。
  * 不建议把Speed Box可执行文件放在读写需要管理员权限的文件夹下。
  * 其它问题
    + 尝试下载更新或[反馈](https://github.com/yjmthu/Speed-Box/issues)。