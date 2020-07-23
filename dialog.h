#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "tcpserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void on_btnStart_clicked();

private:
    Ui::Dialog *ui;
    TcpServer tcpServer;
    bool isStarted;
    QHostAddress addr;
    quint16 port;

    void setState(bool isStarted);
};
#endif // DIALOG_H
