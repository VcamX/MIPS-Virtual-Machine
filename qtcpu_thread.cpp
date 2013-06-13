#include "qtcpu_thread.h"
#include <QDebug>

qtCPU_thread::qtCPU_thread(QObject *parent) :
    QThread(parent)
{
    cpu = NULL;
    loaded_flag = false;
    stop_flag = true;
    have_irq = false;
    irq_key_value = 0;
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
    qDebug() << "CPU is running at thread" << currentThreadId();
    QMutexLocker locker(&mutex);
    stop_flag = false;
    run_condition.wakeOne();
}

void qtCPU_thread::cpu_stop()
{
    qDebug() << "CPU stoppd at thread" << currentThreadId();
    QMutexLocker locker(&mutex);
    stop_flag = true;
}

void qtCPU_thread::keyboard_irq(const byte val)
{
    qDebug() << "Keyboard interupt happened at thread" << currentThreadId();
    QMutexLocker locker(&mutex);
    have_irq = true;
    irq_key_value = val;
}

void qtCPU_thread::run()
{
    qDebug() << currentThreadId();
    if (loaded_flag)
    {
        forever
        {
            /*
             * check whether it needs to stop
             */
            {
                QMutexLocker locker(&mutex);
                if (stop_flag)
                    run_condition.wait(&mutex);
            }

            /*
             * cpu run once
             */
            {
                QMutexLocker locker(&mutex);
                if (!cpu->pc_increment(0))
                    break;
            }

            /*
             * check whether there is keyboard_irq
             */
            {
                QMutexLocker locker(&mutex);
                if (have_irq)
                {
                    have_irq = false;
                    cpu->set_keyboard_irq(irq_key_value);
                }
            }
        }
    }
}
