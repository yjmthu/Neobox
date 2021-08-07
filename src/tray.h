#ifndef BUEATRAY_H
#define BUEATRAY_H

#include <Windows.h>
#include <QObject>

class QTimer;

class Tray: public QObject
{
    Q_OBJECT

public:
    explicit Tray();
    ~Tray();
    QTimer* beautifyTask, *centerTask;

private:
    HWND hTray = NULL;                     //系统主任务栏窗口句柄
    HWND hTaskWnd = NULL;                  //系统主任务列表窗口句柄
    HWND hReBarWnd = NULL;                 //系统主任务工具窗口句柄
    HWND hTaskListWnd = NULL;

    void GetShellAllWnd();
    void SetTaskBarPos(HWND, HWND, HWND, HWND, BOOL);


private slots:
    void keepTaskBar();
    void centerTaskBar();
};

#endif
