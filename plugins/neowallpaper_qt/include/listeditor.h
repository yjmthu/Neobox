#ifndef LISTEDITOR_H
#define LISTEDITOR_H

#include <QWidget>
#include <QString>

class ListEditor: public QWidget
{
public:
  explicit ListEditor(QString title, class YJson& data, const std::function<void()> callback);
  virtual ~ListEditor();
  QString m_ArgEditTitle = "None";
  QString m_ArgEditLabel = "Null";
private:
  void SetBaseLayout();
  void SaveData();
  YJson& m_Data;
  class QListWidget* m_List;
  const std::function<void()> m_CallBack;
};

#endif // LISTEDITOR_H