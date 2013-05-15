#include "loadingdialog.h"
#include "ui_loadingdialog.h"

loadingdialog::loadingdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loadingdialog)
{
    ui->setupUi(this);
}

loadingdialog::~loadingdialog()
{
    delete ui;
}
