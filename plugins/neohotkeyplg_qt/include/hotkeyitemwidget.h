#ifndef HOTKEYITEMWIDGET_H
#define HOTKEYITEMWIDGET_H

#include <QWidget>

class HotKeyItemWidget: public QWidget
{
  Q_OBJECT
protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
public:
  explicit HotKeyItemWidget(QWidget* parent, const class YJson& data);
  virtual ~HotKeyItemWidget();

public:
  bool WriteJson(YJson& data) const;

public slots:
  void SetExlusive(bool on);

public:
  class QPushButton* m_HotKey;
  class QComboBox* m_TargetPlugin;
  class QCheckBox* m_Enabled;

private:
  void SetShortCut();
  void InitCombox(const std::u8string& plugin);
  static bool isChecked;
};

#endif // HOTKEYITEMWIDGET_H
