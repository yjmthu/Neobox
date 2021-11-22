#ifndef DESKTOPMASK_H
#define DESKTOPMASK_H

#include <QWidget>

class DesktopMask
{
public:
    explicit DesktopMask();
    ~DesktopMask();
private:
    friend class VARBOX;
    QWidget *left, *right;
};

#endif // DESKTOPMASK_H
