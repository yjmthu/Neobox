#ifndef BLANKFORM_H
#define BLANKFORM_H

#include <QWidget>
class QPushButton;

class BlankFrom: public QWidget
{
    Q_OBJECT
protected:
    void enterEvent(QEnterEvent *event);
    void leaveEvent(QEvent *event);
public:
    explicit BlankFrom(QWidget* p);
    QPushButton *closeButton;
    QPushButton *minButton;
private:
    void setGeometry();
};


#endif // BLANKFORM_H
