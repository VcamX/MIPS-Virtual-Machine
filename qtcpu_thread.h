#ifndef QTCPU_THREAD_H
#define QTCPU_THREAD_H

#include "qtcpu.h"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class qtCPU_thread : public QThread
{
    Q_OBJECT
public:
    explicit qtCPU_thread(QObject *parent = 0);
    ~qtCPU_thread();

    qtCPU *cpu;

signals:
    
public slots:
    virtual void run();

    void load_cpu(qtCPU *u);
    bool delete_cpu();

    void cpu_stop();
    void cpu_run();

    void keyboard_irq(const byte val);

private:
    bool loaded_flag, stop_flag;

    QMutex mutex;
    QWaitCondition run_condition;
};

#endif // QTCPU_THREAD_H
