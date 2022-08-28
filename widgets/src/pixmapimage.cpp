#include <pixmapimage.h>

#include <QGuiApplication>
#include <QQmlEngine>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

extern void ShowMessage(const std::u8string &title, const std::u8string &text, int type = 0);

static Pix *QImage2Pix(const QImage &&image)
{
    Pix *pix;
    int width = image.width();
    int height = image.height();
    int depth = image.depth();
    pix = pixCreate(width, height, depth);
    if (image.isNull())
    {
        // std::cout << "image is null\n";
        return nullptr;
    }
    if (image.colorCount())
    {
        QVector<QRgb> table = image.colorTable();

        PIXCMAP *map = pixcmapCreate(8);

        int n = table.size();
        for (int i = 0; i < n; i++)
        {
            pixcmapAddColor(map, qRed(table[i]), qGreen(table[i]), qBlue(table[i]));
        }
        pixSetColormap(pix, map);
    }
    int bytePerLine = image.bytesPerLine();
    l_uint32 *start = pixGetData(pix);
    l_int32 wpld = pixGetWpl(pix);
    if (image.format() == QImage::Format_Mono || image.format() == QImage::Format_Indexed8 ||
        image.format() == QImage::Format_RGB888)
    {
        for (int i = 0; i < height; i++)
        {
            const uchar *lines = image.scanLine(i);
            uchar *lined = (uchar *)(start + wpld * i);
            memcpy(lined, lines, static_cast<size_t>(bytePerLine));
        }
    }
    else if (image.format() == QImage::Format_RGB32 || image.format() == QImage::Format_ARGB32)
    {
        // std::cout << "QImage::Format_RGB32\n";
        for (int i = 0; i < image.height(); i++)
        {
            const QRgb *lines = (const QRgb *)image.scanLine(i);
            l_uint32 *lined = start + wpld * i;
            for (int j = 0; j < width; j++)
            {
                uchar rval = qRed(lines[j]);
                uchar gval = qGreen(lines[j]);
                uchar bval = qBlue(lines[j]);
                l_uint32 pixel;
                composeRGBPixel(rval, gval, bval, &pixel);
                lined[j] = pixel;
            }
        }
    }
    return pix;
}

inline QString RemoveExtraSpaces(const char *str)
{
    QString result(str);
    QRegExp pattern("([\u4e00-\u9fff]) ([\u4e00-\u9fff])");
    result.replace(pattern, QStringLiteral("\\1\\2"));
    result.replace(pattern, QStringLiteral("\\1\\2"));
    result.replace("\n\n", "\n");
    return result;
}

PixmapContainer::PixmapContainer(QObject *parent) : QObject(parent)
{
}

PixmapImage::PixmapImage(QQuickItem *parent) : QQuickPaintedItem(parent)
{
}

void PixmapImage::setImage(QObject *pixmap)
{
    PixmapContainer *pc = qobject_cast<PixmapContainer *>(pixmap);
    Q_ASSERT(pc);
    m_Image = std::move(pc->m_Image);
    update();
    pc->deleteLater();
}

QString PixmapImage::getText(const QString &dataPath, int x, int y, int w, int h) const
{
    Pix *pix = QImage2Pix(m_Image.copy(x, y, w, h));

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    const auto &array = dataPath.toLocal8Bit();
#ifdef _WIN32
    char *file = new char[array.size() - 7];
    std::copy(array.begin() + 8, array.end(), file);
    file[array.size() - 8] = '\0';
#else
    char *file = new char[array.size() - 6];
    std::copy(array.begin() + 7, array.end(), file);
    file[array.size() - 7] = '\0';
#endif
    if (api->Init(file, "chi_sim+eng"))
    {
        ShowMessage(u8"错误", u8"Could not initialize tesseract.");
        return QString();
    }

    // Pix *image = pixRead("screenfetch.jpg");
    api->SetImage(pix);
    // Get OCR result
    char *outText = api->GetUTF8Text();
    QString text = RemoveExtraSpaces(outText);

    // Destroy used object and release memory
    api->End();
    delete api;
    delete[] outText;
    pixDestroy(&pix);

    return text;
}

void PixmapImage::paint(QPainter *painter)
{
    painter->drawImage(0, 0, m_Image);
}
