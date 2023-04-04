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
  HttpLib::m_Proxy->SetProxy(PluginObject::QString2Utf8(ui->lineProxy->text()), false);
  HttpLib::m_Proxy->SetUsername(PluginObject::QString2Utf8(ui->lineUsername->text()), false);
  HttpLib::m_Proxy->SetPassword(PluginObject::QString2Utf8(ui->linePassword->text()), false);
  // HttpLib::m_Proxy.port = m_Port = ui->linePort->text().toInt();

  if (ui->rBtnSystemProxy->isChecked()) {
    HttpLib::m_Proxy->SetType(0, false);
  } else if (ui->rBtnUserProxy->isChecked()) {
    HttpLib::m_Proxy->SetType(1, false);
  } else {
    HttpLib::m_Proxy->SetType(2, false);
  }
  HttpLib::m_Proxy->SaveData();
  mgr->ShowMsg("保存成功~");
}

void TabNetProxy::InitData()
{
  switch (HttpLib::m_Proxy->GetType()) {
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

  ui->lineProxy->setText(PluginObject::Utf82QString(HttpLib::m_Proxy->GetProxy()));
  ui->linePassword->setText(PluginObject::Utf82QString(HttpLib::m_Proxy->GetPassword()));
  ui->lineUsername->setText(PluginObject::Utf82QString(HttpLib::m_Proxy->GetUsername()));
}


