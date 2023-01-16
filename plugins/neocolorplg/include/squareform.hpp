#ifndef SQUAREFORM_HPP
#define SQUAREFORM_HPP

#include <QWidget>

class SquareForm: public QWidget
{
  // Q_OBJECT
protected:
  void paintEvent(QPaintEvent *event) override;
  void showEvent(QShowEvent *event) override;
  // void mouseMoveEvent(QMouseEvent *event) override;
public:
  explicit SquareForm(const QPixmap& pix, const QPoint& pos, QWidget* parent = nullptr);
  virtual ~SquareForm();
public:
  void SetScaleSize(short times);
  void MouseMove(const QPoint& global);
private:
  void DrawBorder(QPainter& painter) const;
  void DrawPicture();
private:
  static constexpr int m_BorderWidth = 4;
  QPoint m_Center;
  // QPoint m_MouseGrid;
  const QSize m_BaseSize;
  const short m_BaseScal;
  // QPixmap m_PixMap;
  QImage m_Image;
  QPixmap m_PixMap;
  // std::map<short, QPixmap> m_PixMaps;
  short m_ScalSize;
private:
  class ColorBigger* m_ColorBigger;
};

#endif // SQUAREFORM_HPP