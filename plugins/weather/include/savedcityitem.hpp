#ifndef SAVEDCITYITEM_HPP
#define SAVEDCITYITEM_HPP

#include <QWidget>

#include <weathercfg.h>
#include <citylist.hpp>

#include <QListWidgetItem>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

class SavedCityItem: public QWidget
{
  Q_OBJECT

public:
  const CityInfo m_Info;
  QListWidgetItem*const m_Item;

  explicit SavedCityItem(QWidget* parent, QListWidgetItem* i, const std::u8string& curId, std::u8string_view id, const YJson& item)
    : QWidget(parent)
    , m_Info(CityInfo::FromJSON(id, item))
    , m_Delete(new QPushButton(this))
    , m_Checked(new QRadioButton(this))
    , m_Item(i)
    , m_CurId(curId)
  {
    setObjectName("SavedCityItem");
    auto text = m_Info.adm1 + u8" - " + m_Info.adm2 + u8" - " + m_Info.name;
    m_Checked->setText(QString::fromUtf8(text.data(), text.size()));
    m_Checked->setChecked(curId == id);
    SetupLayout();
    m_Delete->setObjectName("CityDelete");
    m_Delete->hide();
    ConnectAll();
  }
  virtual ~SavedCityItem() {}
  void SetChecked(bool check) {
    m_Checked->setChecked(check);
  }

  void SetupLayout() {
    m_Delete->setFixedSize(16, 16);
    m_Delete->setCursor(Qt::PointingHandCursor);
    auto layout = new QHBoxLayout(this);
    layout->addWidget(m_Checked);
    layout->addWidget(m_Delete);
  }
  QPushButton* const m_Delete;
private:
  QRadioButton*const m_Checked;
  const std::u8string& m_CurId;
private:
  void ConnectAll() {
    connect(m_Delete, &QPushButton::clicked, this, [this]() {
      if (m_CurId != m_Info.id) {
        emit Deleted(this);
      }
    });
    connect(m_Checked, &QRadioButton::clicked, this, [this]() {
      if (m_CurId != m_Info.id) {
        emit Clicked(this);
      }
    });
  }
protected:
  void enterEvent(QEnterEvent *event) override {
    if (m_CurId != m_Info.id) {
      m_Delete->show();
    }
  }

  void leaveEvent(QEvent *event) override {
    m_Delete->hide();
  }

  void mouseReleaseEvent(QMouseEvent *event) override {
    emit Clicked(this);
  }
signals:
  void Clicked(SavedCityItem*);
  void Deleted(SavedCityItem*);
};

#endif // SAVEDCITYITEM_HPP