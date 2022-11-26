#ifndef NEOAPP_H
#define NEOAPP_H

#include <string>

bool glbCreateSharedMemory();
void glbDetachSharedMemory();
void glbWriteSharedFlag(int flag);
int glbReadSharedFlag();
class QMenu* gblbGetMenu();
void glbShowMsg(class QString text);
void glbShowMsgbox(const std::u8string& title,
                 const std::u8string& text,
                 int type = 0);

#endif // NEOAPP_H