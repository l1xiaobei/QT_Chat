#include "chat.h"
#include "ui_chat.h"

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
    ba.append(timeStr + "  " + ui->nameLine->text() + ": " + ui->sendLine->text());
    socket->write(ba);        //write()需要提供bytearray型的参数，所以要把qstring转换一下
}

void chat::server_slot()
{
    QByteArray ba = socket->readAll();
    ui->chatText->append(QString(ba));
}

void chat::on_imageButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择图片"), "*", tr("Images(*.png, *.jpg, *.jpeg, *.bmp, *.gif, *.webp, *.apng, *.sharpp)"));
//    qDebug() << fileName;
    QImage image(fileName); //创建图片对象
//    image.scaled(110, 48, Qt::KeepAspectRatio,Qt::SmoothTransformation);  //缩放图片，但是保持图像纵横比
    QByteArray imgBy;

    QBuffer imgBuf(&imgBy); //将图片数据写入到imgBy字节数组中
    image.save(&imgBuf, "png");

    QByteArray ba;
    QDateTime dateTime= QDateTime::currentDateTime();//获取系统当前的时间
    QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");//格式化时间
    ba.append(timeStr + "  " + ui->nameLine->text() + ": ");   //!还没有发送给服务器哦!
    QString htmlPath = QString("<img src=\"%1\" weight=\"220\" height=\"96\" />").arg(fileName); //resize图片大小
                                                                                                 //   \ 用于转义 "
    ui->chatText->insertHtml(htmlPath);
}
