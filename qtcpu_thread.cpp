#include "qtcpu_thread.h"
#include <QDebug>

qtCPU_thread::qtCPU_thread(QObject *parent) :
    QThread(parent)
{
    cpu = NULL;
    loaded_flag = false;
    stop_flag = true;
}

qtCPU_thread::~qtCPU_thread()
{
    if (this->isRunning())
    {
        this->terminate();
        this->wait();
    }
    if (cpu)
        delete cpu;
}

void qtCPU_thread::load_cpu(qtCPU *u)
{
    stop_flag = true;
    loaded_flag = true;
    if (delete_cpu())
    {
        cpu = u;
        cpu->moveToThread(this);
    }
    else
    {
        qDebug() << "Delete cpu error!";
    }
}

bool qtCPU_thread::delete_cpu()
{
    if (stop_flag)
    {
        if (cpu)
        {
            delete cpu;
        }
        return true;
    }
    else
        return false;
}

void qtCPU_thread::cpu_run()
{
    QMutexLocker locker(&mutex);
    stop_flag = false;
    run_condition.wakeOne();
}

void qtCPU_thread::cpu_stop()
{
    QMutexLocker locker(&mutex);
    stop_flag = true;
}

void qtCPU_thread::keyboard_irq(const byte val)
{
    QMutexLocker locker(&mutex);
    cpu->set_keyboard_irq(val);
}

void qtCPU_thread::run()
{
    if (loaded_flag)
    {
        forever
        {
            {
                QMutexLocker locker(&mutex);
                if (stop_flag)
                    run_condition.wait(&mutex);
            }

            {
                QMutexLocker locker(&mutex);
                if (!cpu->pc_increment(0))
                    break;
            }
        }
    }
}
