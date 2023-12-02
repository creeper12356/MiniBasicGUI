#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->btnClearCode,&QPushButton::clicked,this,&MainWindow::clearAll);
    connect(ui->btnLoadCode,&QPushButton::clicked,this,&MainWindow::loadCode);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clearAll()
{
    ui->CodeDisplay->clear();
    ui->textBrowser->clear();
    ui->treeDisplay->clear();
}

void MainWindow::loadCode()
{
    QString codeFileName = QFileDialog::getOpenFileName(this,"打开文件",QDir::currentPath());
    if(codeFileName.isEmpty()){
        qDebug() << "not opening a file.";
        return ;
    }

    QFile reader(codeFileName);
    reader.open(QIODevice::ReadOnly);
    ui->CodeDisplay->setText(reader.readAll());
    reader.close();
}

void MainWindow::showHelp()
{
    QDialog helpDialog(this);
    helpDialog.setWindowTitle("帮助");
    QLabel label("MiniBasic Interpreter Developed By Creeper" ,&helpDialog);
    helpDialog.exec();
}
