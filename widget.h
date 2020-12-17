#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>  //监听套接字
#include <QTcpSocket>  //通信套接字
#include <QFile>
#include <QTimer>
#include <QRadioButton>
#include <QButtonGroup>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_ButtonAdd_clicked();

    void on_ButtonSend_clicked();

    void on_ButtonClose_clicked();

    void on_ButtonHostinfo_clicked();

    void on_pushButtonYes_clicked();

private:
    Ui::Widget *ui;

    QTcpServer *tcpServer;  //监听套接字
    QTcpSocket *tcpSocket;  //通信套接字

    QFile file;    //文件对象
    QString fileName;    //文件名字
    qint64 fileSize;    //文件大小
    qint64 sendSize;    //已发送文件大小

    QTimer timer;  //定时器
    QTimer timer_data;  //定时器

    QButtonGroup *btnTcpOrUdp;  //选择是udp还是tcp协议
    QButtonGroup *btnSinOrGroup;  //选择udp模式（单播or组播）
};

#endif // WIDGET_H
