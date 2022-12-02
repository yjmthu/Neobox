#ifndef NEOAPP_H
#define NEOAPP_H

#include <string>

struct GlbObject {
private:
  class NeoSystemTray* glbTray;
  class NeoMenu* glbMenu;
  // class QSharedMemory*const glbSharedMemory;
  class NeoMsgDlg* glbMsgDlg;
public:
  explicit GlbObject();
  ~GlbObject();
#if 0
  bool glbCreateSharedMemory();
  void glbDetachSharedMemory();
  void glbWriteSharedFlag(int flag);
  int glbReadSharedFlag();
#endif
  class QMenu* glbGetMenu();
  void glbShowMsg(class QString text);
  void glbShowMsgbox(const std::u8string& title, const std::u8string& text, int type = 0);
};

extern GlbObject* glb;

#endif // NEOAPP_H
