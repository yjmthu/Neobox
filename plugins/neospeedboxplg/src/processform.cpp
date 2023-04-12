#include <processform.h>
#include <processhelper.h>
#include <speedbox.h>

#include <QScreen>
#include <QGuiApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include <format>

using namespace std::literals;

ProcessForm::ProcessForm(SpeedBox* box)
  : WidgetBase(nullptr)
  , m_ProcessCount(7)
  , m_SpeedBox(*box)
  , m_CenterWidget(new QWidget(this))
  , m_Labels(new QLabel[3 * m_ProcessCount])
  , m_ProcessHelper(new ProcessHelper)
{
  SetupUi();
}

ProcessForm::~ProcessForm()
{
  delete [] m_Labels;
  delete m_ProcessHelper;
}

void ProcessForm::SetupUi()
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
  setAttribute(Qt::WA_TranslucentBackground, true);
  auto const layout = new QVBoxLayout(this);
  layout->setContentsMargins(11, 11, 11, 11);
  layout->addWidget(m_CenterWidget);
  m_CenterWidget->setStyleSheet("QWidget{background-color:white; color:black; border-radius: 3px;}");
  SetShadowAround(m_CenterWidget);

  auto const begin = m_Labels, end = m_Labels + m_ProcessCount*3;
  for (auto ptr=begin; ptr != end; ++ptr) {
    ptr->setParent(this);
  }
  auto const mainLayout = new QVBoxLayout(m_CenterWidget);

  {
  auto const titleLayout = new QHBoxLayout;
  QLabel* label = nullptr;
  label = new QLabel("名称", m_CenterWidget);
  label->setFixedWidth(130);
  titleLayout->addWidget(label);
  label = new QLabel("PID", m_CenterWidget);
  label->setFixedWidth(50);
  titleLayout->addWidget(label);
  label = new QLabel("内存", m_CenterWidget);
  label->setFixedWidth(60);
  titleLayout->addWidget(label);
  mainLayout->addLayout(titleLayout);
  }

  for (auto ptr = begin; ptr != end; ptr += 3) {
    auto const layout = new QHBoxLayout;
    ptr[0].setFixedWidth(130);
    ptr[1].setFixedWidth(50);
    ptr[2].setFixedWidth(60);
    layout->addWidget(ptr);
    layout->addWidget(ptr+1);
    layout->addWidget(ptr+2);
    mainLayout->addLayout(layout);
  }
}

void ProcessForm::UpdateList()
{
  m_ProcessHelper->GetProcessInfo();
  auto const begin = m_Labels, end = m_Labels + m_ProcessCount*3;
  auto iter = m_ProcessHelper->m_ProcessInfo.cbegin();
  for (auto ptr = begin; ptr != end; ptr += 3, ++iter) {
    if (iter == m_ProcessHelper->m_ProcessInfo.end()) {
      ptr[0].clear();
      ptr[1].clear();
      ptr[2].clear();
    } else {
      auto& info = **iter;
#ifdef _WIN32
      ptr[0].setText(QString::fromStdWString(info.exeFile));
      ptr[0].setToolTip(QString::fromStdWString(info.commandLine));
#else
      ptr[0].setText(QString::fromStdString(info.exeFile));
      ptr[0].setToolTip(QString::fromStdString(info.commandLine));
#endif
      ptr[1].setText(QString::number(info.processID));
      ptr[2].setText(FormatBytes(info.workingSetSize));
    }
  }
}

QString ProcessForm::FormatBytes(size_t bytes)
{
  static const auto units = "BKMGTP"sv;
  auto iter = units.cbegin();
  uint64_t size = 1;
  bytes &= ((1ull << 60) - 1);
  // 2^11 >> 10 = 2, 2^10 >> 10 = 1, 2^10 | 2^11-1 >> 10 = 1
  for (auto const kb = (bytes >> 10); size <= kb; ++iter) { size <<= 10; }

  return QString::fromStdString(std::format("{:.1f}{}", static_cast<float>(bytes) / size, *iter));
}

void ProcessForm::showEvent(QShowEvent*) {
  const auto qFormRect = m_SpeedBox.frameGeometry();

  const auto size = QGuiApplication::primaryScreen()->size();
  const auto mSize = frameSize();
  int x, y;
  y = (qFormRect.top() - mSize.height() < 0)
          ? qFormRect.bottom() : qFormRect.top() - mSize.height();
  if (((mSize.width() + qFormRect.width()) >> 1) + qFormRect.left() >
      size.width()) {
    x = size.width() - mSize.width();
  } else if ((qFormRect.left() << 1) + qFormRect.width() < mSize.width()) {
    x = 0;
  } else {
    x = qFormRect.right() - ((qFormRect.width() + mSize.width()) >> 1);
  }

  move(x, y);

  activateWindow();
}
