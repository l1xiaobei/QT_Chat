#ifndef THREAD_H
#define THREAD_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QImage>

class thread : public QThread
{
    Q_OBJECT
public:
    explicit thread(QTcpSocket *s);
    void run();
private:
    QTcpSocket *socket;

private slots:
    void run_slots();

signals:
    void send2Widget(QByteArray ba);

};

#endif // THREAD_H
