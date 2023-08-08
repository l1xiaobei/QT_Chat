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
        emit send2Widget(socket);   //发送信号
}
