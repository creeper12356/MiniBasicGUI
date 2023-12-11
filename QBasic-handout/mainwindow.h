#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "inc.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
enum Exception{
    NoLineNum
};

class Code{
public:
    int lineNum;
    QString source;
    bool emptyFlag;
    Code(const QString& source);
    Code();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QProcess* proc;
    QVector<Code> codes;
    //用于显示终端输入的文本框
    QTextBrowser* terminalReflect = nullptr;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //清空所有文本框的内容
    void clearAll();
    //从文件中读取加载代码
    void loadCodeFromFile();
    //运行代码
    void runCode();
    //显示帮助信息
    void showHelp();

    void on_cmdLineEdit_editingFinished();

    void on_inputLineEdit_editingFinished();

private:
    //查找行号lineNum的代码是否存在，若存在返回下标，不存在返回-1
    int findLineNum(int lineNum) const;
    //当lineNum不存在时，返回lineNum行号应插入的下标
    int insertLineNum(int lineNum) const;

private:
    void setTerminalReflect(QTextBrowser* browser);
    //刷新界面代码显示
    void refreshCodeDisplay();
    //后端重新加载代码
    void reload();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
