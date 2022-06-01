#include "pathdialog.h"

#include <QStandardPaths>

PathDialog::PathDialog()
    : QDialog()
    , m_ImageDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
    , m_DataDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.config/Neobox")
{

}
