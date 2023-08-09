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


void Widget::send_slot(QTcpSocket *socket)
{
    QByteArray ba = socket->readAll();
    qDebug() << "test1";
    qDebug()<<isFile<<ba.isEmpty()<<recvSize;
    QString flag = QString(ba).section("#", 1, 1);

    if(QString(ba).section("#", 1, 1) == "image"){fileHead = QString(ba);}
    //==================处理文本信息==================//
    if(flag == "text")
    {
        QString text = QString(ba).section("#", 2, 2);
        ui->textEdit->append(text);

        //向每一个客户端转发服务器收到的文本消息
        for(int i=0; i<clientList.count(); i++)
        {
            clientList.at(i)->write(ba);
        }
    }
    //==================处理图片信息==================//
    if(isFile == false && ba.isEmpty() != true && flag == "image" && recvSize == 0 )//第一次接收，因此是文件头
    {

        //设置图片存放路径
        QString currentPath = QCoreApplication::applicationDirPath();//获取当前程序运行目录
        QString folderName = "server_images";
        QDir dir(currentPath);
        if (!dir.exists(folderName)) {
            if (dir.mkdir(folderName)) {
                qDebug() << "Folder created successfully.";
            } else {
                qDebug() << "Folder creation failed.";
            }
        } else {
            qDebug() << "Folder already exists.";
        }
        QString folderPath = QDir(currentPath).filePath(folderName);//完整文件夹路径
        folderPath = folderPath + QDir::separator() + "server_recv_image.jpg";

        isFile = true;
        qDebug() << "test2";
        //获取文件信息
        QString fileName = QString(ba).section("#", 2, 2);
        qDebug() << "ba:" << ba << endl;

        fileSize = QString(ba).section("#", 3, 3).toInt();
        qDebug() << fileName << fileSize;


        file.setFileName(folderPath);

        if(false == file.open(QIODevice::WriteOnly))//如果没能在这个地址成功写入文件，则报错
        {
            fileName = "";
            fileSize = 0;
            recvSize = 0;
            isFile = false;
            QMessageBox::warning(this, "警告", "创建文件失败");
            return;
        }

        qDebug() << "正在接收文件---->" << fileName << endl << isFile;

        //把userInfo打印出来顺便转发一下
        QString userInfo = QString(ba).section("#", 4, 4);
        ui->textEdit->append(userInfo);
        QByteArray userInfo_buf = "head#text#";
        userInfo_buf.append(userInfo);
        //向每一个客户端转发服务器这条消息
        for(int i=0; i<clientList.count(); i++)
        {
            clientList.at(i)->write(userInfo_buf);
        }

        return;//return防止程序继续顺次执行下去，把文件头当成图片数据了
    }
    if(isFile == true && ba.isEmpty() != true)//说明是图片的文件数据
    {
        qint64 len = file.write(ba);
        recvSize += len ;
        qDebug() << "len:"<<len<<"   recvSize:"<<recvSize<<"   fileSize:"<<fileSize<<endl<<"进度：" << recvSize*100/fileSize <<"%" <<endl;
        if(recvSize == fileSize)
        {
            qDebug() <<"接收完毕"<< recvSize ;
            //收尾工作
            isFile = false;
            recvSize = 0;
            qint64 fileSize_ = fileSize;
            fileSize = 0;


            //图片显示
            QImage *img = new QImage;

            QString currentPath = QCoreApplication::applicationDirPath();//获取当前程序运行目录
            QString folderName = "server_images";
            QDir dir(currentPath);
            QString folderPath = QDir(currentPath).filePath(folderName);//完整文件夹路径
            folderPath = folderPath + QDir::separator() + "server_recv_image.jpg";

            img->load(folderPath);
            file.close();
            QByteArray imgBy;
            QBuffer imgBuf(&imgBy);
            img->save(&imgBuf, "jpeg");//将图片数据写入到imgBy字节数组中
            QString imgBase64 = imgBy.toBase64();
            QString htmlPath = "<img src=\"data:image/png;base64," + imgBase64 + "width=\"220\" height=\"96\"" "\">";
            ui->textEdit->insertHtml(htmlPath);

            //图片向客户端转发：
            //向每一个客户端转发服务器收到的图片消息，先发文件头
            qDebug() << "zhuan发了吗" << endl ;
            for(int i=0; i<clientList.count(); i++)
            {
                file.open(QIODevice::ReadOnly);
                qint64 len = clientList.at(i)->write(fileHead.toUtf8().data());
                clientList.at(i)->waitForBytesWritten();	//等待数据发送完毕
qDebug() << fileHead<<len<<"ruzhuan" << endl ;
                if(len > 0)
                {
                    qint64 chunkSize = 0;
                    qint64 sendSize = 0;
                    do{

                    //每次发送4kb大小的数据，如果剩余的数据不足4kb，就全部发过去
                        char buf[4*1024] = {0};

                        chunkSize = 0;
                        qDebug() << "发了吗" << endl ;
                        chunkSize = file.read(buf, sizeof(buf)); //读数据
                        chunkSize = socket->write(buf, chunkSize); //发数据
                        qDebug() << "如发:"<< endl << "CHUNK_SIZE:"<<chunkSize <<"SEND_SIZE:"<< sendSize;
                        sendSize += chunkSize; //更新已发送的文件大小
                    }while(chunkSize > 0);

                    if(sendSize == fileSize_)
                    {
                       qDebug() << "send success!@!";
                       file.close();
                    }

                    else
                        qDebug() << "文件打开失败";
                }


            }
        }
    }

}
//void Widget::send_slot(QByteArray ba)//不在这里传参了
//void Widget::send_slot(QTcpSocket *socket)
//{
////        ui->textEdit->append(QString(ba));

////        //向每一个客户端转发服务器收到的消息
////        for(int i=0; i<clientList.count(); i++)
////        {
////            clientList.at(i)->write(ba);
////        }
/////////////////////////////////////////////
////    QDataStream in(&ba, QIODevice::ReadOnly);
////    in.setVersion(QDataStream::Qt_5_12);       //设置QDataStream版本
////    int typeMsg;
////    QString msgInfo;
////    QByteArray msgData;
////    in>>typeMsg>>msgInfo>>msgData;
////    QString msgData_ = msgData.toBase64();
////    qDebug() << "接收到的数据大小：" << msgData.size();
////    qDebug() << "消息信息：" << msgInfo;
////    if(typeMsg == MsgType::imageMsg)
////    {
////        ui->textEdit->append(msgInfo);
////        QString htmlPath = "<img src=\"data:image/png;base64," + msgData_ + "width=\"220\" height=\"96\"" "\">";
////        ui->textEdit->insertHtml(htmlPath);
////    }
////    const int CHUNK_SIZE = 1024;
////    QMap<int, QByteArray> dataMap;
////    QByteArray receivedData;
////    while (1)
////    {
////        // 将新接收的数据添加到已接收数据的末尾
////        receivedData.append(ba);

////        // 检查是否接收到了完整的数据块
////        while (receivedData.size() >= sizeof(int) + CHUNK_SIZE) {
////        // 提取序列号和数据块
////        int sequenceNumber = 0;
////        memcpy(&sequenceNumber, receivedData.constData(), sizeof(int));
////        QByteArray chunk = receivedData.mid(sizeof(int), CHUNK_SIZE);

////        // 将数据块存储到映射表中
////        dataMap[sequenceNumber] = chunk;

////        // 从已接收数据中移除已处理的序列号和数据块
////        receivedData.remove(0, sizeof(int) + CHUNK_SIZE);
////        }

////        // 组装完整的数据
////        int nextSequenceNumber = 0;
////        while (dataMap.contains(nextSequenceNumber)) {

////        QByteArray ba_ = dataMap[nextSequenceNumber];
////        QDataStream in(ba);
////        in.setVersion(QDataStream::Qt_5_12);       //设置QDataStream版本
////        int typeMsg;
////        QString msgInfo;
////        QByteArray msgData;
////        in>>typeMsg>>msgInfo>>msgData;
////        QString msgData_ = msgData.toBase64();
////        qDebug() << "接收到的数据大小：" << msgData.size();
////        qDebug() << "消息信息：" << msgInfo;
////        if(typeMsg == MsgType::imageMsg)
////        {
////            ui->textEdit->append(msgInfo);
////            QString htmlPath = "<img src=\"data:image/png;base64," + msgData_ + "width=\"220\" height=\"96\"" "\">";
////            ui->textEdit->insertHtml(htmlPath);
////        }
////        dataMap.remove(nextSequenceNumber);
////        nextSequenceNumber++;
////        }
////    }
////////////////////////////////////////////////////
////    QDataStream in(&ba, QIODevice::ReadOnly);
////    MsgType typeMsg;
////    int totalSize, chunkLength, blockIndex=0,localIndex=1;
////    QByteArray userInfo;
////    QByteArray chunk;
////    QByteArray imageData;
////    in>>blockIndex>>typeMsg>>totalSize>>chunkLength>>userInfo>>chunk;
////    if(typeMsg == MsgType::imageMsg)
////    {
////        qDebug()<<localIndex<<"|||"<<blockIndex<<chunkLength<<"total:"<<totalSize;
////        blockIndex+=1;localIndex+=1;
////        int receiveLength = chunkLength;
////        imageData.append(chunk);
//////      while(receiveLength<totalSize&&typeMsg==MsgType::imageMsg)//也许并不需要循环
////        {
////                if(localIndex == blockIndex)
////                {
////                    MsgType typeMsg_;
////                    QByteArray userInfo_;
////                    int chunkLength_, totalSize_;
////                    QByteArray imageData_;
////                    QByteArray chunk_;//其实这些没必要保持一直，可以加个后缀tmp
////                    in>>blockIndex>>typeMsg_>>totalSize_>>chunkLength_>>userInfo_>>chunk_;    // 等待足够的数据到达
////                    receiveLength += chunkLength_;
////                    imageData.append(chunk_);
////                    qDebug()<<localIndex<<blockIndex<<chunkLength<<"total:"<<totalSize<<"recv:"<<receiveLength;
////                    localIndex+=1;
////                }
////        }
////        ui->textEdit->append(userInfo);
////        //qDebug() << userInfo;
////        QString imageBase64 = imageData.toBase64();
////        ui->textEdit->setText(imageBase64);
////    }
////    //qDebug()<<imageBase64;
//////    qDebug()<<receiveLength;
//////    QImage base64ToImage(QString base64Str);//函数原型声明
//////    QImage image = base64ToImage(imageBase64);
//////    QPixmap pixmap = QPixmap::fromImage(image);
//////    ui->label_2->setPixmap(pixmap);
//////    imageData.clear();
///////////////////////////////////////
////    QDataStream in(&ba, QIODevice::ReadOnly);
////    int blockIndex = 0, totalSize, chunkLength, recvLength = 0;
////    MsgType typeMsg;
////    QByteArray chunk, imageData, userInfo;
////    int localIndex = 0;
////    if (localIndex == blockIndex)
////    {
////        in>>blockIndex>>typeMsg>>totalSize>>chunkLength>>userInfo>>chunk;
////        recvLength += chunkLength;
////        imageData.append(chunk);
////        localIndex += 1;
////        qDebug()<<localIndex<<blockIndex<<chunkLength<<"total:"<<totalSize<<"recv:"<<recvLength;
////    }
////    QDataStream in(socket);
////    int blockIndex, totalSize, chunkLength;
////    int recvLength = 0;
////    MsgType typeMsg;
////    QByteArray userInfo, chunk, imageData;
////    in>>blockIndex>>typeMsg>>totalSize>>chunkLength>>userInfo>>chunk;
////    if(blockIndex == 1 && imageData.isEmpty()==true)
////    {
////        imageData.append(chunk);
////        recvLength += chunkLength;
////    }
////    else if(blockIndex > 1)
////    {
////        imageData.append(chunk);
////        recvLength += chunkLength;
////    }
////    else if(blockIndex == 1 && imageData.isEmpty()!= true)
////    {
////        QString imageBase64 = imageData.toBase64();
////        imageData.clear();
////        chunkLength = 0;
////        /*显示实现*/
////    }
////    qDebug()<<blockIndex<<recvLength;
//}

void Widget::on_closeButton_clicked()
{
    this->close();
}

//QImage base64ToImage(QString base64Str)
//{
//    QImage image;
//    image.loadFromData(QByteArray::fromBase64(base64Str.toLocal8Bit()));
//    return image;
//}
