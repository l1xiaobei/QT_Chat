#include "thread.h"

thread::thread(QTcpSocket *s)
{
    socket = s;
}

void thread::run()
{
    connect(socket, &QTcpSocket::readyRead, this, &thread::run_slots);
}

void thread::run_slots()
{
    QByteArray ba = socket->readAll();
    emit send2Widget(ba);   //发送信号

    //向客户端发送回去消息
    socket->write(ba);
}
