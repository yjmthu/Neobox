#ifndef SQUAREFORM_HPP
#define SQUAREFORM_HPP

#include <colorback.hpp>

class SquareForm: public ColorBack
{
  Q_OBJECT

protected:
  void paintEvent(QPaintEvent *event) override;
  void showEvent(QShowEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
public:
  explicit SquareForm(const QPixmap& pix, const QPoint& pos, QWidget* parent = nullptr);
  virtual ~SquareForm();
  void SetScaleSize(short times);
  void MouseMove(const QPoint& global);
private:
  void DrawPicture();
private:
  static constexpr int m_BorderWidth = 4;
  QPoint m_Center;
  const QSize m_BaseSize;
  const short m_BaseScal;
  QPixmap m_Pixmap;
  short m_ScalSize;
  class ColorBigger* m_ColorBigger;
};

#endif // SQUAREFORM_HPP