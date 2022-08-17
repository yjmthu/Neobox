#ifndef APPCODE_HPP
#define APPCODE_HPP

enum class ExitCode
{
    RETCODE_NORMAL = 0,
    RETCODE_ERROR_EXIT = 1071, // 异常退出常数
    RETCODE_UPDATE = 1072,     // 更新常数，更新软件
    RETCODE_RESTART = 1073     // 重启常数，双击时界面会返回这个常数实现整个程序重新启动。
};

enum class MsgCode
{
    FullScreen = 2731,
    MSG_APPBAR_MSGID = 2731
};

#endif // APPCODE_HPP
