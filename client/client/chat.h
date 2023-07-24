#ifndef CHAT_H
#define CHAT_H

#include <QWidget>
#include <QTcpSocket>
#include <QDateTime>
#include <QFileDialog>
#include <QBuffer>
#include <QDebug>

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
