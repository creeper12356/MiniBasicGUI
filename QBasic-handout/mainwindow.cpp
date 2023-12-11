#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->inputLineEdit->hide();
    connect(ui->btnClearCode,&QPushButton::clicked,this,&MainWindow::clearAll);
    connect(ui->btnLoadCode,&QPushButton::clicked,this,&MainWindow::loadCodeFromFile);
    connect(ui->btnRunCode,&QPushButton::clicked,this,&MainWindow::runCode);
    proc = new QProcess(this);
    proc->setProgram("./MiniBasicCore");
    proc->setArguments(QStringList() << "-s");
    proc->start();

    connect(proc,&QProcess::readyReadStandardOutput,this,[this](){
        QString content = QString::fromUtf8(proc->readAllStandardOutput()).trimmed();
        if(content.endsWith("?")){
            //进程等待输入
            ui->inputLineEdit->setFocus();
            ui->inputLineEdit->show();
        }
        if(terminalReflect){
            terminalReflect->append(content);
        }
    });
    connect(proc,&QProcess::readyReadStandardError,this,[this](){
        QString rawContent = proc->readAllStandardError();
        QStringList contents = rawContent.split("\n",QString::SkipEmptyParts);
        for(auto& content:contents){
            QMessageBox::warning(this,"warning",content);
        }
    });
    ui->cmdLineEdit->setFocus();
}

MainWindow::~MainWindow()
{
    proc->write("exit\n");
    proc->waitForFinished(-1);
    qDebug() << "finished.";
    delete ui;
}

void MainWindow::clearAll()
{
    proc->write("load\n\n");
    codes.clear();
    ui->CodeDisplay->clear();
    ui->ResultDisplay->clear();
    ui->treeDisplay->clear();
}

void MainWindow::loadCodeFromFile()
{
    QString codeFileName = QFileDialog::getOpenFileName(this,"打开文件",QDir::currentPath());
    if(codeFileName.isEmpty()){
        qDebug() << "not opening a file.";
        return ;
    }
    //TODO: read line by line
    QFile reader(codeFileName);
    reader.open(QIODevice::ReadOnly);
    QTextStream ts(&reader);
    codes.clear();
    while(!ts.atEnd()){
        //TODO without lineNum???
        codes.append(Code(ts.readLine()));
    }
    reload();
    refreshCodeDisplay();
    reader.close();
}

void MainWindow::runCode()
{
    if(ui->inputLineEdit->isVisible()){
        //上一次运行未停止
        QMessageBox::warning(this,"警告","程序正在运行。");
        return ;
    }
    ui->ResultDisplay->clear();
    setTerminalReflect(ui->ResultDisplay);
    proc->write("run\n");
}

void MainWindow::showHelp()
{
    QDialog helpDialog(this);
    helpDialog.setWindowTitle("帮助");
    QLabel label("MiniBasic Interpreter Developed By Creeper" ,&helpDialog);
    helpDialog.exec();
}

void MainWindow::on_cmdLineEdit_editingFinished()
{
    //TODO: rename this
    QString cmd = ui->cmdLineEdit->text().trimmed();
    if(cmd == ""){
        return ;
    }
    if(cmd == "RUN"){
        runCode();
    }
    else if(cmd == "CLEAR"){
        clearAll();
    }
    else if(cmd == "LOAD"){
        loadCodeFromFile();
    }
    else if(cmd == "HELP"){
        showHelp();
    }
    else if(cmd == "QUIT"){
        close();
    }
    else{
        try{
            Code newCode(cmd);
            int index = findLineNum(newCode.lineNum);
            if(index == -1){
                //未找到行号
                if(!newCode.emptyFlag){
                    //仅在非空行下插入
                    codes.insert(insertLineNum(newCode.lineNum),newCode);
                }
            }
            else{
                //找到行号
                if(newCode.emptyFlag){
                    //空行，删除原行
                    codes.removeAt(index);
                }
                else{
                    //非空行，覆盖
                    codes[index] = newCode;
                }
            }
            reload();
            refreshCodeDisplay();
        }
        catch(Exception e){
            QMessageBox::warning(this,"警告","您的代码缺少行号。");
            return ;
        }
    }
    ui->cmdLineEdit->clear();
}

int MainWindow::findLineNum(int lineNum) const
{
    int lh = 0,rh = codes.size() - 1;
    int mid;
    while(lh <= rh){
        mid = (lh + rh) / 2;
        if(lineNum == codes[mid].lineNum){
            return mid;
        }
        else if(lineNum > codes[mid].lineNum){
            lh = mid + 1;
        }
        else{
            rh = mid - 1;
        }
    }
    return -1;
}

int MainWindow::insertLineNum(int lineNum) const
{
    int lh = 0,rh = codes.size();
    int mid;
    while(lh < rh){
        mid = (lh + rh) / 2;
        if(lineNum == codes[mid].lineNum){
            //actually will not happen
            return mid;
        }
        else if(lineNum > codes[mid].lineNum){
            lh = mid + 1;
        }
        else{
            rh = mid;
        }
    }
    return lh;
}

void MainWindow::setTerminalReflect(QTextBrowser *browser)
{
    terminalReflect = browser;
}

void MainWindow::refreshCodeDisplay()
{
    ui->CodeDisplay->clear();
    for(auto& code:codes){
        ui->CodeDisplay->append(code.source);
    }
}

void MainWindow::reload()
{
    proc->write("load\n");
    for(Code& code: codes){
        proc->write(code.source.toUtf8() + "\n");
    }
    //finish load
    proc->write("\n");

    ui->treeDisplay->clear();
    setTerminalReflect(ui->treeDisplay);
    proc->write("analyze\n");
}


Code::Code(const QString &src)
{
    QStringList parseList = src.split(" ",QString::SkipEmptyParts);
    //assert parseList.size() > 0
    bool isNum;
    lineNum = parseList[0].toInt(&isNum);
    if(!isNum){
        throw NoLineNum;
    }
    //标记空行
    if(parseList.size() == 1){
        emptyFlag = true;
    }
    else{
        emptyFlag = false;
    }

    source = src;
}

Code::Code()
{

}

void MainWindow::on_inputLineEdit_editingFinished()
{
    QString input = ui->inputLineEdit->text();
    bool isNum;
    input.toInt(&isNum);
    if(!isNum){
        //TODO: 提示
        return ;
    }
    else{
        //写入进程
        proc->write((input + '\n').toUtf8());
        ui->inputLineEdit->clear();
        ui->inputLineEdit->hide();
        ui->cmdLineEdit->setFocus();
        if(terminalReflect){
            terminalReflect->append(input);
        }
    }
}
