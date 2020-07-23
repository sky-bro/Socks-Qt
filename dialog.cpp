#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setWindowTitle("Socks-Qt");
    isStarted = false;
    ui->btnStart->setText("start");
}

Dialog::~Dialog()
{
    delete ui;
}


void Dialog::on_btnStart_clicked()
{
    if (!isStarted) {
        // start server
        addr = QHostAddress(ui->lineEditIP->text());
        port = ui->spinBoxPort->value();
        cout << "starting server @" << addr << port;
        if (tcpServer.listen(addr, port)) {
            cout << this << "server started!";
            setState(true);
        } else {
            cout << this << "failed to start...";
//            setState(false);
        }
    } else {
        // stop server
        tcpServer.close();
        setState(false);
    }
}

void Dialog::setState(bool isStarted)
{
    this->isStarted = isStarted;
    if (isStarted) {
        // server running
        ui->btnStart->setText("stop");
        ui->lineEditIP->setEnabled(false);
        ui->spinBoxPort->setEnabled(false);
    } else {
        // server stopped
        ui->btnStart->setText("start");
        ui->lineEditIP->setEnabled(true);
        ui->spinBoxPort->setEnabled(true);
    }
}
