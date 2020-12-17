// clientwidget.cpp
// 客户端cpp文件
// Jackkii 2020/12/16


#include "clientwidget.h"
#include "ui_clientwidget.h"

#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

Clientwidget::Clientwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Clientwidget)
{
    ui->setupUi(this);

    //初始化
    isStart = true; //最先发送头部信息
    ui->progressBar->setValue(0); //设置初始值为0
    setWindowTitle("客户端");
    ui->ButtonConnect->setEnabled(false);
    ui->ButtonClose->setEnabled(false);

    //设置按钮分组
    btnTcpOrUdp = new QButtonGroup(this);
    btnTcpOrUdp->addButton(ui->radioButtonTcp, 0);
    btnTcpOrUdp->addButton(ui->radioButtonUdp, 1);
    ui->radioButtonTcp->setChecked(true);

    btnSinOrGroup = new QButtonGroup(this);
    btnSinOrGroup->addButton(ui->radioButtonSin, 0);
    btnSinOrGroup->addButton(ui->radioButtonGroup, 1);
    ui->radioButtonGroup->setCheckable(false);
    ui->radioButtonSin->setCheckable(false);

    connect(ui->radioButtonTcp,&QAbstractButton::clicked,[=](){
        //禁用udp模式选项
        ui->radioButtonSin->setChecked(false);
        ui->radioButtonGroup->setChecked(false);
        ui->radioButtonGroup->setCheckable(false);
        ui->radioButtonSin->setCheckable(false);
    });
    connect(ui->radioButtonUdp,&QAbstractButton::clicked,[=](){
        //开启udp模式选项
        ui->radioButtonSin->setChecked(true);
        ui->radioButtonGroup->setCheckable(true);
        ui->radioButtonSin->setCheckable(true);
    });


}

Clientwidget::~Clientwidget()
{
    delete ui;
}

void Clientwidget::on_ButtonConnect_clicked()
{
    //获取服务器的IP地址和端口
    QString ip = ui->lineEditIP->text();
    quint16 port = ui->lineEditPort->text().toInt();

    //主动和服务器连接
    tcpSocket->connectToHost(QHostAddress(ip), port);
    isStart = true;

    connect(tcpSocket, &QTcpSocket::connected,[=](){
        QString str = QString("[%1:%2] 成功连接服务器").arg(ip).arg(port);
        ui->textEdit->append(str);
        ui->ButtonConnect->setEnabled(false);   //禁用按钮
    });

}

void Clientwidget::on_ButtonClose_clicked()
{

    //主动和对方断开连接
    tcpSocket->disconnectFromHost();
    tcpSocket->close();


    ui->textEdit->append("与服务器断开连接");
    ui->ButtonConnect->setEnabled(true);   //启动按钮
    ui->pushButtonYes->setEnabled(true);
}

void Clientwidget::on_pushButtonYes_clicked()
{
    ui->pushButtonYes->setEnabled(false);
    ui->ButtonConnect->setEnabled(true);
    switch(btnTcpOrUdp->checkedId())
    {
        case 0:   //选择使用tcp协议

            ui->textEdit->append("选择tcp协议");
            //通信套接字
            tcpSocket = new QTcpSocket(this);

            connect(tcpSocket, &QTcpSocket::readyRead,[=](){
                //取出接收的内容
                QByteArray buf = tcpSocket->readAll();

                if(isStart == true)  //解析头部信息
                {
                    isStart = false;
                    fileName = QString(buf).section("##", 0, 0);
                    fileSize = QString(buf).section("##", 1, 1).toInt();

                    recvSize = 0;   //已经接收文件大小

                    //只写方式方式，打开文件
                    file.setFileName(fileName);
                    bool isOk = file.open(QIODevice::WriteOnly);

                    if(isOk == false)   //打开失败
                    {
                        qDebug() << "WriteOnly error! clientwidget.cpp";

                        tcpSocket->disconnectFromHost(); //断开连接
                        tcpSocket->close(); //关闭套接字

                        return; //如果打开文件失败，中断函数
                    }
                    //打开文件成功,开始接收文件
                    ui->textEdit->append("文件开始接收");

                    //设置进度条
                    ui->progressBar->setMinimum(0); //最小值
                    ui->progressBar->setMaximum(fileSize/1024); //最大值
                    ui->progressBar->setValue(0); //当前值
                }
                else //读取文件数据
                {
                    qint64 len = file.write(buf);
                    if(len >0) //接收数据大于0
                    {
                        recvSize += len; //累计接收大小
                        qDebug() << len;
                    }

                    //更新进度条
                    ui->progressBar->setValue(recvSize/1024);

                    if(recvSize == fileSize) //文件接收完毕
                    {

                        //先给服务发送(接收文件完成的信息)
                        tcpSocket->write("file done");

                        ui->textEdit->append("文件接收完成");
                        QString str = QString("成功接收文件：[%1:%2kb]").arg(fileName).arg(fileSize/1024);
                        QMessageBox::information(this, "完成", str);
                        file.close(); //关闭文件
                        //断开连接
                        tcpSocket->disconnectFromHost();
                        tcpSocket->close();
                    }
                }
            });
            ui->ButtonClose->setEnabled(true);
            break;

        case 1:   //选择udp协议
            switch(btnSinOrGroup->checkedId()){
                case 0:


                //待更新


                    ui->textEdit->append("选择udp协议单播模式");

                    break;
                case 1:

                //待更新


                    ui->textEdit->append("选择udp协议组播模式");

                    break;

            }

            break;

    }

}
