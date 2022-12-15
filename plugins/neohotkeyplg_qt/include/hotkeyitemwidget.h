#ifndef HOTKEYITEMWIDGET_H
#define HOTKEYITEMWIDGET_H

#include <QWidget>

class HotKeyItemWidget: public QWidget
{
  Q_OBJECT
protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
public:
  explicit HotKeyItemWidget(QWidget* parent, const std::u8string& name, const class YJson& data);
  ~HotKeyItemWidget();

public:
  bool WriteJson(YJson& data) const;

public slots:
  void SetExlusive(bool on);

public:
  class QLineEdit* m_Description;
  class QPushButton* m_HotKey;
  class QCheckBox* m_Enabled;

private:
  void SetShortCut();
  static bool isChecked;
};

#endif // HOTKEYITEMWIDGET_H
