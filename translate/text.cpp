#include "text.h"

#include "widgets/speedapp.h"
#include "widgets/screenfetch.h"
#include "translater.h"

#include <filesystem>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>

TextDlg::TextDlg()
    : QDialog()
    , m_TextEdit(new QPlainTextEdit(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("识别结果");
    using namespace std::literals;

    auto vbx = new QVBoxLayout(this);
    vbx->addWidget(m_TextEdit);
    QPushButton* btCut = new QPushButton(this);
    btCut->setText("截图");
    connect(btCut, &QPushButton::clicked, this, [this](){
        hide();
        ScreenFetch getscreen;
        getscreen.exec();
        if (getscreen.m_IsGetPicture)
            ParsePicture();
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
    
    ParsePicture();
}

void TextDlg::ParsePicture()
{
    using namespace std::literals;
    char *outText;

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "chi_sim+eng")) {
        QMessageBox::critical(nullptr, "错误", "Could not initialize tesseract.");
        return;
    }

    // Open input image with leptonica library
    if (!std::filesystem::exists("screenfetch.jpg"s)) {
        QMessageBox::critical(nullptr, "错误", "没有找到截图文件!");
        return;
    }

    Pix *image = pixRead("screenfetch.jpg");
    api->SetImage(image);
    // Get OCR result
    outText = api->GetUTF8Text();
    // printf("OCR output:\n%s", outText);
    m_TextEdit->setPlainText(outText);

    // Destroy used object and release memory
    api->End();
    delete api;
    delete [] outText;
    pixDestroy(&image);
}
