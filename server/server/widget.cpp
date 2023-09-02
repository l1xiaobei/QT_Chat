#include "widget.h"
#include "ui_widget.h"

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

    class thread *t = new class thread(socket);     //创建线程对象
    t->start();         //开始线程

    connect(t, &thread::send2Widget, this, &Widget::send_slot);

    //显示连接到服务器的客户端
    ui->clientText->clear();
    for(int i=0; i<clientList.count(); i++)
    {
        if (clientList.at(i)->state() == QAbstractSocket::ConnectedState)
        {
            ui->clientText->append(QString("|--client%1 ip:%2 port:%3").arg(i+1).arg(clientList.at(i)
                                                                                     ->peerAddress().toString()).arg(clientList.at(i)->peerPort()));
        }

    }
}

void Widget::send_slot(QTcpSocket *socket)
{

    QByteArray ba = socket->readAll();
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

void Widget::on_closeButton_clicked()
{
    this->close();
}
