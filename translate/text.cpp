#include "text.h"

#include "widgets/speedapp.h"
#include "widgets/screenfetch.h"
#include "translater.h"

#include <filesystem>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <QString>
#include <QThread>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>

TextDlg::TextDlg(void* image)
    : QDialog()
    , m_TextEdit(new QPlainTextEdit(this))
{
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("识别结果");
    using namespace std::literals;

    auto vbx = new QVBoxLayout(this);
    vbx->addWidget(m_TextEdit);
    QPushButton* btCut = new QPushButton(this);
    btCut->setText("截图");
    connect(btCut, &QPushButton::clicked, this, [this](){
        hide();
        QThread::sleep(1);
        ScreenFetch getscreen;
        getscreen.exec();
        void* image = getscreen.m_Picture;
        if (image)
            ParsePicture(image);
        show();
    });
    QPushButton* btCopy = new QPushButton(this);
    btCopy->setText("复制");
    connect(btCopy, &QPushButton::clicked, this, [this](){
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(m_TextEdit->toPlainText());
    });
    QPushButton* btTransWord = new QPushButton(this);
    btTransWord->setText("词译");
    QPushButton* btTransText = new QPushButton(this);
    btTransText->setText("文译");
    connect(btTransText, &QPushButton::clicked, this, [this](){
        m_VarBox->m_Translater->Translate(m_TextEdit->toPlainText());
    });
    auto hbx = new QHBoxLayout;
    hbx->addWidget(btCut);
    hbx->addWidget(btCopy);
    hbx->addWidget(btTransWord);
    hbx->addWidget(btTransText);
    vbx->addLayout(hbx);
    
    ParsePicture(image);
}

inline QString RemoveExtraSpaces(const char* str)
{
    QString result(str);
    QRegExp pattern("([\u4e00-\u9fff]) ([\u4e00-\u9fff])");
    result.replace(pattern, QStringLiteral("\\1\\2"));
    result.replace(pattern, QStringLiteral("\\1\\2"));
    result.replace("\n\n", "\n");
    return result;
}

void TextDlg::ParsePicture(void* image)
{
    using namespace std::literals;
    if (!image) return;
    Pix* pix = reinterpret_cast<Pix*>(image);
    char *outText;

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "chi_sim+eng")) {
        QMessageBox::critical(nullptr, "错误", "Could not initialize tesseract.");
        return;
    }

    // Pix *image = pixRead("screenfetch.jpg");
    api->SetImage(pix);
    // Get OCR result
    outText = api->GetUTF8Text();
    // printf("OCR output:\n%s", outText);
    m_TextEdit->setPlainText(RemoveExtraSpaces(outText));

    // Destroy used object and release memory
    api->End();
    delete api;
    delete [] outText;
    pixDestroy(&pix);
}
