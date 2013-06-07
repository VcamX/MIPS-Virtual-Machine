#include "screendialog.h"
#include "ui_screendialog.h"
#include <QKeyEvent>
#include <QTextCursor>
#include <QDebug>
#include <QMessageBox>

screendialog::screendialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::screendialog)
{
    ui->setupUi(this);

    ui->screenTextEdit->setAutoFillBackground(true);
    QPalette palette;
    //设置QTextEdit文字颜色
    palette.setBrush(QPalette::Active, QPalette::Text, QBrush(Qt::white));
    //设置QTextEdit背景色
    palette.setBrush(QPalette::Active, QPalette::Base, QBrush(Qt::black));
    ui->screenTextEdit->setPalette(palette);

    fresh(QString("s"));
    move_cursor(5, 2);

    qDebug() << ui->screenTextEdit->document()->lineCount();

    QTextCursor cur = ui->screenTextEdit->textCursor();
    cur.movePosition(QTextCursor::EndOfBlock);
    ui->screenTextEdit->setTextCursor(cur);

    qDebug() << cur.position();
    qDebug() << cur.positionInBlock();
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

    ui->screenTextEdit->setText(str);
    //ui->screenTextEdit->setTextCursor();;
}

void screendialog::keyPressEvent ( QKeyEvent* ev )
{
    switch (ev->key())
    {
        case Qt::Key_Right :
            QMessageBox::information(NULL,"keyboard","R");
            break ;
        case Qt::Key_Up :
            QMessageBox::information(NULL,"keyboard","U");
            break ;
        case Qt::Key_Left :
            QMessageBox::information(NULL,"keyboard","L");
            break ;
        case Qt::Key_Down :
            QMessageBox::information(NULL,"keyboard","D");
            break ;
        default :
            QMessageBox::information(NULL,"keyboard","other");
            break ;
    }

    if (ev->text().isEmpty())
    {
        //fresh( QChar(ev->key()) );
    }
    else
    {
        fresh( ev->text() );
    }
}

void screendialog::move_cursor(int row, int column)
{
    /*
    QTextCursor cursor = ui->screenTextEdit->text;
    QTextDocument* doc = ui->screenTextEdit->document();
    int position = doc->findBlockByNumber(row).position();
    cursor.setPosition(position, QTextCursor::MoveAnchor);
    ui->screenTextEdit->setTextCursor(cursor);

    // 获得TextEdit的鼠标指针
    QTextCursor cursor = ui->screenTextEdit->textCursor();
   // 找到line行号所在文本中的流位置，注意，这个位置与行号是不同的
    int position = ui->screenTextEdit->document()->findBlockByNumber( row ).position();
   // 找到文本最后一行的流位置
    int end = ui->screenTextEdit->document()->
            findBlockByNumber( ui->screenTextEdit->document()->lineCount()-1 ).position();
   // 定位到文本末尾，这样做是因为QTextEdit向下定位不准，只能向上准确定位
    cursor.setPosition(end, QTextCursor::MoveAnchor);
    ui->screenTextEdit->setTextCursor(cursor);
   /// 定位到指定的行位置
    cursor.setPosition(position, QTextCursor::MoveAnchor);
    ui->screenTextEdit->setTextCursor(cursor);
    */
}
