#ifndef MENUBASE_HPP
#define MENUBASE_HPP

#include <QMenu>

#if defined(MYSHAREDLIB_LIBRARY)
#  define MYSHAREDLIB_EXPORT Q_DECL_IMPORT
#else
#  define MYSHAREDLIB_EXPORT Q_DECL_EXPORT
#endif

class MYSHAREDLIB_EXPORT MenuBase: public QMenu
{
  Q_OBJECT

public:
  explicit MenuBase(QWidget* parent);
  virtual ~MenuBase();
public:
  std::optional<std::u8string> GetExistingDirectory(
    QString title, QAnyStringView oldDirectory
  );
  std::optional<std::u8string> GetNewU8String(
    QString title, QString label, QAnyStringView oldString
  );
  std::optional<QString> GetNewString(
    QString title, QString label, QAnyStringView oldString
  );
  std::optional<int> GetNewInt(
    QString title, QString label, int min, int max, int val
  );
};

#endif // MENUBASE_HPP