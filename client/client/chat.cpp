#include "chat.h"
#include "ui_chat.h"

QString fileName_global;

//定义一个枚举消息类型用于区分发送的是图片还是消息
enum MsgType
{
    textMsg,
    imageMsg,
};

chat::chat(QTcpSocket *s,QString name, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::chat)
{
    ui->setupUi(this);
    socket = s;

    //定时器设置
    timer = new QTimer(this);
    //connect(timer, &QTimer::timeout, this, &chat::sendImage_slot);
    //connect(timer, SIGNAL(timeout()), this, SLOT(sendImage_slot()));
    connect(timer, &QTimer::timeout, [=]()
        {
            this->timer->stop();
            sendImage_slot();
        });

    ui->nameLine->setText(name);

    //接受服务器发送回的消息
    connect(socket, &QTcpSocket::readyRead, this, &chat::server_slot);


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
    QByteArray ba = "head#text#";
    QDateTime dateTime= QDateTime::currentDateTime();//获取系统当前的时间
    QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");//格式化时间
    ba.append(timeStr + "  " + ui->nameLine->text() + ": " + "\n" );
    socket->write(ba);        //write()需要提供bytearray型的参数，所以要把qstring转换一下
    ui->sendLine->clear();
}

void chat::server_slot()
{
    QByteArray ba = socket->readAll();
    QString flag = QString(ba).section("#", 1, 1);

    //==================处理文本信息==================//
    if(flag == "text")
    {
        QString text = QString(ba).section("#", 2, 2);
        ui->chatText->append(text);
    }

    //==================处理图片信息==================//
    if(isFile == false && ba.isEmpty() != true && flag == "image" && recvSize == 0 )//第一次接收，因此是文件头
    {
        isFile = true;
        qDebug() << "test2";
        //获取文件信息
        QString fileName = QString(ba).section("#", 2, 2);
        qDebug() << "ba:" << ba << endl;

        fileSize = QString(ba).section("#", 3, 3).toInt();
        qDebug() << fileName << fileSize;


        client_recv_file.setFileName("/recvImg.jpg");

        if(false == client_recv_file.open(QIODevice::WriteOnly))//如果没能在这个地址成功写入文件，则报错
        {
            fileName = "";
            fileSize = 0;
            recvSize = 0;
            isFile = false;
            QMessageBox::warning(this, "警告", "创建文件失败");
            return;
        }

        qDebug() << "正在接收文件---->" << fileName << endl << isFile;
        return;//return防止程序继续顺次执行下去，把文件头当成图片数据了
    }
    if(isFile == true && ba.isEmpty() != true)//说明是图片的文件数据
    {
        qint64 len = client_recv_file.write(ba);
        recvSize += len ;
        qDebug() << "len:"<<len<<"   recvSize:"<<recvSize<<"   fileSize:"<<fileSize<<endl<<"进度：" << recvSize*100/fileSize <<"%" <<endl;
        if(recvSize == fileSize)
        {
            qDebug() <<"接收完毕"<< recvSize ;
            //收尾工作
            isFile = false;
            recvSize = 0;
            fileSize = 0;

            //图片显示
            QImage *img = new QImage;
            img->load("/recvImg.jpg");
            QByteArray imgBy;
            QBuffer imgBuf(&imgBy);
            img->save(&imgBuf, "jpeg");//将图片数据写入到imgBy字节数组中
            QString imgBase64 = imgBy.toBase64();
            QString htmlPath = "<img src=\"data:image/png;base64," + imgBase64 + "width=\"220\" height=\"96\"" "\">";
            ui->chatText->insertHtml(htmlPath);
        }
    }
}

void chat::on_imageButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择图片"), "*", tr("Images(*.png, *.jpg, *.jpeg, *.bmp, *.gif, *.webp, *.apng, *.sharpp)"));
    fileName_global = fileName;
    qDebug() << "fileName" << fileName;

    if(fileName.isEmpty()!=true)//得先确保选择图片了
    {
        QFile file(fileName); //创建图片对象
        if(file.open(QIODevice::ReadOnly)){
        qint64 fileSize = file.size();
        qDebug() << "fileSize" << fileSize;

        QDateTime dateTime= QDateTime::currentDateTime();//获取系统当前的时间//这个时间差不多是接收到的时间而不是发送的时间
        QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");//格式化时间
        QString userInfo;
        userInfo.append(timeStr + "  " + ui->nameLine->text() + ": " + "\n" + ui->sendLine->text());//发送用户的信息

        //自定义文件头   head#图片#文件名#文件大小 （%1 %2 都是占位符）
        QString imgFlag = "image";
        QString buf = QString("head#%1#%2#%3#%4").arg(imgFlag).arg(fileName).arg(fileSize).arg(userInfo);

        //发送文件头，toUtf8()将QString转换为UTF-8编码，data()则返回原始数据的指针。socket->write方法会返回
        //写入数据的字节数，最终存储在len变量中。qint64是64位的有符号整数。
        qint64 len = socket->write(buf.toUtf8().data());
        socket->waitForBytesWritten();	//等待数据发送完毕
        file.close();


        if(len > 0)
        {
            qDebug() << "timer qidong!";
            //使用定时器延时10ms再发送文件数据 防止粘包
            timer->start(100);


        }
    }

    }
}

//逐帧发送的函数实现好像在定时器里？
void chat::sendImage_slot()
{
    QFile file_(fileName_global); //创建图片对象
    qDebug() << fileName_global;
    if(file_.open(QIODevice::ReadOnly)) //报device not open了 是不是这个原因
    {
    qint64 fileSize = file_.size();
    qint64 chunkSize = 0;
    qint64 sendSize = 0;
    qDebug() << "怎么发不过去捏" << fileSize << sendSize ;
    do{

    //每次发送4kb大小的数据，如果剩余的数据不足4kb，就全部发过去
        char buf[4*1024] = {0};

        chunkSize = 0;
        qDebug() << "发了吗" << endl ;
        chunkSize = file_.read(buf, sizeof(buf)); //读数据
        chunkSize = socket->write(buf, chunkSize); //发数据
        qDebug() << "如发:"<< endl << "CHUNK_SIZE:"<<chunkSize <<"SEND_SIZE:"<< sendSize;
        sendSize += chunkSize; //更新已发送的文件大小
    }while(chunkSize > 0);

    if(sendSize == fileSize)
    {
       qDebug() << "send success!@!";
       file_.close();
    }
    }
    else
        qDebug() << "文件打开失败";

}


