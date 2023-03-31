#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include <QDialog>
#include <yjson.h>
#include <editorbase.hpp>

class MapEditor: public EditorBase
{
public:
  explicit MapEditor(QString title, YJson data, Callback callback);
  virtual ~MapEditor();
private:
  void SetBaseLayout();
  void SaveData();
  class QTableWidget* m_Table;
};

#endif // MAPEDITOR_H