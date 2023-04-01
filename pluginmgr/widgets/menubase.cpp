#include <menubase.hpp>

#include <filesystem>

#include <QFileDialog>
#include <QInputDialog>

namespace fs = std::filesystem;

MenuBase::MenuBase(QWidget* parent)
  : QMenu(parent)
{
  setWindowFlag(Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground, true);
  setToolTipsVisible(true);
}

MenuBase::~MenuBase()
{
  //
}

std::optional<std::u8string> MenuBase::GetExistingDirectory(
  QString title, const QAnyStringView oldDirectory
) {

  auto oldDir = oldDirectory.toString();
  auto const newDir = QFileDialog::getExistingDirectory(this, title, oldDir);

  if (newDir.isEmpty()) {
    return std::nullopt;
  }

  fs::path newPath = newDir.toStdU16String();
  if (newPath == oldDir.toStdU16String()) {
    return std::nullopt;
  }

  newPath.make_preferred();
  return newPath.u8string();
}

std::optional<std::u8string> MenuBase::GetNewU8String(
  QString title, QString label, const QAnyStringView oldString
) {
  const auto qNewString = QInputDialog::getText(this,
      title, label,
      QLineEdit::Normal, oldString.toString()
  );

  if (qNewString.isEmpty() || oldString == qNewString)
    return std::nullopt;
  
  auto buffer(qNewString.toUtf8());
  return std::u8string(buffer.begin(), buffer.end());
}

std::optional<QString> MenuBase::GetNewString(
  QString title, QString label, const QAnyStringView oldString
) {
  const auto qNewString = QInputDialog::getText(this,
      title, label,
      QLineEdit::Normal, oldString.toString()
  );
  if (qNewString.isEmpty() || oldString == qNewString)
    return std::nullopt;
  
  return qNewString;
}

std::optional<int> MenuBase::GetNewInt(
  QString title, QString label, int min, int max, int val
) {
  const int newVal = QInputDialog::getInt(this,
      title, label, val, min, max
  );
  if (val == newVal)
    return std::nullopt;
  
  return newVal;
}
