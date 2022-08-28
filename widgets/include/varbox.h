#ifndef VARBOX_H
#define VARBOX_H

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

extern VarBox *m_VarBox;

#endif
