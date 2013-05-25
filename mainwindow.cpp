#include <QTableView>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "screendialog.h"
#include "loadingdialog.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //ui->regGroupBox->hide();
    //ui->infoGroupBox->hide();
    ui->memoryTabWidget->hide();
    //this->setFixedHeight(this->height());
    //this->setFixedSize(this->width(), this->height());
    this->layout()->setSizeConstraint(QLayout::SetDefaultConstraint);

    connect(ui->aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(resetAll()));
    connect(ui->openAction, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->steprunButton, SIGNAL(clicked()), this, SLOT(steprun()));
    connect(ui->runButton, SIGNAL(clicked()), this, SLOT(run()));
    connect(ui->screenButton, SIGNAL(clicked()), this, SLOT(clickscreen()));

    screen = new screendialog(this);
    connect(this, SIGNAL(modified(const QString &)), screen, SLOT(fresh(const QString &)));

    commd_model = NULL;
    Reg_model = NULL;
    mainmem_model = NULL;
    staticmem_model = NULL;
    viewmem_model = NULL;

    resetAll(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resetAll(int mode) {
    if (ui->codeTableView->model()) {
        paintRow(commd_model, myCPU.getIC(), QBrush(QColor(255, 255, 255)));
        ui->addressLabel->setText("Program counter: 00000000");
    }
    else {
        ui->addressLabel->setText("Program counter: ");
    }

    myCPU.rst(mode);

    setReg(myCPU.getReg(), myCPU.REGNUM, 1);

    /*
    if (mode) {
        setTableView(ui->main_memTableView, &mainmem_model, myCPU.MAIN_MEM, myCPU.DISP_MEM,
                     myCPU.getMem(0), myCPU.MAIN_MEM, 1);

        setTableView(ui->static_memTableView, &staticmem_model, myCPU.STATIC_MEM, myCPU.MAIN_MEM,
                     myCPU.getMem(0), myCPU.STATIC_MEM, 1);
    }
    setTableView(ui->view_memTableView, &viewmem_model, myCPU.DISP_MEM, myCPU.END_MEM,
                 myCPU.getMem(0), myCPU.DISP_MEM, 1);
    */

    setTableView(ui->main_memTableView, &mainmem_model, myCPU.MAIN_MEM, myCPU.DISP_MEM,
                 myCPU.getMem(0), myCPU.MAIN_MEM, 1);

    setTableView(ui->static_memTableView, &staticmem_model, myCPU.STATIC_MEM, myCPU.MAIN_MEM,
                 myCPU.getMem(0), myCPU.STATIC_MEM, 1);

    setTableView(ui->view_memTableView, &viewmem_model, myCPU.DISP_MEM, myCPU.END_MEM,
                 myCPU.getMem(0), myCPU.DISP_MEM, 1);

    paintRow(commd_model, myCPU.getIC(), QBrush(QColor(255, 0, 0, 127)));
    emit modified(getDispContent());

    if (!mode) {
        ui->breakpointLineEdit->setEnabled(true);
        ui->endpointLineEdit->setEnabled(true);
        //ui->codeTableView->selectRow(0);
    }
}

int MainWindow::paintRow(QStandardItemModel *model, int row, const QBrush &brush) {
    if (!model) return 1;
    for (int i = 0; model->item(row, i); i++)
        model->item(row, i)->setBackground(brush);
    return 0;
}

void MainWindow::setReg(const dword Reg[], int size, int mode) {
    if (mode == 0) {
        int i;
        for (i = 0; i < (size-2)/2; i++) {
            if (Reg_model->item(i, 1)->text() != dword2QString(Reg[i])) {
                Reg_model->item(i, 1)->setText(dword2QString(Reg[i]));
                Reg_model->item(i, 1)->setBackground(QBrush(QColor(0, 0, 255, 127)));
            }
            else
                Reg_model->item(i, 1)->setBackground(QBrush(QColor(255, 255, 255)));

            if (Reg_model->item(i, 3)->text() != dword2QString(Reg[i+(size-2)/2])) {
                Reg_model->item(i, 3)->setText(dword2QString(Reg[i+(size-2)/2]));
                Reg_model->item(i, 3)->setBackground(QBrush(QColor(0, 0, 255, 127)));
            }
            else
                Reg_model->item(i, 3)->setBackground(QBrush(QColor(255, 255, 255)));
        }

        if (Reg_model->item(i, 1)->text() != dword2QString(Reg[size-2])) {
            Reg_model->item(i, 1)->setText(dword2QString(Reg[size-2]));
            Reg_model->item(i, 1)->setBackground(QBrush(QColor(0, 0, 255, 127)));
        }
        else
            Reg_model->item(i, 1)->setBackground(QBrush(QColor(255, 255, 255)));

        if (Reg_model->item(i, 3)->text() != dword2QString(Reg[size-1])) {
            Reg_model->item(i, 3)->setText(dword2QString(Reg[size-1]));
            Reg_model->item(i, 3)->setBackground(QBrush(QColor(0, 0, 255, 127)));
        }
        else
            Reg_model->item(i, 3)->setBackground(QBrush(QColor(255, 255, 255)));

        return;
    }


    QStandardItemModel* model = new QStandardItemModel;
    model->setColumnCount(4);
    model->setHeaderData(0, Qt::Horizontal, tr("Reg"));
    model->setHeaderData(1, Qt::Horizontal, tr("Content"));
    model->setHeaderData(2, Qt::Horizontal, tr("Reg"));
    model->setHeaderData(3 ,Qt::Horizontal, tr("Content"));

    QString temp;
    deassembler t;
    char s[5];
    for (int i = 0; i < (size-2)/2; i++) {
        t.regName(s, (dword)i);
        temp.sprintf("%s (%d)", s, i);
        model->setItem(i, 0, new QStandardItem(temp));
        model->item(i, 0)->setTextAlignment(Qt::AlignCenter);

        model->setItem(i, 1, new QStandardItem(dword2QString(Reg[i])));
        model->item(i, 1)->setTextAlignment(Qt::AlignCenter);

        int j = i + (size-2)/2;
        t.regName(s, (dword)j);
        temp.sprintf("%s (%d)", s, j);
        model->setItem(i, 2, new QStandardItem(temp));
        model->item(i, 2)->setTextAlignment(Qt::AlignCenter);

        model->setItem(i, 3, new QStandardItem(dword2QString(Reg[j])));
        model->item(i, 3)->setTextAlignment(Qt::AlignCenter);
    }
    int i = size/2 - 1;
    t.regName(s, (dword)32);
    model->setItem(i, 0, new QStandardItem(QString(s)));
    model->item(i, 0)->setTextAlignment(Qt::AlignCenter);

    model->setItem(i, 1, new QStandardItem(dword2QString(Reg[size-2])));
    model->item(i, 1)->setTextAlignment(Qt::AlignCenter);

    t.regName(s, (dword)33);
    model->setItem(i, 2, new QStandardItem(QString(s)));
    model->item(i, 2)->setTextAlignment(Qt::AlignCenter);

    model->setItem(i, 3, new QStandardItem(dword2QString(Reg[size-1])));
    model->item(i, 3)->setTextAlignment(Qt::AlignCenter);


    if (ui->regTableView->model())
        delete ui->regTableView->model();

    Reg_model = model;
    ui->regTableView->setModel(Reg_model);
    ui->regTableView->resizeColumnToContents(0);
    ui->regTableView->resizeRowsToContents();
    ui->regTableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->regTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
}

int MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open MIPS command file"), "*.s",
                tr("MIPS command file (*.s)"));

    QString fileName_kernel = QFileDialog::getOpenFileName(
                this, tr("Open MIPS kernel file"), "*.s",
                tr("MIPS command file (*.s)"));

    if (!fileName.isEmpty() && !loadFile(fileName) &&
        !fileName_kernel.isEmpty() && !loadFile_kernel(fileName_kernel))
    {
        return 0;
    }
    else {
        /*
        ui->runButton->setEnabled(false);
        ui->steprunButton->setEnabled(false);
        ui->resetButton->setEnabled(false);
        ui->screenButton->setEnabled(false);
        */
        QMessageBox::warning(this, tr("Warning"), tr("Loading error!"), QMessageBox::Yes);
        return 1;
    }
}

int MainWindow::loadFile_kernel(QString fileName)
{
    assembler myassembler1;
    if (myassembler1.load(fileName.toStdString()))
    {
        QMessageBox::warning(this, tr("Warning"), tr("Loading Kernel error!"), QMessageBox::Yes);
        return 1;
    }

    if (myassembler1.assemble_kernel())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Assembling Kernel error!"), QMessageBox::Yes);
        return 1;
    }
    //myassembler1.print();

    dword* mem = new dword[myassembler1.get_commd_num()];
    myassembler1.save(mem);

    if (myCPU.load_mem_data(mem, myassembler1.get_commd_num(), 0, CPU::USER_MEM))
    {
        QMessageBox::warning(this, tr("Warning"), tr("CPU loading Kernel error!"), QMessageBox::Yes);
        delete [] mem;
        return 1;
    }

    delete [] mem;
    return 0;
}

int MainWindow::loadFile(QString fileName) {
    loadingdialog *loading = new loadingdialog(this);
    connect(this, SIGNAL(loadingdone()), loading, SLOT(close()));
    loading->show();


    resetAll(1);

    // assemble
    assembler myassembler;
    if (myassembler.load(fileName.toStdString()))
    {
        QMessageBox::warning(this, tr("Warning"), tr("Loading MIPS Command error!"), QMessageBox::Yes);
        return 1;
    }

    if (myassembler.assemble())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Assembling MIPS Command error!"), QMessageBox::Yes);
        return 1;
    }
    dword* mem = new dword[myassembler.get_commd_num()];
    myassembler.save(mem);
    //myassembler.print();


    // CPU load commd
    if (myCPU.boot(mem, myassembler.get_commd_num()))
    {
        QMessageBox::warning(this, tr("Warning"), tr("CPU loading MIPS Command error!"), QMessageBox::Yes);
        return 1;
    }

    // CPU load static memory
    byte *static_mem = new byte[myassembler.get_static_mem_size()];
    myassembler.save_static_mem(static_mem);
    myCPU.load_static_data(static_mem, myassembler.get_static_mem_size());

    delete [] static_mem;

    setTableView(ui->static_memTableView, &staticmem_model, myCPU.STATIC_MEM, myCPU.MAIN_MEM,
                 myCPU.getMem(0), myCPU.STATIC_MEM, 1);


    // deassemble
    deassembler mydeassembler;
    mydeassembler.load(mem, myassembler.get_commd_num(), CPU::USER_MEM);
    //mydeassembler.print();
    //std::cout << "assembler: " << myassembler.get_commd_num() << std::endl;
    //std::cout << "deassembler: " << mydeassembler.get_instru_num() << std::endl;


    QStandardItemModel* model = new QStandardItemModel;
    model->setColumnCount(3);
    model->setHeaderData(0,Qt::Horizontal,tr("Address"));
    model->setHeaderData(1,Qt::Horizontal,tr("Instruction"));
    model->setHeaderData(2,Qt::Horizontal,tr("Command"));

    int i;
    QString temp;
    printf("%d\n", mydeassembler.get_instru_num());
    for (i = 0; i < mydeassembler.get_instru_num(); i++) {

        model->setItem(i, 0, new QStandardItem(dword2QString(myCPU.USER_MEM+i*4)));
        model->item(i, 0)->setTextAlignment(Qt::AlignCenter);

        temp.sprintf("%s", mydeassembler.get_instru(i).c_str());
        model->setItem(i, 1, new QStandardItem(temp));
        model->item(i, 1)->setTextAlignment(Qt::AlignCenter);

        model->setItem(i, 2, new QStandardItem(dword2QString(mem[i])));
        model->item(i, 2)->setTextAlignment(Qt::AlignCenter);
    }
    model->setItem(i, 0, new QStandardItem("End"));
    model->item(i, 0)->setTextAlignment(Qt::AlignCenter);

    model->setItem(i, 1, new QStandardItem("End"));
    model->item(i, 1)->setTextAlignment(Qt::AlignCenter);

    model->setItem(i, 2, new QStandardItem("End"));
    model->item(i, 2)->setTextAlignment(Qt::AlignCenter);

    paintRow(model, 0, QBrush(QColor(255, 0, 0, 127)));

    delete [] mem;

    if (ui->codeTableView->model())
        delete ui->codeTableView->model();

    commd_model = model;
    ui->codeTableView->setModel(commd_model);
    //ui->codeTableView->resizeColumnsToContents();
    //ui->codeTableView->resizeColumnToContents(0);
    //ui->codeTableView->resizeRowsToContents();
    ui->codeTableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->codeTableView->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    //ui->codeTableView->selectRow(0);

    temp.sprintf("Program counter: %08X", myCPU.getPC());
    ui->addressLabel->setText(temp);

    // enable buttons
    ui->runButton->setEnabled(true);
    ui->steprunButton->setEnabled(true);
    ui->resetButton->setEnabled(true);
    ui->screenButton->setEnabled(true);

    // disable widgets
    //ui->breakpointLineEdit->setEnabled(false);
    //ui->endpointLineEdit->setEnabled(false);

    ui->breakpointLineEdit->setText(dword2QString(myCPU.getEP()));
    ui->endpointLineEdit->setText(dword2QString(myCPU.getEP()));


    emit loadingdone();
    delete loading;

    return 0;
}

void MainWindow::setPoint() {
    if (ui->breakpointLineEdit->isEnabled()) {
        ui->breakpointLineEdit->setEnabled(false);
        ui->endpointLineEdit->setEnabled(false);

        breakpoint = ui->breakpointLineEdit->text().toInt(0, 16);
        breakpoint = (breakpoint - myCPU.USER_MEM) / 4 * 4 + myCPU.USER_MEM;
        myCPU.setEP(ui->endpointLineEdit->text().toInt(0, 16));

        ui->breakpointLineEdit->setText(dword2QString(breakpoint));
        ui->endpointLineEdit->setText(dword2QString(myCPU.getEP()));
    }
}

int MainWindow::steprun() {
    setPoint();

    paintRow(commd_model, myCPU.getIC(), QBrush(QColor(255, 255, 255)));

    int flag = myCPU.execute_single();

    paintRow(commd_model, myCPU.getIC(), QBrush(QColor(255, 0, 0, 127)));
    //ui->codeTableView->selectRow((int)CPU.getIC());

    if (!flag) {
        QString temp;
        temp.sprintf("Program counter: %08X", myCPU.getPC());
        ui->addressLabel->setText(temp);

        setReg(myCPU.getReg(), myCPU.REGNUM);

        if (myCPU.is_mem_modified()) {
            /*
            dword addr = myCPU.get_mem_modified_addr();

            if (myCPU.DISP_MEM <= addr && addr < myCPU.END_MEM) {
                setTableView(ui->view_memTableView, &viewmem_model,
                             myCPU.DISP_MEM, myCPU.END_MEM,
                             myCPU.getMem(0), addr);
                emit modified(getDispContent());
            }
            else
                if (myCPU.STATIC_MEM <= addr && addr < myCPU.MAIN_MEM) {
                    setTableView(ui->static_memTableView, &staticmem_model, myCPU.STATIC_MEM, myCPU.MAIN_MEM,
                                 myCPU.getMem(0), addr);
                }
                else
                    if (myCPU.MAIN_MEM <= addr && addr < myCPU.DISP_MEM) {
                        setTableView(ui->main_memTableView, &mainmem_model, myCPU.MAIN_MEM, myCPU.DISP_MEM,
                                     myCPU.getMem(0), addr);
                    }
            */

            setTableView(ui->main_memTableView, &mainmem_model, myCPU.MAIN_MEM, myCPU.DISP_MEM,
                         myCPU.getMem(0), myCPU.MAIN_MEM, 1);

            setTableView(ui->static_memTableView, &staticmem_model, myCPU.STATIC_MEM, myCPU.MAIN_MEM,
                         myCPU.getMem(0), myCPU.STATIC_MEM, 1);

            setTableView(ui->view_memTableView, &viewmem_model, myCPU.DISP_MEM, myCPU.END_MEM,
                         myCPU.getMem(0), myCPU.DISP_MEM, 1);

            emit modified(getDispContent());
        }

        return 0;
    }
    else {
        //paintRow(commd_model, CPU.getIC(), QBrush(QColor(255, 0, 0, 127)));
        QMessageBox::warning(this, tr("Warning"), tr("End point!\nExecution is stopped!"), QMessageBox::Yes);
        return 1;
    }
}

int MainWindow::run() {
    setPoint();

    for (;;) {
        if (myCPU.getPC() == breakpoint) {
            QMessageBox::warning(this, tr("Warning"), tr("Break point!\nExecution is stopped!"),
                                 QMessageBox::Yes);
            break;
        }
        if (steprun())
            return 0;
    }
    return 1;
}

void MainWindow::clickscreen() {
    if (screen->isHidden())
        screen->show();
    else
        screen->hide();
}

QString MainWindow::getDispContent() {
    QString content;
    const byte *memory = myCPU.getDispMem();
    for (int i = 0; i < myCPU.WIDTH*myCPU.HEIGHT; i++) {
        if (i && i % myCPU.WIDTH == 0)
            content += QChar('\n');
        if (memory[i])
            content += QChar(memory[i]);
        else
            content += QChar(' ');
    }
    return content;
}

QString MainWindow::dword2QString(const dword &data) {
    QString str;
    str.sprintf("%08X", data);
    return str;
}

void MainWindow::setTableView(QTableView *tableview, QStandardItemModel **view_model,
                              dword addr_st, dword addr_ed, const byte *mem_ptr,
                              dword addr, int mode) {
    if (mode == 0) {
        for (dword i = addr; i < addr+4; i++) {
            QStringList t;
            QString str;
            dword row = (i - addr_st) / 4, col = i % 4;

            str = QString((*view_model)->item(row, 1)->text());
            t = QString((*view_model)->item(row, 1)->text()).split(" ");
            QString temp;
            temp.sprintf("%02X", mem_ptr[i]);
            t[col] = temp;
            (*view_model)->setItem(row, 1, new QStandardItem( t.join(" ") ));
            (*view_model)->item(row, 1)->setTextAlignment(Qt::AlignCenter);

            str = QString((*view_model)->item(row, 2)->text());
            str[col*2] = (mem_ptr[i] > 31 && mem_ptr[i] < 127) ? QChar(mem_ptr[i]) : QChar('.');
            (*view_model)->setItem(row, 2, new QStandardItem( str ));
            (*view_model)->item(row, 2)->setTextAlignment(Qt::AlignCenter);
        }
        return;
    }

    QStandardItemModel *model = new QStandardItemModel;
    model->setColumnCount(3);
    model->setHeaderData(0, Qt::Horizontal, tr("Address"));
    model->setHeaderData(1, Qt::Horizontal, tr("Content"));
    model->setHeaderData(2, Qt::Horizontal, tr("Ascii"));

    for (int i = 0; addr_st+i < addr_ed; i+=4) {
        QStringList t;
        int row = i/4;

        model->setItem(row, 0, new QStandardItem(dword2QString(addr_st+i)));
        model->item(row, 0)->setTextAlignment(Qt::AlignCenter);

        t.clear();
        for (int j = 0; j < 4; j++) {
            QString str;
            str.sprintf("%02X", mem_ptr[addr_st+i+j]);
            t << str;
        }
        model->setItem(row, 1, new QStandardItem( t.join(" ") ));
        model->item(row, 1)->setTextAlignment(Qt::AlignCenter);

        t.clear();
        for (int j = 0; j < 4; j++) {
            t << ( (mem_ptr[addr_st+i+j] > 31 && mem_ptr[addr_st+i+j] < 127) ?
                        QChar(mem_ptr[addr_st+i+j]) : QChar('.') );
        }
        model->setItem(row, 2, new QStandardItem( t.join(" ") ));
        model->item(row, 2)->setTextAlignment(Qt::AlignCenter);
    }

    if (tableview->model())
        delete tableview->model();

    *view_model = model;

    tableview->setModel(*view_model);
    //tableview->resizeColumnsToContents();
    //tableview->resizeColumnToContents(0);
    //tableview->resizeRowsToContents();
    tableview->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    tableview->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
}

void MainWindow::about() {
    QMessageBox::about(this, tr("About MIPS Virtual Machine"),
                       tr("<h2>MIPS Virtual Machine v1.0</h2>"
                          "<p>Copyright &copy; 2013 WJR<p>"
                          "MIPS Virtual Machine is a software implemented abstraction of the underlying MIPS hardware,"
                          "It emulates the MIPS-based machine and implement many basic instrution and psedo-instrution."));
}
