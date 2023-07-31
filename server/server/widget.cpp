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

    clientList.append(socket);  //将连接到的客户端套接字存起来////////////////////////

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
///////////////////////////////////////////
//    QDataStream in(&ba, QIODevice::ReadOnly);
//    in.setVersion(QDataStream::Qt_5_12);       //设置QDataStream版本
//    int typeMsg;
//    QString msgInfo;
//    QByteArray msgData;
//    in>>typeMsg>>msgInfo>>msgData;
//    QString msgData_ = msgData.toBase64();
//    qDebug() << "接收到的数据大小：" << msgData.size();
//    qDebug() << "消息信息：" << msgInfo;
//    if(typeMsg == MsgType::imageMsg)
//    {
//        ui->textEdit->append(msgInfo);
//        QString htmlPath = "<img src=\"data:image/png;base64," + msgData_ + "width=\"220\" height=\"96\"" "\">";
//        ui->textEdit->insertHtml(htmlPath);
//    }
//    const int CHUNK_SIZE = 1024;
//    QMap<int, QByteArray> dataMap;
//    QByteArray receivedData;
//    while (1)
//    {
//        // 将新接收的数据添加到已接收数据的末尾
//        receivedData.append(ba);

//        // 检查是否接收到了完整的数据块
//        while (receivedData.size() >= sizeof(int) + CHUNK_SIZE) {
//        // 提取序列号和数据块
//        int sequenceNumber = 0;
//        memcpy(&sequenceNumber, receivedData.constData(), sizeof(int));
//        QByteArray chunk = receivedData.mid(sizeof(int), CHUNK_SIZE);

//        // 将数据块存储到映射表中
//        dataMap[sequenceNumber] = chunk;

//        // 从已接收数据中移除已处理的序列号和数据块
//        receivedData.remove(0, sizeof(int) + CHUNK_SIZE);
//        }

//        // 组装完整的数据
//        int nextSequenceNumber = 0;
//        while (dataMap.contains(nextSequenceNumber)) {

//        QByteArray ba_ = dataMap[nextSequenceNumber];
//        QDataStream in(ba);
//        in.setVersion(QDataStream::Qt_5_12);       //设置QDataStream版本
//        int typeMsg;
//        QString msgInfo;
//        QByteArray msgData;
//        in>>typeMsg>>msgInfo>>msgData;
//        QString msgData_ = msgData.toBase64();
//        qDebug() << "接收到的数据大小：" << msgData.size();
//        qDebug() << "消息信息：" << msgInfo;
//        if(typeMsg == MsgType::imageMsg)
//        {
//            ui->textEdit->append(msgInfo);
//            QString htmlPath = "<img src=\"data:image/png;base64," + msgData_ + "width=\"220\" height=\"96\"" "\">";
//            ui->textEdit->insertHtml(htmlPath);
//        }
//        dataMap.remove(nextSequenceNumber);
//        nextSequenceNumber++;
//        }
//    }
//////////////////////////////////////////////////
    QDataStream in(&ba, QIODevice::ReadOnly);
    MsgType typeMsg;
    int totalSize, chunkLength;
    QByteArray userInfo;
    QByteArray chunk;
    QByteArray imageData;
    in>>typeMsg>>totalSize>>chunkLength>>userInfo>>chunk;
    if(typeMsg == MsgType::imageMsg)
    {
        imageData.append(chunk);
        int receiveLength = chunkLength;
        while(receiveLength<totalSize)
        {
            MsgType typeMsg_;
            QByteArray userInfo_;
            int chunkLength_, totalSize_;
            QByteArray imageData_;
            QByteArray chunk_;//其实这些没必要保持一直，可以加个后缀tmp
            in>>typeMsg_>>totalSize_>>chunkLength_>>userInfo_>>chunk_;    // 等待足够的数据到达
            receiveLength += chunkLength_;
            imageData.append(chunk_);
        }
    ui->textEdit->append(userInfo);
    qDebug() << userInfo;
    QString imageBase64 = imageData.toBase64();
    ui->textEdit->setText(imageBase64);
    qDebug()<<imageBase64;
    qDebug()<<receiveLength;
    QImage base64ToImage(QString base64Str);//函数原型声明
    QImage image = base64ToImage(imageBase64);
    QPixmap pixmap = QPixmap::fromImage(image);
    ui->label_2->setPixmap(pixmap);
    imageData.clear();
    }
}

void Widget::on_closeButton_clicked()
{
    this->close();
}

QImage base64ToImage(QString base64Str)
{
    QImage image;
    image.loadFromData(QByteArray::fromBase64(base64Str.toLocal8Bit()));
    return image;
}
