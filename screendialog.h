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

    int width, height;

protected:
    void keyPressEvent(QKeyEvent* ev);

public slots:
    void fresh(const QString s);
    
private:
    Ui::screendialog *ui;

    QString content;
};

#endif // SCREENDIALOG_H
