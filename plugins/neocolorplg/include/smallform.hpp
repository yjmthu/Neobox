#ifndef SMALLFORM_HPP
#define SMALLFORM_HPP

#include <QWidget>
#include <QPainterPath>
#include <QTextOption>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Ui {
  class SmallForm;
}

class SmallForm: public QWidget
{
  // Q_OBJECT

protected:
  void showEvent(QShowEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  // void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
public:
  explicit SmallForm(class ColorConfig& settings);
  virtual ~SmallForm();
public:
  static void PickColor(ColorConfig& settings);
private:
  void TransformPoint(QPoint& point);
  void SetColor(const QColor& color);
  void GetScreenColor(int x, int y);
  void AutoPosition(const QPoint& point);
  static bool InstallHook();
  static bool UninstallHook();
  void QuitHook(bool succeed);
private:
  static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK LowLevelKeyProc(int nCode, WPARAM wParam, LPARAM lParam);
public slots:
  void DoMouseWheel(LPARAM lParam);
  QColor m_Color;
  static SmallForm* m_Instance;
  static HHOOK m_Hoock[2];
private:
  class ColorConfig& m_Settings;
  QScreen* m_Screen;
  short m_ScaleTimes;
  class SquareForm* m_SquareForm;
  QPixmap m_BackPixMap;
  const QTextOption m_TextOption;
  QPainterPath m_ColorPath;
  const QFont m_TextFont;
};

#endif // SMALLFORM_HPP