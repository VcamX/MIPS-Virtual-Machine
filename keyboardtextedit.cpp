#include "keyboardtextedit.h"
#include <QDebug>
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopWidget>

keyboardTextEdit::keyboardTextEdit(QWidget *parent) :
    QTextEdit(parent)
{
    /*
     * set color of background
     */
    setAutoFillBackground(true);
    QPalette palette;
    palette.setBrush(QPalette::Active, QPalette::Text, QBrush(Qt::white));
    palette.setBrush(QPalette::Active, QPalette::Base, QBrush(Qt::black));
    setPalette(palette);

    setGeometry(0, 0, 810, 410);
    setMinimumSize(810, 410);
    setMaximumSize(810, 410);

    setFont(QFont(QString("Lucida Console"), 12));
    setAutoFillBackground(false);
    setFocusPolicy(Qt::StrongFocus);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setUndoRedoEnabled(false);
    setReadOnly(true);
    setAcceptRichText(false);
    setCursorWidth(1);
    setTextInteractionFlags(Qt::TextSelectableByKeyboard);
    setWordWrapMode(QTextOption::ManualWrap);

    init(QChar(' '));
    row = 0;
    col = 0;
    set_cursor_pos(row, col);

    /*
    fresh( 0x0000F830, 0x61626364 );
    fresh( 0x0000F87E, 0x61626364 );
    fresh( 0x0000FFB0, 0x61626364 );
    fresh( 0x0000FFFD, 0x61626364 );
    */
}

keyboardTextEdit::~keyboardTextEdit()
{
}

void keyboardTextEdit::about_to_send_scancode(const int val, const int mode)
{
    byte key_val = (byte)( mode ? val|0x80 : val );
    qDebug() << "Keyboard sent scancode:" << key_val;
    emit send_scancode( key_val );
}

void keyboardTextEdit::about_to_send_asciicode(const byte val)
{
    qDebug() << "Keyboard sent asciicode:" << val;
    emit send_asciicode( val );
}

void keyboardTextEdit::closeEvent(QCloseEvent* e)
{
    e->accept();
}

void keyboardTextEdit::keyPressEvent(QKeyEvent* e)
{
    //qDebug() << "key press(scan code):" << e->nativeScanCode();
    //about_to_send_scancode(e->nativeScanCode(), 0);

    if (!e->text().isEmpty())
    {
        byte val = e->text().at(0).toAscii();
        //qDebug() << e->text() << val;
        about_to_send_asciicode(val);
    }

    /*
    int val;
    if (Qt::Key_Space <= e->key() && e->key() <= Qt::Key_QuoteLeft)
    {
         val = (byte)e->key();
         if (e->modifiers() == Qt::ShiftModifier &&
             0x41 <= val && val <= 0x5a)
         {
             val += 0x20;
         }
    }
    else if (Qt::)
    qDebug() << "ascii code:" << val;
    about_to_send_asciicode(val);
    */

    /*
    if (e->key() == Qt::Key_Right)
    {
        if (++col >= WIDTH)
        {
            col = 0;
            if (++row >= HEIGHT)
                row = 0;
        }
        set_cursor_pos(row, col);
        qDebug() << row << col;
    }
    else if (e->key() == Qt::Key_Down)
    {
        if (++row >= HEIGHT)
            row = 0;
        set_cursor_pos(row, col);
        qDebug() << row << col;
    }
    else if (e->key() == Qt::Key_Escape)
    {
        fresh( 0x0000FFB0, 0x61626364 );
        setTextInteractionFlags(Qt::NoTextInteraction);
    }
    else if (e->key() == Qt::Key_Return)
    {
        setTextInteractionFlags(Qt::TextSelectableByKeyboard);
    }
    */

}

void keyboardTextEdit::keyReleaseEvent(QKeyEvent* e)
{
    //qDebug() << "key release(scan code):" << e->nativeScanCode();
    //about_to_send_scancode(e->nativeScanCode(), 1);
}

void keyboardTextEdit::init(const byte* mem)
{
    QString str;
    for (int i = 0; i < WIDTH*HEIGHT; i++)
    {
        str.append( QChar(mem[i]) );
    }
    setText(str);
    set_cursor_pos(0, 0);
}

void keyboardTextEdit::fresh(const dword addr, const dword val)
{
    dword t_addr = addr - DISP_MEM;
    t_addr = (t_addr / WIDTH) * (WIDTH+1) + (t_addr % WIDTH);
    QString content = document()->toPlainText();
    int i = 0;
    while (i < 4)
    {
        if (content[t_addr] == QChar('\n'))
            t_addr++;
        if (t_addr < (dword)content.length())
        {
            byte ch = (val >> (8*(3-i))) & 0xFF;
            content[t_addr++] = (0x20 < ch && ch < 0x7F) ? QChar(ch) : QChar(' ');
        }
        i++;
    }
    setText(content);
}

void keyboardTextEdit::init(const QChar ch)
{
    QString str;
    for (int i = 0; i < WIDTH*HEIGHT; i++)
    {
        if (i && i % WIDTH == 0)
            str.append('\n');
        str.append( ch );
    }
    setText(str);
    set_cursor_pos(0, 0);
    //qDebug() << str.length();
}

void keyboardTextEdit::set_cursor_pos(const byte row, const byte col)
{
    if (WIDTH*row + col >= WIDTH*HEIGHT) return;
    int pos = row * (WIDTH+1) + col;
    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    setTextCursor(cursor);
}
