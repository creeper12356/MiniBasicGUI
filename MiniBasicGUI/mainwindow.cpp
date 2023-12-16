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
    proc->waitForStarted(-1);

    connect(proc,&QProcess::readyReadStandardOutput,this,[this](){
       QByteArray content = proc->readAllStandardOutput();
       qDebug() << "isResultDisplay: " << (terminalReflect == ui->ResultDisplay);
       qDebug() << "content : " << content;
       for(auto ch: content){
           if(ch == '#' ){
               //switch terminal output
               if(terminalReflect == ui->ResultDisplay) terminalReflect = ui->treeDisplay;
               else terminalReflect = ui->ResultDisplay;
           }
           else if(ch == '?'){
               //wait for integer input
               terminalReflect->insertPlainText("?");
               ui->inputLineEdit->setFocus();
               ui->inputLineEdit->show();
           }
           else{
               terminalReflect->insertPlainText(QString(ch));
           }
       }
    });
    connect(proc,&QProcess::readyReadStandardError,this,[this](){
        ui->errorDisplay->insertPlainText(QString::fromUtf8(proc->readAllStandardError()));
    });
    terminalReflect = ui->ResultDisplay;
    ui->cmdLineEdit->setFocus();
}

MainWindow::~MainWindow()
{
    proc->write("exit\n");
    proc->waitForFinished(-1);
    qDebug() << "finished.";
    delete ui;
}

void MainWindow::loadCode(const QString &code)
{
    try{
        Code newCode(code);
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

    }
    catch(Exception e){
        QMessageBox::warning(this,"警告","您的代码缺少行号。");
    }
}

void MainWindow::clearAll()
{
    proc->write("clear\n");
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
    QFile reader(codeFileName);
    reader.open(QIODevice::ReadOnly);
    QTextStream ts(&reader);
    codes.clear();
    while(!ts.atEnd()){
        loadCode(ts.readLine());
    }
    refreshTreeDisplay();
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
    else if(cmd.startsWith("INPUT ")
            || cmd.startsWith("PRINT ")
            || cmd.startsWith("LET ")){
        proc->write(QString("cmd " + cmd + "\n").toUtf8());
    }
    else{
        loadCode(cmd);
        refreshTreeDisplay();
        refreshCodeDisplay();
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

void MainWindow::refreshCodeDisplay()
{
    ui->CodeDisplay->clear();
    for(auto& code:codes){
        ui->CodeDisplay->append(code.source);
    }
}

void MainWindow::refreshTreeDisplay()
{
    ui->treeDisplay->clear();
    proc->write("analyze\n");
}

void MainWindow::reload()
{
    proc->write("load\n");
    for(Code& code: codes){
        proc->write(code.source.toUtf8() + "\n");
    }
    //finish load
    proc->write("\n");
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
            terminalReflect->insertPlainText(input + "\n");
        }
    }
}
