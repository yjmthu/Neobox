#include <QButtonGroup>
#include <QJsonDocument>
#include <QClipboard>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextCodec>

#include "form.h"
#include "translater.h"
#include "md5.h"

//#define appid "20210503000812254"
//#define password QString("Q_2PPxmCr66r6B2hi0ts")
#define API "http://api.fanyi.baidu.com/api/trans/vip/translate?"

Translater::Translater(Form* v) :
	ui(new Ui::Translater),
	form(v)
{
	ui->setupUi(this);
    group = new QButtonGroup(this);                                    //创建按钮组合
	group->addButton(ui->ENTOZH);
	group->addButton(ui->ZHTOEN);
	group->setExclusive(true);                                   //中英互斥
	setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
	ui->TextFrom->installEventFilter(this);
	check_is_auto_hide();
	setMinimumSize(TRAN_WIDTH, TRAN_HEIGHT);
	setMaximumSize(TRAN_WIDTH, TRAN_HEIGHT);
    shortcut_show = new QxtGlobalShortcut(this);                 //显示界面
	shortcut_hide = new QxtGlobalShortcut(this);                 //隐藏界面
	shortcut_show->setShortcut(QKeySequence("Shift+Z"));
	shortcut_hide->setShortcut(QKeySequence("Shift+A"));
	connect(shortcut_show, &QxtGlobalShortcut::activated, this,  //第三个参数this可以不加，但是如果加上去会让Qt能够在对象被delete后自动取消信号和槽关联
		[this](void)
		{
            if (this->isHidden())
				this->showself();
			QClipboard* cl = QApplication::clipboard();              //读取剪切板
			QString content = cl->text();
			this->ui->TextFrom->setPlainText(content);
			this->getReply(content);
			this->showself();
		});
	connect(shortcut_hide, &QxtGlobalShortcut::activated, this,
		[this](void)
		{
            if (!isHidden())
                close();
		});
}

Translater::~Translater()
{
    D("析构极简翻译中。");
    A(delete shortcut_hide;)
    A(delete shortcut_show;)
    A(delete ui;)
    D("极简翻译析构成功。");
}

void Translater::check_is_auto_hide() const                    //检查是否应该自动隐藏
{
    QSettings IniRead(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniRead.beginGroup("Translate");
    ui->isFix->setChecked(!IniRead.value("AutoHide").toBool());
    IniRead.endGroup();
}

void Translater::showself()
{
	int x, y, px = form->pos().x(), py = form->pos().y();
	if (py < TRAN_HEIGHT + ADAPT_HEIGHT)
		y = py + FORM_HEIGHT + ADAPT_HEIGHT;
	else
		y = py - TRAN_HEIGHT;
	if (px + FORM_WIDTH / 2 + TRAN_WIDTH / 2 < VarBox.SCREEN_WIDTH)
	{
		if (px + FORM_WIDTH / 2 > TRAN_WIDTH / 2)
			x = px + FORM_WIDTH / 2 - TRAN_WIDTH / 2;
		else
			x = 0;
	}
	else
		x = VarBox.SCREEN_WIDTH - TRAN_WIDTH;
	setGeometry(x, y, TRAN_WIDTH, TRAN_HEIGHT);               //移动translater到悬浮窗正上方或正下方
	show();
	ui->TextFrom->setFocus();
    QTimer::singleShot(10000, this, [this](void){ if ( !isHidden() && !ui->isFix->isChecked()) close();});
}

bool Translater::eventFilter(QObject* target, QEvent* event)
{
	if (target == ui->TextFrom)
	{
		if (int(event->type()) == 6)               //QEvent::KeyPress
		{
			QKeyEvent* k = static_cast<QKeyEvent*>(event);

			if (k->key() == Qt::Key_Return)       //回车键
			{
				getReply(ui->TextFrom->toPlainText());
				return true;
			}
			else if (k->key() == Qt::Key_Delete)  //删除键
			{
				ui->TextFrom->clear();
				ui->TextTo->clear();
				return true;
			}
		}
	}
	return QWidget::eventFilter(target, event);
}

void Translater::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Escape)                                        //按下Esc键关闭
        close();
	else if (event->key() == Qt::Key_Delete)                                  //按下delete键清空要翻译的文本
	{
		ui->TextFrom->clear();
		ui->TextTo->clear();
		ui->TextFrom->setFocus();
	}
	else if ((event->key() == Qt::Key_Alt) || (event->key() == Qt::Key_AltGr))  //按下Alt键切换中英文转换
	{
		if (ui->ZHTOEN->isChecked())
		{
			ui->ENTOZH->setChecked(true);
			from = "en";
			to = "zh";
			getReply(ui->TextFrom->toPlainText());
		}
		else
		{
			ui->ZHTOEN->setChecked(true);
			from = "zh";
			to = "en";
			getReply(ui->TextFrom->toPlainText());
		}
	}
	else
	{
        D("您的按键是" + QString::number(event->key()) + "暂时不支持该快捷键。");
	}
}



void encodeURI(QString str, QByteArray &outArr){
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    if(codec->canEncode(str)) {
        QByteArray tmpArr;
        tmpArr = codec->fromUnicode(str);
        for(int i=0,size = tmpArr.length();i<size;i++){
            char ch = tmpArr.at(i);
            if((int)ch < 128 && ch > 0){
                outArr.append(ch);
            }else{
                uchar low = ch & 0xff;
                char c[3];
                sprintf(c,"%02X",low);
                outArr.append("%").append(c);
            }
        }
    }
}

void Translater::getReply(QString q)
{
	ui->TextTo->clear();
	QString sign = MD5((VarBox.APP_ID + q + salt + VarBox.PASS_WORD).toStdString()).toStr().c_str(); //经过md5加密后的字符串
    QString all;
    QByteArray url;
    encodeURI(API"q="+q+"&from="+from+"&to="+to+"&appid="+VarBox.APP_ID+"&salt="+salt+"&sign="+sign, url);
    if (FuncBox::getTransCode(QString(url).toStdWString().c_str(), all))
		replyFinished(&all);
	else
        emit msgBox("没有网络！");
}

void Translater::on_isFix_clicked(bool checked)
{
    QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("Translate");
    IniWrite.setValue("AutoHide", !checked);
    IniWrite.endGroup();
}

void Translater::replyFinished(QString* all)
{
    qDebug() << "翻译结果：" << (*all);
	QJsonParseError jsonError;
	QJsonDocument json = QJsonDocument::fromJson(all->toUtf8(), &jsonError);
	if ((jsonError.error == QJsonParseError::NoError) && json.isObject())        //没有错误发生
	{
		QJsonObject rootObj = json.object();
		if (rootObj.contains("trans_result"))
		{
			QJsonValue resultValue = rootObj.value(QString("trans_result")); //获取翻译结果
            QString mean;
			QJsonArray array = resultValue.toArray();
			for (int i = 0; i < array.size(); i++)
			{
				QJsonObject explains = array.at(i).toObject();
				if (explains.contains("dst"))
				{
					QJsonValue dst = explains.value(QString("dst"));          //获取dst对应的值
					mean += dst.toString();
				}
			}
			ui->TextTo->setPlainText(mean);                                  //显示翻译好的字符串
		}
        else
            emit msgBox("出现错误，原因可能是：\n1. App ID、密钥错误；\n2. 要翻译的文本中含有换行符或制表符；\n3. 字数超出限制；\n4. 访问太频繁； \n5. 网络问题。");
	}
    else
        emit msgBox("出现错误，原因可能是：\n1. App ID、密钥错误；\n2. 要翻译的文本中含有换行符或制表符；\n3. 字数超出限制；\n4. 访问太频繁； \n5. 网络问题。");
}

void Translater::on_ENTOZH_clicked(bool checked)
{
	if (checked)
	{
		from = "en";
		to = "zh";
	}
	else
	{
		from = "zh";
		to = "en";
	}
	getReply(ui->TextFrom->toPlainText());
}

void Translater::on_ZHTOEN_clicked(bool checked)
{
	if (checked)
	{
		from = "zh";
		to = "en";
	}
	else
	{
		from = "en";
		to = "zh";
	}
	getReply(ui->TextFrom->toPlainText());
}

void Translater::on_pBtnCopyTranlate_clicked()
{
	QApplication::clipboard()->setText(ui->TextTo->toPlainText());
}
