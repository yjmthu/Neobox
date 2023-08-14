#ifndef DOWNLOADINGDLG_HPP
#define DOWNLOADINGDLG_HPP

#include <QDialog>

class DownloadingDlg: public QDialog
{
  Q_OBJECT

signals:
  void DownloadFinished();
  void Downloading(size_t, size_t);
  void Terminate();
protected:
  void closeEvent(QCloseEvent *) override;
public:
  explicit DownloadingDlg(QWidget* parent = nullptr);
  ~DownloadingDlg();
public:
  bool m_PreventClose;
  void emitFinished();
  void emitProcess(size_t process, size_t total);
private:
  class QProgressBar* m_ProgressBar;
  class QLabel* m_Label;
public slots:
  void SetPercent(size_t, size_t);
};

#endif // DOWNLOADINGDLG_HPP
