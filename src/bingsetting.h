#ifndef BINGSETTING_H
#define BINGSETTING_H

#include <QDialog>

class GMPOperateTip;

namespace Ui {
    class BingSetting;
}

class BingSetting : public QDialog
{
    Q_OBJECT
protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

public:
    BingSetting();
    ~BingSetting();

private:
    Ui::BingSetting *ui;
    bool mouse_moved = false;
    QPoint _startPos;                                    //记录鼠标
    QPoint _endPos;                                      //鼠标移动向量
    GMPOperateTip* jobTip;

private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
};

#endif // BINGSETTING_H
