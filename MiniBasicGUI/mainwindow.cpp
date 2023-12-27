#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isRunning(false)
{
    ui->setupUi(this);
    ui->inputLineEdit->hide();
    terminalReflect = ui->ResultDisplay;
    ui->cmdLineEdit->setFocus();

    connect(ui->btnClearCode,&QPushButton::clicked,this,&MainWindow::clearAll);
    connect(ui->btnLoadCode,&QPushButton::clicked,this,&MainWindow::loadCodeFromFile);
    connect(ui->btnRunCode,&QPushButton::clicked,this,&MainWindow::runCodes);
    connect(ui->cmdLineEdit,&TerminalLineEdit::editingFinished,this,&MainWindow::parseCmd);
    connect(ui->inputLineEdit,&QLineEdit::editingFinished,this,&MainWindow::inputFinished);

    proc = new QProcess(this);
    if(!QFile::exists("./MiniBasicCore")){
        QString errorInfo = "找不到文件" + QDir::current().absolutePath() + "/MiniBasicCore";
        QMessageBox::critical(this,"错误",errorInfo);
        exit(-1);
    }
    proc->setProgram("./MiniBasicCore");
    proc->setArguments(QStringList() << "-b");
    proc->start();
    proc->waitForStarted(-1);

    connect(proc,&QProcess::readyReadStandardOutput,this,&MainWindow::readStdOut);
    connect(proc,&QProcess::readyReadStandardError,this,&MainWindow::readStdErr);
}

MainWindow::~MainWindow()
{
    qDebug() <<  "isRunning: " << this->isRunning;
    if(isRunning){
        proc->kill();
    }
    else{
        proc->write("exit\n");
    }
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
    ui->errorDisplay->clear();
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
    refreshCodeDisplay();
    reader.close();
}

void MainWindow::runCodes()
{
    if(isRunning){
        //上一次运行未停止
        QMessageBox::warning(this,"警告","程序正在运行。");
        return ;
    }
    ui->ResultDisplay->clear();
    ui->errorDisplay->clear();
    isRunning = true;
    proc->write("run\n");
    ui->treeDisplay->clear();
}

void MainWindow::showHelp()
{
    QDialog helpDialog(this);
    helpDialog.setWindowTitle("帮助");
    helpDialog.resize(340,150);
    QVBoxLayout layout(&helpDialog);
    QLabel label("MiniBasic Interpreter Developed By Creeper" ,&helpDialog);
    layout.addWidget(&label);
    helpDialog.exec();
}

void MainWindow::readStdOut()
{
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
               ui->inputLineEdit->show();
               ui->inputLineEdit->setFocus();

               ui->btnClearCode->setEnabled(false);
               ui->btnLoadCode->setEnabled(false);
               ui->btnRunCode->setEnabled(false);
               ui->cmdLineEdit->setReadOnly(true);
               ui->cmdLineEdit->setEnabled(false);
           }
           else if(ch == '$'){
               //mark run finished
               isRunning = false;
               ui->treeDisplay->clear();
               proc->write("analyze\n");
           }
           else if(ch == '['){
               //格式开始符
               isPlainText = false;
           }
           else if(ch == ']'){
               //格式结束符
               QString html = "<b style=\"color:red\">" + htmlContent + "</b> ";
               htmlContent.clear();
               isPlainText = true;
               terminalReflect->insertHtml(html);
           }
           else{
               if(isPlainText){
                   terminalReflect->insertPlainText(QString(ch));
               }
               else{
                   //缓存html内容
                    htmlContent.append(ch);
               }
           }
       }
}

void MainWindow::readStdErr()
{
    ui->errorDisplay->insertPlainText(QString::fromUtf8(proc->readAllStandardError()));
}

void MainWindow::parseCmd()
{
    QString cmd = ui->cmdLineEdit->text().trimmed();
    if(cmd == ""){
        //do not accept empty input
        return ;
    }
    if(cmd == "RUN"){
        runCodes();
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

void MainWindow::inputFinished()
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

        ui->btnClearCode->setEnabled(true);
        ui->btnLoadCode->setEnabled(true);
        ui->btnRunCode->setEnabled(true);
        ui->cmdLineEdit->setEnabled(true);
        ui->cmdLineEdit->setReadOnly(false);
        ui->cmdLineEdit->setFocus();

        if(terminalReflect){
            terminalReflect->insertPlainText(input + "\n");
        }
    }
}
