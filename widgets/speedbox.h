#ifndef SPEEDBOX_H
#define SPEEDBOX_H

#include <QWidget>

class SpeedBox : public QWidget
{
    Q_OBJECT
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEvent* event) override;
#else
    void enterEvent(QEnterEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
public:
    SpeedBox(QWidget *parent = nullptr);
    ~SpeedBox();
    void SetupUi();

private:
    friend class SpeedMenu;
    const int m_Width, m_Height;
    const int m_ScreenWidth, m_ScreenHeight;
    class QPropertyAnimation* m_Animation;
    class NetSpeedHelper *m_NetSpeedHelper;
    QPoint m_LastPos;
    QColor m_BackCol;
    std::array<std::tuple<QColor, QFont, QPoint>, 3> m_Style;
    void ReadPosition();
    void AutoHide();
    void GetStyle();
    void GetBackGroundColor();
private slots:
    void WritePosition();

public slots:
    void OnTimer();
private slots:
    void SetBackGroundColor(QColor col);
    void SetBackGroundAlpha(int alpha);
};
#endif // SPEEDBOX_H
