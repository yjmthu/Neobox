#include "downloadingdlg.hpp"

#include <format>

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QCloseEvent>

DownloadingDlg::DownloadingDlg(QWidget* parent)
  : QDialog(parent)
  , m_PreventClose(false)
  , m_ProgressBar(new QProgressBar(this))
  , m_Label(new QLabel(this))
{
  setWindowTitle("正在下载，请稍等");
  auto const layout = new QVBoxLayout(this);
  layout->addWidget(m_ProgressBar);
  layout->addWidget(m_Label);
  m_ProgressBar->setFixedHeight(6);
  m_ProgressBar->setRange(0, 0);

  connect(this, &DownloadingDlg::DownloadFinished, this, &QDialog::close);
  connect(this, &DownloadingDlg::Downloading, this, &DownloadingDlg::SetPercent);
}

DownloadingDlg::~DownloadingDlg()
{
}

void DownloadingDlg::SetPercent(size_t count, size_t size)
{
  std::wstring const text = std::format(L"正在下载：{}/{}", count, size);
  m_ProgressBar->setRange(0, size);
  if (size) m_ProgressBar->setValue(count + 1);
  m_Label->setText(QString::fromStdWString(text));
}

void DownloadingDlg::closeEvent(QCloseEvent * event)
{
  if (m_PreventClose) {
    event->ignore();
  } else {
    emit Terminate();
    event->accept();
  }
}

void DownloadingDlg::emitFinished() {
  emit DownloadFinished();
}

void DownloadingDlg::emitProcess(size_t process, size_t total) {
  emit Downloading(process, total);
}
