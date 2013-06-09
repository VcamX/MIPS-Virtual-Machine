#ifndef KEYBOARDTEXTEDIT_H
#define KEYBOARDTEXTEDIT_H

#include "data_type.h"
#include "qtcpu.h"
#include <QTextEdit>

class keyboardTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit keyboardTextEdit(QWidget *parent = 0);
    ~keyboardTextEdit();

signals:
    void send_scancode(const byte key_val);
    void send_asciicode(const byte key_val);

public slots:
    void init(QChar ch);
    void init(const byte* mem);
    void fresh(const dword addr, const dword val);
    void set_cursor_pos(const byte row, const byte col);

private:
    enum
    {
        DISP_MEM = qtCPU::DISP_MEM,
        WIDTH = qtCPU::WIDTH,
        HEIGHT = qtCPU::HEIGHT
    };

    int row, col;

    void about_to_send_scancode(const int val, const int mode);
    void about_to_send_asciicode(const byte val);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void keyReleaseEvent(QKeyEvent *e);
    virtual void closeEvent(QCloseEvent *e);
};

#endif // KEYBOARDTEXTEDIT_H
