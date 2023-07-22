#include "chat.h"
#include "ui_chat.h"

chat::chat(QTcpSocket *s,QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::chat)
{
    ui->setupUi(this);
    socket = s;

    ui->nameLine->setText(name);
}

chat::~chat()
{
    delete ui;
}

void chat::on_deleteButton_clicked()
{
    ui->sendLine->clear();
}

void chat::on_sendButton_clicked()
{
    QByteArray ba;
    ba.append(ui->nameLine->text() + ": " + ui->sendLine->text());
    socket->write(ba);        //write()需要提供bytearray型的参数，所以要把qstring转换一下
}
