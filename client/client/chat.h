#ifndef CHAT_H
#define CHAT_H

#include <QWidget>
#include <QTcpSocket>
#include <QDateTime>
#include <QFileDialog>
#include <QBuffer>
#include <QDebug>
#include <QImage>
#include <QThread> //线程延时时用到
#include <iomanip>  //设置小数显示位数时用到

namespace Ui {
class chat;
}

class chat : public QWidget
{
    Q_OBJECT

public:
    explicit chat(QTcpSocket *s, QString name, QWidget *parent = nullptr);
    ~chat();

private slots:
    void on_deleteButton_clicked();

    void on_sendButton_clicked();

    void server_slot();

    void on_imageButton_clicked();

private:
    Ui::chat *ui;
    QTcpSocket *socket;
};

#endif // CHAT_H
