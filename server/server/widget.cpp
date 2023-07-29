#include "widget.h"
#include "ui_widget.h"

//定义一个枚举消息类型用于区分发送的是图片还是消息
enum MsgType
{
    textMsg,
    imageMsg,
};

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    server = new QTcpServer;
    server->listen(QHostAddress::AnyIPv4, PORT);    //创建服务器对象并监听来自给定端口号（这里是8000），任意IPv4地址的客户端连接请求
    connect(server, &QTcpServer::newConnection, this, &Widget::connect_slot);   //建立newConnection信号与connect_slot槽函数之间的连接
}

Widget::~Widget()
{
    delete ui;
}

void Widget::connect_slot()
{
    QTcpSocket *socket = server->nextPendingConnection();

    clientList.append(socket);  //将连接到的客户端套接字存起来

    ui->ipLine->setText(socket->peerAddress().toString());          //获取客户端的ip，并显示
    ui->portLine->setText(QString::number(socket->peerPort()));     //获取客户端的端口号（不等于服务器的端口），并显示

    class thread *t = new class thread(socket);     //创建线程对象
    t->start();         //开始线程

    connect(t, &thread::send2Widget, this, &Widget::send_slot);
}

void Widget::send_slot(QByteArray ba)
{
//        ui->textEdit->append(QString(ba));

//        //向每一个客户端转发服务器收到的消息
//        for(int i=0; i<clientList.count(); i++)
//        {
//            clientList.at(i)->write(ba);
//        }
    QDataStream in(&ba, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_12);       //设置QDataStream版本
    int typeMsg;
    QString msgInfo;
    QByteArray msgData;
    in>>typeMsg>>msgInfo>>msgData;
    QString msgData_ = msgData.toBase64();
    qDebug() << "接收到的数据大小：" << msgData.size();
    qDebug() << "消息信息：" << msgInfo;
    if(typeMsg == MsgType::imageMsg)
    {
        ui->textEdit->append(msgInfo);
        QString htmlPath = "<img src=\"data:image/png;base64," + msgData_ + "width=\"220\" height=\"96\"" "\">";
        ui->textEdit->insertHtml(htmlPath);
    }

}

void Widget::on_closeButton_clicked()
{
    this->close();
}
