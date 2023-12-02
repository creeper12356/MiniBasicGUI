#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "inc.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    //代码文本框中的所有代码
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //清空所有文本框的内容
    void clearAll();
    //从文件中读取加载代码
    void loadCode();
    //显示帮助信息
    void showHelp();

private:
    //给定行号，返回代码的插入位置，需要测试
    inline int insertIndex(int lineNum) const;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
