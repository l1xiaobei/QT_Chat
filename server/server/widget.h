#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

#include "thread.h"

#define PORT 8000

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_closeButton_clicked();
    void connect_slot();
    void send_slot(QTcpSocket *socket);

private:
    Ui::Widget *ui;
    QTcpServer *server;
    bool isFile = false;
    QFile file;
    qint64 recvSize = 0;
    qint64 fileSize = 0;
    QString fileHead;

    QList <QTcpSocket*> clientList; //连接到服务器的客户端列表
};
#endif // WIDGET_H
