#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFileDialog>
#include <QMessageBox>
#include <QTableView>
#include <QStandardItemModel>

#include "CPU.h"
#include "compiler.h"
#include "decompiler.h"
#include "screendialog.h"

namespace Ui {
class MainWindow;
}

class screen;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void modified(const QString &);
    
private slots:
    void resetAll(int mode = 0);
    int openFile();
    int loadFile(QString fileName);
    int steprun();
    int run();
    void clickscreen();

private:
    Ui::MainWindow *ui;
    void setReg(const dword Reg[], int size, int mode = 0);
    QString getDispContent();
    int paintRow(QStandardItemModel *model, int row, const QBrush &brush);
    void setPoint();
    void setTableView(QTableView *tableview, QStandardItemModel **view_model,
                      dword addr_st, dword addr_ed, const byte *mem_ptr,
                      dword addr, int mode = 0);

    // tools for convinient
    inline QString dword2QString(const dword &data);

    CPU myCPU;
    screendialog *screen;

    QStandardItemModel *Reg_model, *commd_model, *viewmem_model, *mainmem_model;
    dword breakpoint;
};

#endif // MAINWINDOW_H
