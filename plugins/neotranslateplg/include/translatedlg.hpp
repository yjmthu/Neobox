#ifndef TRANSLATEDLG_HPP
#define TRANSLATEDLG_HPP

#include <widgetbase.hpp>
#include <QPlainTextEdit>
#include <QTextBrowser>

#include <translate.h>
#include <pluginobject.h>

class QPushButton;

class NeoTranslateDlg : public WidgetBase
{
  Q_OBJECT

protected:
  void showEvent(QShowEvent*) override;
  void hideEvent(QHideEvent *event) override;
  bool eventFilter(QObject*, QEvent*) override;
  void resizeEvent(QResizeEvent *event) override;

public:
  explicit NeoTranslateDlg(class TranslateCfg& setings);
  ~NeoTranslateDlg();
  void ToggleVisibility();
  void GetResultData(QUtf8StringView text);

private:
  friend class HeightCtrl;
  TranslateCfg& m_Settings;
  QWidget* m_CenterWidget;
  QPlainTextEdit *m_TextFrom;
  QTextBrowser *m_TextTo;
  class QComboBox *m_BoxFrom, *m_BoxTo;
  class Translate* m_Translate;
  class HeightCtrl* m_HeightCtrl;
  QPushButton* m_BtnReverse;
  QPushButton *m_BtnCopyFrom, *m_BtnCopyTo;
  QComboBox* m_BoxTransMode;
  QPoint m_LastPostion;
  QSize m_LastSize;
  bool m_LanPairChanged = false;
  bool m_TextFromChanged = false;
private:
  void SetStyleSheet();
  void SetupUi();
  void UpdateToolButtons();
  class QWidget* ReferenceObject() const;
  void CreateFromRightMenu(QMouseEvent* event);
  void CreateToRightMenu(QMouseEvent* event);
  void AddCombbox(class QHBoxLayout* layout);
signals:
  void HttpFinished(QString result);
private slots:
  void ReverseLanguage();
  void ChangeLanguageSource(int index);
  void ChangeLanguageFrom(int index);
  void ChangeLanguageTo(int index);
};

#endif // TRANSLATEDLG_HPP
