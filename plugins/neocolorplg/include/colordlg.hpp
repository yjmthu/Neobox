#ifndef COLORDLG_HPP
#define COLORDLG_HPP

#include <widgetbase.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

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
  explicit ColorDlg(YJson& settings);
  virtual ~ColorDlg();
public:
  static ColorDlg* m_Instance;
  void AddColor(const QColor& color);
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
  // void ScalTarget(int value);
private:
  YJson& m_Settings;
  YJson& m_ColorsArray;
  QWidget* m_CenterWidget;
  // QColor m_CurrentColor;
  Ui::ColorForm* const ui;
  std::set<QString> m_Colors;
  bool m_ColorsChanged;
  QString m_StyleWide, m_StyleNarrow;
};

#endif // COLORDLG_HPP
