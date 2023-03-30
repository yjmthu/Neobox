#ifndef LISTEDITOR_H
#define LISTEDITOR_H

#include <QDialog>
#include <QString>
#include <editorbase.hpp>

#include <yjson.h>

class ListEditor: public EditorBase, public QDialog
{
public:
  explicit ListEditor(QString title, YJson data, Callback callback);
  virtual ~ListEditor();
  QString m_ArgEditTitle = "None";
  QString m_ArgEditLabel = "Null";
private:
  void SetBaseLayout();
  void SaveData();
  class QListWidget* m_List;
};

#endif // LISTEDITOR_H