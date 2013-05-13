#include "screendialog.h"
#include "ui_screendialog.h"

screendialog::screendialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::screendialog)
{
    ui->setupUi(this);
}

screendialog::~screendialog()
{
    delete ui;
}

void screendialog::fresh(const QString &content) {
    ui->screenTextBrowser->setText(content);
}
