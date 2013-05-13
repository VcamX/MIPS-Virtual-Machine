#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "screendialog.h"

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
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(resetAll()));
    connect(ui->openAction, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui->steprunButton, SIGNAL(clicked()), this, SLOT(steprun()));
    connect(ui->runButton, SIGNAL(clicked()), this, SLOT(run()));
    connect(ui->screenButton, SIGNAL(clicked()), this, SLOT(clickscreen()));

    screen = new screendialog(this);
    connect(this, SIGNAL(modified(const QString &)), screen, SLOT(fresh(const QString &)));

    commd_model = NULL;
    Reg_model = NULL;

    resetAll(0);
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

    myCPU.rst();
    setReg(myCPU.getReg(), myCPU.REGNUM, 1);
    paintRow(commd_model, myCPU.getIC(), QBrush(QColor(255, 0, 0, 127)));
    emit modified(getDispContent());

    if (mode) {
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
    decompiler t;
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
    ui->codeTableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->regTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
}

int MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open MIPS command file"), "*.s",
                tr("MIPS command file (*.s)"));
    if (!fileName.isEmpty() && !loadFile(fileName)) {
        return 0;
    }
    else {
        /*
        ui->runButton->setEnabled(false);
        ui->steprunButton->setEnabled(false);
        ui->resetButton->setEnabled(false);
        ui->screenButton->setEnabled(false);
        */
        return 1;
    }
}

int MainWindow::loadFile(QString fileName) {

    resetAll();

    // compile
    compiler mycompiler;
    if (mycompiler.load(fileName.toStdString()))
        return 1;

    mycompiler.compile();
    dword* mem;
    mem = (dword*)malloc(sizeof(dword) * mycompiler.get_commd_num());
    mycompiler.save(mem);
    mycompiler.print();


    // CPU load commd
    myCPU.boot(mem, mycompiler.get_commd_num());


    // decompile
    decompiler mydecompiler;
    mydecompiler.load(mem, (dword)mycompiler.get_commd_num());
    mydecompiler.print();
    //std::cout << "compiler: " << mycompiler.get_commd_num() << std::endl;
    //std::cout << "decompiler: " << mydecompiler.get_instru_num() << std::endl;


    QStandardItemModel* model = new QStandardItemModel;
    model->setColumnCount(3);
    model->setHeaderData(0,Qt::Horizontal,tr("Address"));
    model->setHeaderData(1,Qt::Horizontal,tr("Instruction"));
    model->setHeaderData(2,Qt::Horizontal,tr("Command"));

    int i;
    QString temp;
    for (i = 0; i < mydecompiler.get_instru_num(); i++) {
        model->setItem(i, 0, new QStandardItem(dword2QString(myCPU.USER_MEM+i*4)));
        model->item(i, 0)->setTextAlignment(Qt::AlignCenter);

        temp.sprintf("%s", mydecompiler.get_instru(i).c_str());
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
    ui->codeTableView->resizeColumnsToContents();
    ui->codeTableView->resizeRowsToContents();
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

    return 0;
}

void MainWindow::setPoint() {
    if (ui->breakpointLineEdit->isEnabled()) {
        ui->breakpointLineEdit->setEnabled(false);
        ui->endpointLineEdit->setEnabled(false);

        breakpoint = ui->breakpointLineEdit->text().toInt(0, 16);
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

        if (myCPU.is_mem_modified())
            emit modified(getDispContent());

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
