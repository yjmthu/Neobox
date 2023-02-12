#ifndef LISTEDITOR_H
#define LISTEDITOR_H

#include <QDialog>
#include <QString>

#include <yjson.h>

class ListEditor: public QDialog
{
public:
  explicit ListEditor(QString title, YJson::ArrayType& data, const std::function<void()> callback);
  virtual ~ListEditor();
  QString m_ArgEditTitle = "None";
  QString m_ArgEditLabel = "Null";
private:
  void SetBaseLayout();
  void SaveData();
  YJson::ArrayType& m_Data;
  class QListWidget* m_List;
  const std::function<void()> m_CallBack;
};

#endif // LISTEDITOR_H