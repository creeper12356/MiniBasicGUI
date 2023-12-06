#ifndef TERMINALLINEEDIT_H
#define TERMINALLINEEDIT_H
#include "inc.h"
#define MAX_SIZE 10
class TerminalLineEdit:public QLineEdit
{
private:
    QVector<QString> codes;
    int current = 0;
public:
    TerminalLineEdit(QWidget* parent = 0);
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
protected slots:
    void pushCode(const QString& newCode);
};

#endif // TERMINALLINEEDIT_H
