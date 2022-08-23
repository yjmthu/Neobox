#ifndef PIXMAPIMAGE_H
#define PIXMAPIMAGE_H

#include <QQuickPaintedItem>
#include <QPainter>

// see more at https://forum.qt.io/topic/69209/how-to-display-a-qpixmap-qimage-in-qml/3

class PixmapContainer: public QObject
{
  Q_OBJECT
public:
    explicit PixmapContainer(QObject* parent = nullptr);
    QImage m_Image;
};

class PixmapImage: public QQuickPaintedItem
{
  Q_OBJECT
public:
  PixmapImage(QQuickItem *parent = Q_NULLPTR);
  Q_INVOKABLE void setImage(QObject *pixmap);
  Q_INVOKABLE QString getText(int x, int y, int w, int h) const;
protected:
  virtual void paint(QPainter *painter) override;
private:
  QImage m_Image;
};

#endif
