#ifndef SCREENFETCH_H
#define SCREENFETCH_H

#include <QDialog>

class ScreenFetch: public QDialog
{
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    // void mouseReleaseEvent(QMouseEvent* event) override;
    // void mouseDoubleClickEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
public:
    explicit ScreenFetch();
    ~ScreenFetch();
    bool m_IsGetPicture;
private:
    QRect m_TextRect;
    bool m_IsTrackingMouse;
};

#endif // SCREENFETCH_H
