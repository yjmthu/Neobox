#ifndef ITEMHOTKEY_H
#define ITEMHOTKEY_H

#include <QWidget>

class ItemHotKey: public QWidget
{
  Q_OBJECT
protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
public:
  explicit ItemHotKey(QWidget* parent, const class YJson& data);
  virtual ~ItemHotKey();

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

#endif // ITEMHOTKEY_H
