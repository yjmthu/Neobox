#include "downloadingdlg.hpp"

#include <format>

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

DownloadingDlg::DownloadingDlg(QWidget* parent)
  : QDialog(parent)
  , m_ProgressBar(new QProgressBar(this))
  , m_Label(new QLabel(this))
{
  setWindowTitle("正在下载，请稍等");
  auto const layout = new QVBoxLayout(this);
  layout->addWidget(m_ProgressBar);
  layout->addWidget(m_Label);
  m_ProgressBar->setRange(0, 0);
}

DownloadingDlg::~DownloadingDlg()
{
}

void DownloadingDlg::SetPercent(int count, int size)
{
  std::wstring const text = std::format(L"正在下载：{}/{}", count, size);
  m_ProgressBar->setRange(0, size);
  m_ProgressBar->setValue(count + 1);
  m_Label->setText(QString::fromStdWString(text));
}

