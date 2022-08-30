#ifndef SPEEDMENU_H
#define SPEEDMENU_H

#include <QObject>

class SpeedMenu : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool appAutoStart READ appAutoStart WRITE appSetAutoStart NOTIFY appAutoStartChanged)
    Q_PROPERTY(int wallpaperType READ wallpaperType WRITE wallpaperSetType NOTIFY wallpaperTypeChanged)
    Q_PROPERTY(int wallpaperTimeInterval READ wallpaperTimeInterval WRITE wallpaperSetTimeInterval NOTIFY
                   wallpaperTimeIntervalChanged)
    Q_PROPERTY(QString wallpaperDir READ wallpaperDir WRITE wallpaperSetDir NOTIFY wallpaperDirChanged)
    Q_PROPERTY(bool wallpaperAutoChange READ wallpaperAutoChange WRITE wallpaperSetAutoChange NOTIFY
                   wallpaperAutoChangeChanged)
    Q_PROPERTY(bool wallpaperFirstChange READ wallpaperFirstChange WRITE wallpaperSetFirstChange NOTIFY
                   wallpaperFirstChangeChanged)

  public:
    explicit SpeedMenu();
    ~SpeedMenu();
    Q_INVOKABLE bool toolOcrEnableScreenShotCut(const QString &keys, bool enable);
    Q_INVOKABLE static void appShutdownComputer();
    Q_INVOKABLE static void appRestartComputer();
    Q_INVOKABLE static void appOpenDir(const QString &path);
    Q_INVOKABLE static void appOpenAppDir();
    Q_INVOKABLE static void appOpenCfgDir();
    Q_INVOKABLE void wallpaperGetNext();
    Q_INVOKABLE void wallpaperGetPrev();
    Q_INVOKABLE void wallpaperRemoveCurrent();
    Q_INVOKABLE void wallpaperUndoDelete();
    Q_INVOKABLE void wallpaperClearJunk();
    Q_INVOKABLE void wallpaperSetFavotite();
    Q_INVOKABLE void wallpaperUnSetFavotite();
    Q_INVOKABLE QString wallpaperGetCurJson() const;
    Q_INVOKABLE QString wallpaperGetCurWallpaper() const;
    Q_INVOKABLE void wallpaperSetCurJson(const QString &str);
    Q_INVOKABLE void wallpaperSetDrop(const QString &str);
    Q_INVOKABLE static QObject *toolOcrGrabScreen();

  private:
    class Wallpaper *m_Wallpaper;

  private:
    bool appAutoStart();
    void appSetAutoStart(bool start);
    int wallpaperType() const;
    void wallpaperSetType(int type);
    int wallpaperTimeInterval() const;
    void wallpaperSetTimeInterval(int minute);
    QString wallpaperDir() const;
    void wallpaperSetDir(const QString &dir);
    bool wallpaperAutoChange() const;
    void wallpaperSetAutoChange(bool val);
    bool wallpaperFirstChange() const;
    void wallpaperSetFirstChange(bool val);

  private slots:
  signals:
    void appAutoStartChanged();
    void wallpaperTypeChanged(bool);
    void wallpaperTimeIntervalChanged();
    void wallpaperDirChanged();
    void wallpaperAutoChangeChanged();
    void wallpaperFirstChangeChanged();
};

#endif // SPEEDMENU_H
