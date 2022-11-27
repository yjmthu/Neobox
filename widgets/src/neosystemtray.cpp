#include <neosystemtray.h>
#include <neoapp.h>

#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QApplication>

// extern GlbObject* glb;

NeoSystemTray::NeoSystemTray()
{
  setIcon(QIcon(QStringLiteral(":/icons/neobox.ico")));
  setToolTip("Neobox");
  InitDirs();
  InitConnect();
}

NeoSystemTray::~NeoSystemTray()
{
  //
}

void NeoSystemTray::InitDirs()
{
  QDir dir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
  constexpr char relPath[] = ".config/Neobox";
  if (dir.exists(relPath) || dir.mkpath(relPath)) {
    dir.cd(relPath);
    QDir::setCurrent(dir.absolutePath());
  } else {
#if 0
    glb->glbWriteSharedFlag(1);
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList {});
#endif
    qApp->quit();
    return;
  }
}

void NeoSystemTray::InitConnect()
{
  connect(this, &QSystemTrayIcon::activated,
    [](QSystemTrayIcon::ActivationReason reason){
      switch (reason)
      {
        // case QSystemTrayIcon::DoubleClick:
        //   if (VarBox::GetSpeedBox()->isVisible()) {
        //     VarBox::GetSpeedBox()->hide();
        //     VarBox::GetSettings(u8"FormGlobal")[u8"ShowForm"] = false;
        //     VarBox::ShowMsg("隐藏悬浮窗成功！");
        //   } else {
        //     VarBox::GetSpeedBox()->show();
        //     VarBox::GetSettings(u8"FormGlobal")[u8"ShowForm"] = true;
        //     VarBox::ShowMsg("显示悬浮窗成功！");
        //   }
        //   VarBox::WriteSettings();
        //   break;
        // case QSystemTrayIcon::Trigger:
        //   VarBox::GetSpeedBox()->raise();
        //   break;
        default:
          break;
      }
    });
}
