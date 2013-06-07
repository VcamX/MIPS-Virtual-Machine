#ifndef SCREENDIALOG_H
#define SCREENDIALOG_H

#include "qtCPU.h"
#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class screendialog;
}

class screendialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit screendialog(QWidget *parent = 0);
    ~screendialog();

public slots:
    void fresh(const QString s);
    void move_cursor(int row, int column);

signals:
    void key_val_send(const int val);
    
private:
    Ui::screendialog *ui;

    QString content;

protected:
    void keyPressEvent(QKeyEvent* ev);
};

#endif // SCREENDIALOG_H
