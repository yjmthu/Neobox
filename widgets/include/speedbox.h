#ifndef SPEEDBOX_H
#define SPEEDBOX_H

#include <QObject>

class SpeedBox : public QObject
{
    Q_OBJECT
    //  void dragEnterEvent(QDragEnterEvent* event) override;
    //  void dropEvent(QDropEvent* event) override;
    Q_PROPERTY(int memUseage READ memUseage NOTIFY memUseageChanged)
    Q_PROPERTY(double netUpSpeed READ netUpSpeed NOTIFY netUpSpeedChanged)
    Q_PROPERTY(double netDownSpeed READ netDownSpeed NOTIFY netDownSpeedChanged)

  public:
    SpeedBox(QObject *parent = nullptr);
    ~SpeedBox();
    Q_INVOKABLE void updateInfo();
    Q_INVOKABLE void setRoundRect(int x, int y, int w, int h, int r, bool set);
    int memUseage() const;
    double netUpSpeed() const;
    double netDownSpeed() const;

  private:
    friend class SpeedMenu;
    class NetSpeedHelper *m_NetSpeedHelper;
  signals:
    void memUseageChanged();
    void netUpSpeedChanged();
    void netDownSpeedChanged();
};
#endif // SPEEDBOX_H
