#include "tabnetproxy.hpp"

#include <yjson/yjson.h>
#include <neobox/pluginmgr.h>
#include <neobox/systemapi.h>
#include <ui_tabnetproxy.h>
#include <neobox/pluginobject.h>
#include <neobox/httplib.h>

#include <QIntValidator>
#include <QButtonGroup>

TabNetProxy::TabNetProxy(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::FormProxy)
  , m_BtnGroup(new QButtonGroup(this))
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

  m_BtnGroup->addButton(ui->rBtnSystemProxy, 0);
  m_BtnGroup->addButton(ui->rBtnUserProxy, 1);
  m_BtnGroup->addButton(ui->rBtnNoProxy, 2);
}

void TabNetProxy::InitSignals()
{
  connect(ui->pBtnSave, &QPushButton::clicked, this, &TabNetProxy::SaveData);
  connect(ui->pBtnCancel, &QPushButton::clicked, this, &TabNetProxy::InitData);
  connect(m_BtnGroup, &QButtonGroup::idClicked, this, [this](int id){
    ui->gBoxProxyInfo->setEnabled(id == 1);
  });
}

void TabNetProxy::SaveData()
{
  constexpr auto toU8 = PluginObject::QString2Utf8;
  HttpLib::m_Proxy->SetProxy(toU8(ui->lineProxy->text()), false);
  HttpLib::m_Proxy->SetUsername(toU8(ui->lineUsername->text()), false);
  HttpLib::m_Proxy->SetPassword(toU8(ui->linePassword->text()), false);
  // HttpLib::m_Proxy.port = m_Port = ui->linePort->text().toInt();

  HttpLib::m_Proxy->SetType(m_BtnGroup->checkedId(), false);
  HttpLib::m_Proxy->SaveData();
  mgr->ShowMsg("保存成功~");
}

void TabNetProxy::InitData()
{
  auto button = m_BtnGroup->button(HttpLib::m_Proxy->GetType());
  if (!button) button = ui->rBtnNoProxy;
  button->setChecked(true);

  constexpr auto toQs = PluginObject::Utf82QString;
  ui->lineProxy->setText(toQs(HttpLib::m_Proxy->GetProxy()));
  ui->linePassword->setText(toQs(HttpLib::m_Proxy->GetPassword()));
  ui->lineUsername->setText(toQs(HttpLib::m_Proxy->GetUsername()));

  ui->gBoxProxyInfo->setEnabled(m_BtnGroup->checkedId() == 1);
}


