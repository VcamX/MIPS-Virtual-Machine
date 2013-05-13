#ifndef SCREENDIALOG_H
#define SCREENDIALOG_H

#include <QDialog>
#include "CPU.h"

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

public slots:
    void fresh(const QString &content);
    
private:
    Ui::screendialog *ui;

    QString content;
};

#endif // SCREENDIALOG_H
