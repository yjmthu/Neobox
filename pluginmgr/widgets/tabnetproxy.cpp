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
  , m_Proxy(m_Settings[u8"Proxy"].getValueString())
  , m_Password(m_Settings[u8"Password"].getValueString())
  , m_Username(m_Settings[u8"Username"].getValueString())
  , m_Type(m_Settings[u8"Type"].getValueDouble())
  , ui(new Ui::FormProxy)
{
  InitLayout();
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
  m_Proxy = PluginObject::QString2Utf8(ui->lineProxy->text());
  m_Username = PluginObject::QString2Utf8(ui->lineUsername->text());
  m_Password = PluginObject::QString2Utf8(ui->linePassword->text());
  HttpLib::m_Proxy.proxy = m_Proxy;
  HttpLib::m_Proxy.username = m_Username;
  HttpLib::m_Proxy.password = m_Password;
  // HttpLib::m_Proxy.port = m_Port = ui->linePort->text().toInt();

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
  ui->lineProxy->setText(PluginObject::Utf82QString(m_Proxy));
  ui->linePassword->setText(PluginObject::Utf82QString(m_Password));
  ui->lineUsername->setText(PluginObject::Utf82QString(m_Username));
}


