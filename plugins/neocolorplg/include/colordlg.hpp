#ifndef COLORDLG_HPP
#define COLORDLG_HPP

#include <widgetbase.hpp>
#include <windows.h>

#include <set>

class YJson;
namespace Ui {
    class ColorForm;
} // namespace Ui

class ColorDlg: public WidgetBase
{
  Q_OBJECT

protected:
  void SaveTopState(bool isTop) override;
  bool eventFilter(QObject *target, QEvent *event) override;
  // bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
public:
  explicit ColorDlg(YJson& settings, QWidget* parent=nullptr);
  virtual ~ColorDlg();
public:
  static ColorDlg* m_Instance;
public slots:
  void PickColor();
private slots:
  void SetCurItem(int index);
private:
  void SetupUi();
  void SetStyleSheet();
  void InitSignals();
  void LoadHistory();
  void SaveHistory();
  void SetCurColor(const QColor& color);
  void RemoveColor(const QColor& color);
  static void SetStyleSheet(QWidget* target, const QColor& color);
private:
  bool InstallHook();
  bool UninstallHook();
public:
  // QPoint WinPoint2QPoint(int x, int y) const;
  static void DoMouseMove(LPARAM lParam);
  static void DoMouseWheel(LPARAM lParam);
  void QuitHook(bool succeed);
  void AddColor(const QColor& color);
  void ScalTarget(int value);
private:
  YJson& m_Settings;
  YJson& m_ColorsArray;
  QWidget* m_CenterWidget;
  // QColor m_CurrentColor;
  Ui::ColorForm* const ui;
  std::set<QString> m_Colors;
  bool m_ColorsChanged;
  QString m_StyleWide, m_StyleNarrow;
public:
  static HHOOK m_Hoock[2];
  class SmallForm* m_SmallForm;
};

#endif // COLORDLG_HPP
