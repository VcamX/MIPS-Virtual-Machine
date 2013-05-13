#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFileDialog>
#include <QMessageBox>
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
    void resetAll(int mode = 1);
    int openFile();
    int loadFile(QString fileName);
    int steprun();
    int run();
    void clickscreen();

private:
    Ui::MainWindow *ui;
    void setReg(const dword Reg[], int size, int mode = 0);
    void setMainMem(const dword Mem[], int size, int mode = 0);
    void setViewMem(const dword Mem[], int size, int mode = 0);
    QString getDispContent();
    int paintRow(QStandardItemModel *model, int row, const QBrush &brush);
    void setPoint();

    // tools for convinient
    inline QString dword2QString(const dword &data);

    CPU myCPU;
    screendialog *screen;

    QStandardItemModel *Reg_model;
    QStandardItemModel *commd_model;
    dword breakpoint;
};

#endif // MAINWINDOW_H
