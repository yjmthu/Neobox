#include <ocrdialog.h>
#include <pluginmgr.h>

#include <QFile>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QStandardPaths>

OcrDialog::OcrDialog(NeoOcr& ocr, OcrDialog*& self)
  : WidgetBase(nullptr)
  , m_Self(self)
  , m_OcrEngine(ocr)
  , m_Image(nullptr)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setAttribute(Qt::WA_TranslucentBackground);
  setFixedSize(800, 500);
  InitBaseLayout();
  AddCloseButton();
  AddMinButton();
}

OcrDialog::~OcrDialog()
{
  m_Self = nullptr;
  delete m_Image;
}

void OcrDialog::InitBaseLayout()
{
  auto const backWidget = new QWidget(this);
  backWidget->setGeometry(10, 10, width() - 20, height() - 20);
  backWidget->setStyleSheet(
    "background-color: white;"
    "border-radius: 6px;"
  );
  auto const title = new QLabel("图片扫描", backWidget);
  title->move(10, 10);
  m_ImageLabel = new QLabel(backWidget);
  m_ImageLabel->setGeometry(10, 30, 430, 435);
  m_ImageLabel->setStyleSheet("background-color: gray;");

  auto const panel = new QWidget(backWidget);
  panel->setGeometry(450, 30, 320, 435);
  panel->setStyleSheet("background-color: gray;");

  auto const controls = new QWidget(panel);
  controls->setGeometry(10, 10, 300, 100);
  controls->setStyleSheet("background-color:white;");
  auto const plainText = new QPlainTextEdit(panel);

  auto const btnOpenImg = new QPushButton("打开图片", controls);
  btnOpenImg->move(10, 10);
  connect(btnOpenImg, &QPushButton::clicked, this, [this](){
    auto path = OpenImage();
    SetImage(path);
  });
  auto const btnScan = new QPushButton("识别文字", controls);
  btnScan->move(60, 10);
  connect(btnScan, &QPushButton::clicked, this, [this, plainText](){
    if (!m_Image) {
      mgr->ShowMsg("请先打开图片！");
      return;
    }
    auto result = m_OcrEngine.GetTextEx(*m_Image);
    for (const auto &i: result) {
      plainText->appendPlainText(QString::fromUtf8(i.text.data(), i.text.size()));
    }
  });

  plainText->setGeometry(10, 125, 300, 300);
  plainText->setStyleSheet(
    "color:white;"
    "background-color:black;"
  );
}

void OcrDialog::SetImage(QString path)
{
  if (path.isEmpty()) {
    mgr->ShowMsg("图片路径为空！");
    return;
  }
  if (!QFile::exists(path)) {
    mgr->ShowMsg("图片文件不存在！");
    return;
  }
  if (!m_Image) {
    m_Image = new QImage;
  }
  if (m_Image->load(path)) {
    m_ImageLabel->setPixmap(QPixmap::fromImage(*m_Image));
  } else {
    mgr->ShowMsg("加载图片文件失败！");
  }
}

QString OcrDialog::OpenImage()
{
  QString openFile = QFileDialog::getOpenFileName(
    this, "请选择一个图片文件",
    QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
    "Image Files(*.jpg *.png *.bmp *.pgm *.pbm);;All(*.*)"
  );
  return openFile;
}
