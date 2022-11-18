#include <yjson.h>
#include <memory>

#define O YJson::O
#define A YJson::A

std::unique_ptr<YJson> GetMenuJson() {
  return std::unique_ptr<YJson>{ new YJson{ O {
    {u8"设置中心", O {
      {u8"type", u8"Group"},
      {u8"children", O {
        {u8"软件设置", O {
          {u8"type", u8"Group"},
          {u8"children", O {
            {u8"开机自启", O {
              {u8"type", u8"Checkable"},
              {u8"function", u8"AppAutoSatrt"},
              {u8"tip", u8"设置软件是否自动开机启动"},
            }},
            {u8"网卡选择", O {
              {u8"type", u8"Group"},
              {u8"name", u8"NetCardSelect"},
              {u8"children", O { }},
            }},
            {u8"显示托盘", O {
              {u8"type", u8"Checkable"},
              {u8"function", u8"AppShowTrayIcon"},
              {u8"tip", u8"设置是否显示托盘"},
            }},
            {u8"还原位置", O {
              {u8"type", u8"Normal"},
              {u8"function", u8"AppMoveToLeftTop"},
              {u8"tip", u8"把悬浮窗移动到屏幕左上方的位置"},
            }},
            {u8"窗口效果", O {
              {u8"type", u8"ExclusiveGroup"},
              {u8"function", u8"AppFormEffect"},
              {u8"range", A {0, -1}},
              {u8"tip", u8"Windows10/11的五种窗口效果"},
              {u8"children", O {
                {u8"普通效果", O { {u8"type", u8"GroupItem"} }},
                {u8"渐变效果", O { {u8"type", u8"GroupItem"} }},
                {u8"透明效果", O { {u8"type", u8"GroupItem"} }},
                {u8"磨砂效果", O { {u8"type", u8"GroupItem"} }},
                {u8"亚克力效果", O { {u8"type", u8"GroupItem"} }},
              }},
            }},
            {u8"背景颜色", O {
              {u8"type", u8"Normal"},
              {u8"function", u8"AppFormBackground"},
              {u8"tip", u8"修改悬浮窗背景颜色"},
            }},
            {u8"Separator", O {
              {u8"type", u8"Separator"},
            }},
            {u8"皮肤选择", O {
              {u8"type", u8"Group"},
              {u8"name", u8"AppSelectSkin"},
              {u8"tip", u8"悬浮窗皮肤选择"},
              {u8"children", O { }},
            }},
            {u8"皮肤添加", O {
              {u8"type", u8"Normal"},
              {u8"function", u8"AppAddSkin"},
              {u8"tip", u8"选择用户自定义皮肤"},
            }},
            {u8"皮肤删除", O {
              {u8"type", u8"Group"},
              {u8"name", u8"AppRemoveSkin"},
              {u8"children", O { }},
            }},
          }}
        }},
        {u8"壁纸设置", O {
          {u8"type", u8"Group"},
          {u8"children", O {
            {u8"壁纸来源", O {
              {u8"type", u8"ExclusiveGroup"},
              {u8"range", A {0, -1}},
              {u8"function", u8"WallpaperType"},
              {u8"tip", u8"六种内置壁纸源"},
              {u8"children", O {
                {u8"壁纸天堂", O {
                  {u8"type", u8"GroupItem"}, 
                  {u8"tip", u8"wallhaven.cc"},
                }},
                {u8"必应壁纸", O {
                  {u8"type", u8"GroupItem"},
                  {u8"tip", u8"cn.bing.com"},
                }},
                {u8"直链壁纸", O {
                  {u8"type", u8"GroupItem"},
                  {u8"tip", u8"下载并设置链接中的壁纸，例如https://source.unsplash.com/random"},
                }},
                {u8"本地壁纸", O {
                  {u8"type", u8"GroupItem"},
                  {u8"tip", u8"将本地文件夹中的图片作为壁纸来源"},
                }},
                {u8"脚本输出", O {
                  {u8"type", u8"GroupItem"},
                  {u8"tip", u8"运行程序/脚本，将第一行输出（壁纸路径）设为壁纸"},
                }},
                {u8"收藏喜欢", O {
                  {u8"type", u8"GroupItem"},
                  {u8"tip", u8"收藏夹内的壁纸"},
                }},
              }},
            }},
            {u8"自动更换", O {
              {u8"type", u8"Checkable"},
              {u8"function", u8"WallpaperAutoChange"},
              {u8"tip", u8"是否按照时间间隔自动切换壁纸"},
            }},
            {u8"首次更换", O {
              {u8"type", u8"Checkable"},
              {u8"function", u8"WallpaperInitChange"},
              {u8"tip", u8"启动软件时，更换一次壁纸（目前不稳定，可能导致软件崩溃）"},
            }},
            {u8"时间间隔", O {
              {u8"type", u8"Normal"},
              {u8"function", u8"WallpaperTimeInterval"},
              {u8"tip", u8"设置自动切换壁纸的时间间隔"},
            }},
            {u8"存储位置", O {
              {u8"type", u8"Normal"},
              {u8"function", u8"WallpaperDir"},
              {u8"tip", u8"修改当前壁纸的存储位置"},
            }},
            {u8"清理垃圾", O {
              {u8"type", u8"Normal"},
              {u8"function", u8"WallpaperClean"},
              {u8"tip", u8"删除垃圾箱里面的壁纸"},
            }},
            {u8"更多设置", O {
              {u8"type", u8"VarGroup"},
              {u8"name", u8"Wallpaper"},
            }},
          }},
        }},
        {u8"插件设置", O {
          {u8"type", u8"Group"},
          {u8"children", O {
            {u8"极简翻译", O {
              {u8"type", u8"Group"},
              {u8"children", O {
                {u8"翻译热键", O {
                  {u8"type", u8"Group"},
                  {u8"tip", u8"该热键用于显示/隐藏翻译窗口"},
                  {u8"children", O {
                    {u8"热键名称", O {
                      {u8"type", u8"Normal"},
                      {u8"function", u8"ToolTransShortcut"},
                      {u8"tip", u8"修改热键按键"},
                    }},
                    {u8"注册热键", O {
                      {u8"type", u8"Checkable"},
                      {u8"function", u8"ToolTransRegistKey"},
                      {u8"tip", u8"注册该热键后，便可以使用它了；如果不想使用，取消注册热键即可"},
                    }},
                  }},
                }},
                {u8"自动翻译", O {
                  {u8"type", u8"Checkable"},
                  {u8"tip", u8"打开翻译窗口时自动翻译文本内容"},
                  {u8"function", u8"ToolTransAutoTranslate"},
                }},
                {u8"读剪切板", O {
                  {u8"type", u8"Checkable"},
                  {u8"tip", u8"快捷键唤醒窗口时自动读取剪切板内容"},
                  {u8"function", u8"ToolTransReadClipboard" },
                }},
              }},
            }},
            {u8"文字识别", O {
              {u8"type", u8"Group"},
              {u8"children", O {
                {u8"数据位置", O {
                  {u8"type", u8"Normal"},
                  {u8"function", u8"ToolOcrDataPath"},
                  {u8"tip", u8"文字识别训练数据的存放位置，默认在配置目录的tessdata文件夹下"},
                }},
                {u8"截屏热键", O {
                  {u8"type", u8"Group"},
                  {u8"tip", u8"该热键用于截取屏幕并识别其中的文字"},
                  {u8"children", O {
                    {u8"热键名称", O {
                      {u8"type", u8"Normal"},
                      {u8"function", u8"ToolOcrShortcut"},
                      {u8"tip", u8"修改热键按键"},
                    }},
                    {u8"注册热键", O {
                      {u8"type", u8"Checkable"},
                      {u8"function", u8"ToolOcrRegistKey"},
                      {u8"tip", u8"注册该热键后，便可以使用它了；如果不想使用，取消注册热键即可"},
                    }},
                  }},
                }},
              }},
            }},
            {u8"颜色拾取", O {
              {u8"type", u8"Normal"},
              {u8"function", u8"ToolColorPick"},
              {u8"tip", u8"编辑颜色，或者拾取屏幕颜色"},
            }},
          }},
        }},
        {u8"系统设置", O {
          {u8"type", u8"Group"},
          {u8"children", O {
            {u8"右键复制", O {
              {u8"type", u8"Checkable"},
              {u8"function", u8"OtherSetDesktopRightMenu"},
              {u8"tip", u8"在右键添加“复制路径”选项，Windows下默认复制反斜杠“\\”"},
            }},
          }},
        }},
        {u8"其他设置", O {
          {u8"type", u8"Group"},
          {u8"children", O {
            {u8"关于软件", O {
              {u8"type", u8"Normal"},
              {u8"function", u8"AppWbsite"},
              {u8"tip", u8"打开软件网站"},
            }},
          }},
        }},
      }},
    }},
    {u8"翻译功能", O {
      {u8"type", u8"Group"},
      {u8"children", O {
        {u8"翻译窗口", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"ToolTransShowDlg"},
          {u8"tip", u8"打开翻译窗口"},
        }},
        {u8"截图翻译", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"ToolOcrGetScreen"},
          {u8"tip", u8"截取屏幕，并将其转化为可复制的文字"},
        }},
      }},
    }},
    {u8"壁纸切换", O {
      {u8"type", u8"Group"},
      {u8"children", O {
        {u8"上一张图", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"WallpaperPrev"},
          {u8"tip", u8"切换到上一张壁纸"},
        }},
        {u8"下一张图", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"WallpaperNext"},
          {u8"tip", u8"切换到下一张壁纸"},
        }},
        {u8"不看此图", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"WallpaperDislike"},
          {u8"tip", u8"将当前壁纸移动到垃圾箱，并自动切换到下一张壁纸"},
        }},
        {u8"撤销删除", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"WallpaperUndoDislike"},
          {u8"tip", u8"撤销移动壁纸到垃圾箱操作"},
        }},
        {u8"收藏图片", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"WallpaperCollect"},
          {u8"tip", u8"将当前壁纸文件复制到收藏夹"},
        }},
        {u8"取消收藏", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"WallpaperUndoCollect"},
          {u8"tip", u8"将当前壁纸从收藏夹中移除"},
        }},
      }},
    }},
    {u8"打开位置", O {
      {u8"type", u8"Group"},
      {u8"children", O {
        {u8"程序目录", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"AppOpenExeDir"},
          {u8"tip", u8"打开Neobox.exe文件所在的位置"},
        }},
        {u8"配置目录", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"AppOpenConfigDir"},
          {u8"tip", u8"打开配置文件存放位置"},
        }},
        {u8"Separator", O {
          {u8"type", u8"Separator"},
        }},
        {u8"壁纸目录", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"AppOpenWallpaperDir"},
          {u8"tip", u8"打开当前类型壁纸存放位置"},
        }},
        {u8"壁纸位置", O {
          {u8"type", u8"Normal"},
          {u8"function", u8"AppOpenWallpaperLocation"},
          {u8"tip", u8"打开当前壁纸存放位置，并选中改壁纸文件"},
        }},
      }},
    }},
    {u8"开机自启", O {
      {u8"type", u8"Checkable"},
      {u8"function", u8"AppAutoSatrt"},
      {u8"tip", u8"设置软件是否自动开机启动"},
    }},
    {u8"防止息屏", O {
      {u8"type", u8"Checkable"},
      {u8"function", u8"SystemStopSleep"},
      {u8"tip", u8"防止电脑自动进入息屏或休眠状态"},
    }},
    {u8"快速关机", O {
      {u8"type", u8"Normal"},
      {u8"function", u8"SystemShutdown"},
      {u8"tip", u8"关机"},
    }},
    {u8"快捷重启", O {
      {u8"type", u8"Normal"},
      {u8"function", u8"SystemRestart"},
      {u8"tip", u8"重启电脑"},
    }},
    {u8"退出软件", O {
      {u8"type", u8"Normal"},
      {u8"function", u8"AppQuit"},
      {u8"tip", u8"退出软件。如果只是想要重启软件，用鼠标中键点击网速悬浮窗即可"},
    }},
  } } };
}

#undef O
#undef A
