#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFileDialog>
#include <QSplashScreen>
#include <QMessageBox>
#include <QTableView>
#include <QStandardItemModel>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QMetaType>

#include "qtcpu.h"
#include "qtcpu_thread.h"
#include "assembler.h"
#include "deassembler.h"
#include "keyboardtextedit.h"

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
    void loadingdone();

    void cpu_run_once(int action);
    void cpu_rst();

    //void disp_fresh(const dword addr, const dword val);

public slots:
    void gui_ins_counter_update(const int ins_counter);
    void gui_reg_update(const dword reg, const dword val);
    void gui_mem_update(const dword mem_addr, const dword val);
    
private slots:
    void reset_all();
    void delete_all();

    void debug_connect_init();
    void debug_disconnect_init();
    void normal_connect_init();
    void other_setting_init();

    void gui_reset();

    void gui_ins_init(const dword* mem, const dword size);
    int gui_ins_paint_row(QStandardItemModel *model, int row, const QBrush &brush);

    void gui_reg_init(const dword reg[]);

    void gui_mem_init_view(QTableView *tableview, QStandardItemModel **view_model,
                           dword addr_st, dword addr_ed, const byte *mem_ptr, int mode = 0);
    void gui_mem_init(const byte* mem_ptr);
    void gui_mem_update_view(QStandardItemModel **view_model,
                             const dword addr_st, const dword addr, const dword val);

    void gui_button_debug_init();
    void gui_button_timer_start();
    void gui_button_normal_init();
    void gui_button_normal_start();

    int calc_clock_cycle(int val);
    void gui_clk_update(int val);

    dword get_break_point();

    void about_to_open_file();
    int open_file(qtCPU *temp_cpu);
    void debug_about_to_open_file(qtCPU* new_cpu);
    void normal_about_to_open_file(qtCPU* new_cpu);

    int load_user(QString fileName, qtCPU *my_cpu);
    int load_kernel(QString fileName, qtCPU *my_cpu);

    void clock_update(int val);

    void timer_run_init();
    void timer_run_once();
    void timer_run_stop();
    void timer_run_restart();

    void exec_result_receive(bool flag);

    void screen_init();
    void disp_screen();

    void about();

private:
    Ui::MainWindow *ui;

    QDialog *screen;
    keyboardTextEdit *keyboard_screen;

    QStandardItemModel
        *reg_model, *commd_model,
        *kernel_mem_model, *main_mem_model, *static_mem_model, *disp_mem_model;

    QTimer clock;
    qtCPU* my_cpu;
    QThread* my_cpu_thread;
    qtCPU_thread t_cpu_thread;

    int ins_counter;

    dword codeview_current_row, my_cpu_current_pc;
    bool is_stopped; // for break point
    bool exec_result; // record whether cpu is kept running

    // tools for convinient
    inline QString dword2QString(const dword &data);
};

#endif // MAINWINDOW_H
