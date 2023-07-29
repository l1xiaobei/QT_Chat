#include "chat.h"
#include "ui_chat.h"

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
    QByteArray ba;
    QDateTime dateTime= QDateTime::currentDateTime();//获取系统当前的时间
    QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");//格式化时间
    ba.append(timeStr + "  " + ui->nameLine->text() + ": " + "\n" + ui->sendLine->text());
    socket->write(ba);        //write()需要提供bytearray型的参数，所以要把qstring转换一下
    ui->sendLine->clear();
}

void chat::server_slot()
{
    QByteArray ba = socket->readAll();
    ui->chatText->append(QString(ba));
}

void chat::on_imageButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择图片"), "*", tr("Images(*.png, *.jpg, *.jpeg, *.bmp, *.gif, *.webp, *.apng, *.sharpp)"));
    if(fileName.isEmpty()!=true)//得先确保选择图片了
    {
        QImage image(fileName); //创建图片对象
        image.scaled(220, 96, Qt::KeepAspectRatio,Qt::FastTransformation);  //缩放图片，但是保持图像纵横比
        QByteArray imgBy;
        QBuffer imgBuf(&imgBy); //将图片数据写入到imgBy字节数组中
        image.save(&imgBuf, "png");

        //-------------------------
        //直接在本地显示图片
//        QByteArray ba;
//        QDateTime dateTime= QDateTime::currentDateTime();//获取系统当前的时间
//        QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");//格式化时间
//        ba.append(timeStr + "  " + ui->nameLine->text() + ": " + "\n");   //!还没有发送给服务器哦!
//        QString htmlPath = QString("<img src=\"%1\" weight=\"220\" height=\"96\" />").arg(fileName); //resize图片大小
//        ui->chatText->append(ba);                                                                                         //   \ 用于转义 "
//        ui->chatText->insertHtml(htmlPath);
        //-------------------------

        //将图片转换为 base64 格式，并构造富文本标签显示图片
//        QString base64Image = imgBy.toBase64();
//        QString htmlPath = "<img src=\"data:image/png;base64," + base64Image + "width=\"220\" height=\"96\"" "\">";
//        QByteArray ba;
//        QDateTime dateTime= QDateTime::currentDateTime();//获取系统当前的时间//这个时间差不多是接收到的时间而不是发送的时间
//        QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");//格式化时间
//        ba.append(timeStr + "  " + ui->nameLine->text() + ": " + "\n");
//        ui->chatText->append(ba);                                                                                         //   \ 用于转义 "
//        ui->chatText->insertHtml(htmlPath);
//        socket->write(imgBy);

        //好像不太对，再换种传送方式，利用QDataStream
        QByteArray infoBlock;
        QDataStream out(&infoBlock, QIODevice::WriteOnly);   //将out与infoBlock关联起来
        out.setVersion(QDataStream::Qt_5_12);       //设置QDataStream版本

        int typeMsg = MsgType::imageMsg;        //设置发送的消息类型是图片类型

        QDateTime dateTime= QDateTime::currentDateTime();//获取系统当前的时间//这个时间差不多是接收到的时间而不是发送的时间
        QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");//格式化时间

//        QString base64Image = imgBy.toBase64();    //设置发送的图片信息

        out<<typeMsg<<timeStr<<imgBy;     //将消息全部封装进infoBlock
        socket->write(infoBlock);
    }
}
