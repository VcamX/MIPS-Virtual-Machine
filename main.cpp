#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QDateTime>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap(":/icon/icon/splash.png"));
    splash->setDisabled(true);
    splash->show();

    splash->showMessage(QObject::tr("Starting..."), Qt::AlignRight | Qt::AlignTop, Qt::white);

    MainWindow w;
    w.show();

    splash->finish(&w);
    delete splash;
    
    return a.exec();
}
