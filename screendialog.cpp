#include "screendialog.h"
#include "ui_screendialog.h"
#include <QKeyEvent>

screendialog::screendialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::screendialog)
{
    ui->setupUi(this);

    ui->screenTextBrowser->setAutoFillBackground(true);
    QPalette palette;
    //设置QTextEdit文字颜色
    palette.setBrush(QPalette::Active, QPalette::Text, QBrush(Qt::white));
    //设置QTextEdit背景色
    palette.setBrush(QPalette::Active, QPalette::Base, QBrush(Qt::black));
    ui->screenTextBrowser->setPalette(palette);
}

screendialog::~screendialog()
{
    delete ui;
}

void screendialog::fresh(const QString s)
{
    QString str;
    for (int i = 0; i < 25; i++)
    {
        for (int j = 0; j < 80; j++)
        {
            str.append( s[0] );
        }
        if (i < 25-1)
            str.append(QChar('\n'));
    }

    ui->screenTextBrowser->setText(str);
}

void screendialog::keyPressEvent ( QKeyEvent* ev )
{
    if (ev->text().isEmpty())
    {
        //fresh( QChar(ev->key()) );
    }
    else
    {
        fresh( ev->text() );
    }
}

