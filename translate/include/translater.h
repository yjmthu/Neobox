#ifndef TRANSLATER_H
#define TRANSLATER_H

#include <QWidget>

class Translater : public QWidget
{
  protected:
    bool eventFilter(QObject *target, QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);

  public:
    explicit Translater(QWidget *parent = nullptr);
    ~Translater();
    void SetupKey();

  private:
    class QPlainTextEdit *m_TextFrom;
    class QPlainTextEdit *m_TextTo;
    class QButtonGroup *m_BtnGroup;
    class QxtGlobalShortcut *m_Shortcut;
    void SetupUi();
    void GetReply(const QString &text);
  public slots:
    void IntelligentShow();
    void Translate(const QString &text);
};

#endif // TRANSLATER_H
