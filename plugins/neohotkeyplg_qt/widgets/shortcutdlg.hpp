#ifndef SHORTCUTDLG_H
#define SHORTCUTDLG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class ShortcutDlg : public QDialog
{
  Q_OBJECT

public:
  ShortcutDlg(class YJson& setting);
 ~ShortcutDlg();

public slots:
  void SaveSetting();

private:
  void InitLayout();
  void InitConnect();

signals:
  void finished();

private:
  Ui::Dialog *ui;
  class YJson& m_Setting;
};

#endif // SHORTCUTDLG_H
