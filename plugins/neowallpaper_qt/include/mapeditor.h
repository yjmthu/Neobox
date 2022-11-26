#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include <QWidget>

#include <functional>

class MapEditor: public QWidget
{
public:
  explicit MapEditor(QString title, class YJson& data, const std::function<void()> callback);
  virtual ~MapEditor();
private:
  void SetBaseLayout();
  void SaveData();
  YJson& m_Data;
  class QTableWidget* m_Table;
  const std::function<void()> m_CallBack;
};

#endif // MAPEDITOR_H