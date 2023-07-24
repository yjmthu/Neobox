#ifndef DOWNLOADINGDLG_HPP
#define DOWNLOADINGDLG_HPP

#include <QDialog>

class DownloadingDlg: public QDialog
{
  Q_OBJECT

signals:
  void DownloadFinished();
  void Downloading(int, int);
protected:
  void closeEvent(QCloseEvent *) override;
public:
  explicit DownloadingDlg(QWidget* parent = nullptr);
  ~DownloadingDlg();
public:
  bool m_PreventClose;
  void emitFinished();
  void emitProcecs(int process, int total);
private:
  class QProgressBar* m_ProgressBar;
  class QLabel* m_Label;
public slots:
  void SetPercent(int, int);
};

#endif // DOWNLOADINGDLG_HPP
