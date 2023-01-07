#ifndef GLBOBJECT_H
#define GLBOBJECT_H

#include <string>

struct GlbObject {
private:
  class NeoSystemTray* m_Tray;
  class NeoMenu* m_Menu;
  class QSharedMemory*const m_SharedMemory;
  class NeoMsgDlg* m_MsgDlg;
public:
  explicit GlbObject();
  ~GlbObject();
public:
  int Exec();
  void Quit();
  void Restart();
public:
  void glbWriteSharedFlag(int flag);
  int glbReadSharedFlag();
  class QMenu* glbGetMenu();
  class NeoSystemTray* glbGetSystemTray();
  void glbShowMsg(class QString text);
  void glbShowMsgbox(const std::u8string& title, const std::u8string& text, int type = 0);
private:
  bool glbCreateSharedMemory();
  void glbDetachSharedMemory();
};

extern GlbObject *glb;

#endif // GLBOBJECT_H
