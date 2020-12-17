// clientwidget.h
// 客户端头文件
// Jackkii 2020/12/16


#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QFile>
#include <QRadioButton>
#include <QButtonGroup>

namespace Ui {
class Clientwidget;
}

class Clientwidget : public QWidget
{
    Q_OBJECT

public:
    explicit Clientwidget(QWidget *parent = 0);
    ~Clientwidget();

private slots:
    void on_ButtonConnect_clicked();

    void on_ButtonClose_clicked();

    void on_pushButtonYes_clicked();

private:
    Ui::Clientwidget *ui;

    QTcpSocket *tcpSocket;    //通信套接字

    QFile file;   //文件对象
    QString fileName;    //文件名字
    qint64 fileSize;     //文件大小
    qint64 recvSize;    //已接收文件大小

    bool isStart;       //标志位，是否为头部信息（为防止TCP粘包而设）

    QButtonGroup *btnTcpOrUdp;  //选择是udp还是tcp协议
    QButtonGroup *btnSinOrGroup;  //选择udp模式（单播or组播）
};

#endif // CLIENTWIDGET_H
