#ifndef QTCPU_H
#define QTCPU_H

#include "data_type.h"
#include <QObject>
#include <QMutex>

class qtCPU : public QObject
{
    Q_OBJECT
public:
    explicit qtCPU(QObject *parent = 0);

    int boot(const dword *commd_set, int size);
    int load_static_data(const byte *static_mem, dword size);
    int load_mem_commd_data(const dword *mem, dword size, const dword adr_st, const dword adr_ed);

    const byte* get_mem_ptr(const dword addr_st = 0);
    const dword* get_reg_ptr();

    void cpy_reg();
    void about_to_reg_update();

    enum
    {
        REGNUM = 35, REG_HI = 32, REG_LO = 33, REG_PC = 34,
        WIDTH = 80, HEIGHT = 25,
        END_MEM = 0x10000, // 64Kb
        KERNEL_MEM = 0, USER_MEM = 0x2000, STATIC_MEM = 0x5000,
        MAIN_MEM = 0x7000, DISP_MEM = END_MEM - WIDTH*HEIGHT
    };

signals:
    //int pc_update(const dword pc);
    //void ins_counter_update(const dword ins_counter);
    void reg_update(const dword reg, const dword val);
    void mem_update(const dword mem_addr, const dword val);
    //int mem_update_disp();

    void exec_result_send(bool flag);
    
public slots:
    void pc_increment(int action);
    void rst();

private slots:
    void clr_mem(byte mem[], dword size);

    void send_mem_update(const dword addr);

    int execute();

    dword get_ins(const dword addr);
    void set_keyboard_irq(const byte val);

private:
    //dword currsize;
    dword reg[REGNUM], t_reg[REGNUM];
    byte memory[END_MEM];

    dword epc;
    bool have_irq, is_in_irq;
};

#endif // QTCPU_H
