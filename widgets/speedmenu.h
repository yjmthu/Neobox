#ifndef SPEEDMENU_H
#define SPEEDMENU_H

#include <QMenu>

class SpeedMenu : public QMenu
{
    Q_OBJECT
protected:
    void showEvent(QShowEvent *event);
public:
    explicit SpeedMenu(QWidget *parent);
    ~SpeedMenu();
private:
    QAction* m_Actions;
    QAction* m_AdditionalAction;
    QAction* m_AutoStartApp;
    void SetupUi();
    void SetupConntects();
    void SetupSettingMenu();
    void SetupImageType(QMenu* parent, QAction* ac);
    void SetAdditionalMenu();
signals:
    void ChangeBoxColor(QColor col);
    void ChangeBoxAlpha(int alpha);
};

#endif // SPEEDMENU_H
