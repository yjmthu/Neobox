#include "tabnetproxy.hpp"

#include <yjson.h>
#include <pluginmgr.h>
#include <glbobject.h>
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
  , ui(new Ui::Form)
{
  ui->setupUi(this);
  auto validator = new QIntValidator;
  validator->setRange(1, 65535);
  ui->linePort->setValidator(validator);
  InitData();
}

TabNetProxy::~TabNetProxy()
{
  delete ui;
}

void TabNetProxy::InitSignals()
{
  connect(ui->pBtnSave, &QPushButton::clicked, this, &TabNetProxy::SaveData);
  connect(ui->pBtnCancel, &QPushButton::clicked, this, &TabNetProxy::InitData);
}

void TabNetProxy::SaveData()
{
  HttpLib::m_Proxy.domain = Utf82WideString(m_Domain =
    PluginObject::QString2Utf8(ui->lineDomain->text()));
  HttpLib::m_Proxy.username = Utf82WideString(m_Username =
    PluginObject::QString2Utf8(ui->lineUsername->text()));
  HttpLib::m_Proxy.password = Utf82WideString(m_Password =
    PluginObject::QString2Utf8(ui->linePassword->text()));
  HttpLib::m_Proxy.port = m_Port = ui->linePort->text().toInt();

  if (ui->rBtnSystemProxy->isChecked()) {
    m_Type = 0;
  } else if (ui->rBtnUserProxy->isChecked()) {
    m_Type = 1;
  } else {
    m_Type = 2;
  }
  HttpLib::m_Proxy.type = m_Type;

  mgr->SaveSettings();
  glb->glbShowMsg("保存成功~");
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


