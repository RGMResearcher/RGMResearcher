#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_exit_triggered()
{
    this->close();
}

void MainWindow::on_open_triggered()
{
    auto* file= new QFile(QFileDialog::getOpenFileName(this,
        "Укажить путь к сохранённому графу", "", "*.gv"));
    if(file->open(QFile::ReadOnly | QFile::Text) &&
            file->fileName().endsWith(QString(".gv")))
    {
        QTextStream fin(file);
        graph gr;
        QString str;
        while (fin.readLineInto(&str))
        {
            auto lst = str.split(QString(" -- "));
            unsigned first = lst[0].toUInt();
            unsigned second = lst[1].toUInt();
            gr.new_vertex(first);
            gr.new_vertex(second);
            gr.connect_by_name(first, second);
        }
        file->close();
    }
    else
    {

    }
    delete file;
}

void MainWindow::on_save_triggered()
{
    // сохранить через qfiledialog
}
