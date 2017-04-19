#include "mainwindow.h"
#include <QApplication>
#include <fstream>
#include "graphutility.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    int t = clustered();
    MainWindow w;
    w.show();

    return a.exec();
}
