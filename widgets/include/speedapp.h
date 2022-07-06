#ifndef SPEEDAPP_H
#define SPEEDAPP_H

#include <QObject>

class VarBox : public QObject
{
  Q_OBJECT
public:
  VarBox();
  ~VarBox();

private:
  void LoadSettings();
  void LoadFonts();
  void LoadQmlFiles();
};

extern VarBox* m_VarBox;

#endif // SPEEDAPP_H
