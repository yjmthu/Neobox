#ifndef PATHDIALOG_H
#define PATHDIALOG_H

#include <QDialog>

class PathDialog : public QDialog {
  Q_OBJECT
 public:
  PathDialog();
  QString m_ImageDir;
  QString m_DataDir;
};

#endif  // PATHDIALOG_H
