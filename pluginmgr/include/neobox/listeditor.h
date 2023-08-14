#ifndef LISTEDITOR_H
#define LISTEDITOR_H

#include <QDialog>
#include <QString>
#include <neobox/editorbase.hpp>

#include <yjson/yjson.h>

class ListEditor: public EditorBase
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