#include <config.h>

#include <atomic>
#include <filesystem>
#include <format>


#include <Shlobj.h>
#include <Windows.h>
#include <tlhelp32.h>


#include <QApplication>
#include <QDialog>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QSharedMemory>
#include <QStandardPaths>


namespace fs = std::filesystem;
using namespace std::literals;


void EraseRegister() {
  {
    QSettings settings("HKEY_LOCAL_"
                     "MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Un"
                     "install\\Neobox",
                     QSettings::NativeFormat);
    settings.remove("");
  }
  {
    QSettings settings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)", QSettings::NativeFormat);
    if (settings.contains("Neobox")) {
      settings.remove("Neobox");
    }
  }
}

bool ExitAppInstance(fs::path appFile) {
  appFile.make_preferred();
  QSharedMemory memory("__Neobox__");
  if (memory.attach()) {
    std::atomic_bool quit = false;
    bool result = false;
    auto const dialog = new QDialog;
    dialog->setWindowTitle("Neobox卸载程序");
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(dialog, &QDialog::destroyed, [&quit]() { quit = true; });
    std::thread([&memory, &quit, appFile, dialog, &result]() {
      int code = -1;
      memory.lock();
      *reinterpret_cast<int *>(memory.data()) = code;
      memory.unlock();

      while (code != 1 && !quit) {
        std::this_thread::sleep_for(500ms);
        memory.lock();
        code = *reinterpret_cast<const int *>(memory.constData());
        memory.unlock();
      }

      while (!quit && fs::exists(appFile)) {
        try {
          fs::remove(appFile);
        } catch (fs::filesystem_error error) {
          continue;
        }
      }

      result = !quit;

      QMetaObject::invokeMethod(dialog, &QDialog::close);
    }).detach();
    dialog->exec();
    memory.detach();
    return result;
  }
  return true;
}

void RemoveLinks() {
  auto strDesktopLink = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
  fs::path pathDesktopLink = strDesktopLink.toStdWString();
  pathDesktopLink /= "Neobox.lnk";
  if (fs::exists(pathDesktopLink)) {
    fs::remove(pathDesktopLink);
  }
  auto strStartMenu = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
  fs::path pathStartMenu = strStartMenu.toStdWString();
  pathStartMenu /= "Neobox";
  if (fs::exists(pathStartMenu)) {
    fs::remove_all(pathStartMenu);
  }
}

void RemoveFiles() {
  fs::path dir = QApplication::applicationDirPath().toStdWString();
  dir.make_preferred();

  auto const dwSize = GetEnvironmentVariableW(L"COMSPEC", nullptr, 0);
  std::wstring strEnvCmd(dwSize, 0);
  GetEnvironmentVariableW(L"COMSPEC", strEnvCmd.data(), dwSize);
  fs::path pathCmd = strEnvCmd.substr(0, dwSize - 1);

  fs::current_path(pathCmd.parent_path());
  std::wstring exeDirectory = dir.wstring();
  exeDirectory.push_back(L'\0');
  std::wstring arguments = std::format(L"/c rmdir /S /Q \"{}\"", dir.wstring());

  // 设置进程优先级-实时，使其抢先于操作系统组件之前运行
  SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
  // 设置线程优先级-实时
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
  // 通知资源管理器，删除工作进程文件
  SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, exeDirectory.data(), NULL);
  // 隐藏启动控制台程序，执行删除文件指令
  ShellExecuteW(NULL, L"open", strEnvCmd.data(), arguments.c_str(), NULL,
                SW_HIDE);
  // 退出当前进程
  ExitProcess(ERROR_SUCCESS);
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  if (QMessageBox::question(nullptr, "提示", "您真的要完全卸载Neobox吗？") ==
      QMessageBox::No)
    return 0;
  fs::path pathApp = app.applicationDirPath().toStdWString();
  pathApp /= "neobox.exe";
  if (ExitAppInstance(pathApp)) {
    RemoveLinks();
    EraseRegister();
    RemoveFiles();
  }
  return 0;
}
