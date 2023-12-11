#include "terminallineedit.h"

TerminalLineEdit::TerminalLineEdit(QWidget *parent):
    QLineEdit(parent)
{
    //TODO : implement terminal-like effect
//	connect(this,&TerminalLineEdit::editingFinished,
//            this,[this])
}

void TerminalLineEdit::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Tab){
        qDebug() << "tab presses!";
        return ;
    }
    if(event->key() == Qt::Key_Up){
        qDebug() << "up";
        return ;
    }
    QLineEdit::keyPressEvent(event);
}

void TerminalLineEdit::focusOutEvent(QFocusEvent *event)
{
    if(event->reason() == Qt::TabFocusReason){
        qDebug() << "tab pressed";
        QKeyEvent ev(QEvent::KeyPress,Qt::Key_Tab,Qt::NoModifier);
        QCoreApplication::sendEvent(this,&ev);
        this->setFocus();
    }
}

void TerminalLineEdit::pushCode(const QString &newCode)
{
    if(codes.size() >= MAX_SIZE){
        //overflow
        codes.removeFirst();
    }
    codes.push_back(newCode);
}
