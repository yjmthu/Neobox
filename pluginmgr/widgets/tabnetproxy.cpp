#include "tabnetproxy.hpp"

#include <yjson.h>
#include <pluginmgr.h>
#include <systemapi.h>
#include <ui_tabnetproxy.h>
#include <pluginobject.h>
#include <httplib.h>

#include <QIntValidator>

TabNetProxy::TabNetProxy(QWidget* parent)
  : QWidget(parent)
  , m_Settings(mgr->GetNetProxy())
  , m_Domain(m_Settings[u8"Domain"].getValueString())
  , m_Password(m_Settings[u8"Password"].getValueString())
  , m_Username(m_Settings[u8"Username"].getValueString())
  , m_Port(m_Settings[u8"Port"].getValueDouble())
  , m_Type(m_Settings[u8"Type"].getValueDouble())
  , ui(new Ui::FormProxy)
{
  InitLayout();
  auto validator = new QIntValidator;
  validator->setRange(1, 65535);
  ui->linePort->setValidator(validator);
  InitSignals();
  InitData();
}

TabNetProxy::~TabNetProxy()
{
  delete ui;
}

void TabNetProxy::InitLayout()
{
  auto const background = new QWidget(this);
  auto mainLayout  = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0,0,0,0);
  mainLayout->addWidget(background);
  ui->setupUi(background);
  background->setObjectName("whiteBackground");
}

void TabNetProxy::InitSignals()
{
  connect(ui->pBtnSave, &QPushButton::clicked, this, &TabNetProxy::SaveData);
  connect(ui->pBtnCancel, &QPushButton::clicked, this, &TabNetProxy::InitData);
  connect(ui->rBtnSystemProxy, &QRadioButton::toggled, this, [this](bool checked){
    if (checked) ui->gBoxProxyInfo->setEnabled(false);
  });
  connect(ui->rBtnUserProxy, &QRadioButton::toggled, this, [this](bool checked){
    if (checked) ui->gBoxProxyInfo->setEnabled(true);
  });
  connect(ui->rBtnNoProxy, &QRadioButton::toggled, this, [this](bool checked){
    if (checked) ui->gBoxProxyInfo->setEnabled(false);
  });
}

void TabNetProxy::SaveData()
{
  m_Domain = PluginObject::QString2Utf8(ui->lineDomain->text());
  m_Username = PluginObject::QString2Utf8(ui->lineUsername->text());
  m_Password = PluginObject::QString2Utf8(ui->linePassword->text());
#ifdef _WIN32
  HttpLib::m_Proxy.domain = Utf82WideString(m_Domain);
  HttpLib::m_Proxy.username = Utf82WideString(m_Username);
  HttpLib::m_Proxy.password = Utf82WideString(m_Password);
  HttpLib::m_Proxy.port = m_Port = ui->linePort->text().toInt();
#elif defined (__linux__)
  HttpLib::m_Proxy.proxy.assign(m_Domain.begin(), m_Domain.end());
  HttpLib::m_Proxy.username.assign(m_Username.begin(), m_Username.end());
  HttpLib::m_Proxy.password.assign(m_Password.begin(), m_Password.end());
  // HttpLib::m_Proxy.port = m_Port = ui->linePort->text().toInt();
#endif

  if (ui->rBtnSystemProxy->isChecked()) {
    m_Type = 0;
  } else if (ui->rBtnUserProxy->isChecked()) {
    m_Type = 1;
  } else {
    m_Type = 2;
  }
  HttpLib::m_Proxy.type = m_Type;

  mgr->SaveSettings();
  mgr->ShowMsg("保存成功~");
}

void TabNetProxy::InitData()
{
  switch (static_cast<int>(m_Type)) {
    case 0:
      ui->rBtnSystemProxy->setChecked(true);
      break;
    case 1:
      ui->rBtnUserProxy->setChecked(true);
      break;
    default:
      ui->rBtnNoProxy->setChecked(true);
      break;
  }
  ui->lineDomain->setText(PluginObject::Utf82QString(m_Domain));
  ui->linePort->setText(QString::number(static_cast<int>(m_Port)));
  ui->linePassword->setText(PluginObject::Utf82QString(m_Password));
  ui->lineUsername->setText(PluginObject::Utf82QString(m_Username));
}


