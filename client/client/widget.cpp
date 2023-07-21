#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    socket = new QTcpSocket;
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_loginButton_clicked()
{
    QString ip = ui->ipLine->text();
    QString port = ui->portLine->text();        //获取ip，port

    socket->connectToHost(QHostAddress(ip), port.toShort());    //连接到服务器

    connect(socket, &QTcpSocket::connected, [this](){
       QMessageBox::information(this, "连接状态", "连接成功");
       this->hide();

       chat *c = new chat(socket);      //在堆空间里面创建chat对象防止被杀掉
       c->show();

    });

    connect(socket, &QTcpSocket::disconnected, [this](){
       QMessageBox::information(this, "连接状态", "连接异常");
    });
}

void Widget::on_cancelButton_clicked()
{
    this->close();  //关闭窗口
}

