// widget.cpp
// 服务端cpp文件
// Jackkii 2020/12/16

#include "widget.h"
#include "ui_widget.h"

#include <QFileDialog>
#include <QDebug>
#include <QFileInfo>
#include <QHostInfo>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //初始化
    setWindowTitle("服务端端口为：8888");
    ui->progressBar->setValue(0);

    //未连接成功，加载和发送按钮都不能按
    ui->ButtonAdd->setEnabled(false);
    ui->ButtonSend->setEnabled(false);
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

Widget::~Widget()
{
    delete ui;
}


//加载文件按钮
void Widget::on_ButtonAdd_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,"open","C:\\Users\\chenj\\Desktop\\+网络收发测试CMultiCast\\");
    if(filePath.isEmpty() == false){    //选择文件路径有效
        fileName.clear();
        fileSize = 0;

        //获取文件信息
        QFileInfo info(filePath);
        fileName = info.fileName();
        fileSize = info.size();

        //初始化发送文件的大小
        sendSize = 0;

        //以只读方式打开文件
        file.setFileName(filePath);
        bool isOk = file.open(QIODevice::ReadOnly);
        if(isOk == false)
        {
            qDebug() << "只读方式打开文件失败 Widget.cpp 80";
        }

        //打印下文件路径
        ui->textEdit->append(filePath);

        ui->ButtonAdd->setEnabled(false);
        ui->ButtonSend->setEnabled(true);
    }
}

//发送数据按钮
void Widget::on_ButtonSend_clicked()
{
    ui->ButtonAdd->setEnabled(false);

    //先发送文件头信息  文件名##文件大小
    QString head = QString("%1##%2").arg(fileName).arg(fileSize);
    //发送头部信息
    qint64 len = tcpSocket->write(head.toUtf8());  //?
    //如果文件头部信息发送成功，为防止粘包，延时20ms
    if(len > 0){
        timer.start(20);
    }
    else{  //如果文件头部信息发送失败，则报错
        qDebug()<<"头部信息发送失败 Widget.cpp ";
        file.close();
        ui->ButtonAdd->setEnabled(true);
        ui->ButtonSend->setEnabled(false);
    }
}


void Widget::on_ButtonClose_clicked()
{
    if(tcpSocket == NULL)
    {
        return;
    }

    //主动和客户端断开连接
    tcpSocket->disconnectFromHost();
    tcpSocket->close();
    tcpSocket = NULL;


    ui->textEdit->append("与客户端断开连接");
    ui->ButtonAdd->setEnabled(false);
    ui->ButtonSend->setEnabled(false);
    ui->pushButtonYes->setEnabled(true);
}

void Widget::on_ButtonHostinfo_clicked()
{
    //获取主机信息
    QString hostName = QHostInfo::localHostName();
    QHostInfo info = QHostInfo::fromName(hostName);
    QList<QHostAddress> addressLi = info.addresses();

    ui->textEdit->append(hostName);
    foreach(QHostAddress address, addressLi){    //遍历容器
        ui->textEdit->append(address.toString());
    }
}


void Widget::on_pushButtonYes_clicked()
{
    ui->pushButtonYes->setEnabled(false);
    switch(btnTcpOrUdp->checkedId())
    {
        case 0:   //选择使用tcp协议

            ui->textEdit->append("选择tcp协议");

            //开始监听套接字
            tcpServer = new QTcpServer(this);

            //绑定和监听
            tcpServer->listen(QHostAddress::Any, 8888);

            //如果客户端发起连接且成功连接，则服务端监听套接字tcpServer会触发newConnection信号
            connect(tcpServer, &QTcpServer::newConnection,[=](){

                //获得已连接上的客户端的信息，即通信套接字
                tcpSocket = tcpServer->nextPendingConnection();
                //获取对方的ip地址和端口
                QString ip = tcpSocket->peerAddress().toString();
                quint16 port = tcpSocket->peerPort();
                //打印信息
                QString str = QString("[%1:%2] 成功连接 ").arg(ip).arg(port);
                ui->textEdit->append(str);

                //成功连接，加载文件按钮可按
                ui->ButtonAdd->setEnabled(true);
            });


            //头部信息发送完毕，定时器到达定时时间，则可以发送数据
            connect(&timer, &QTimer::timeout,[=](){
                //关闭定时器
                timer.stop();

                //打开数据传送定时器
                timer_data.start(ui->lineEditSendStep->text().toInt());
                //开始发送文件数据
                ui->textEdit->append("开始发送数据文件......");

                //设置进度条
                ui->progressBar->setMinimum(0); //最小值
                ui->progressBar->setMaximum(fileSize/1024); //最大值
                ui->progressBar->setValue(0); //当前值
            });


            connect(&timer_data, &QTimer::timeout,[=]{

                //每次发送数据的大小
                char buf[4*1024] = {0};
                qint64 len = 0;
                bool isOk = 1;

                //往文件中读数据
                len = file.read(buf, sizeof(buf));

                if(len <= 0){
                    timer_data.stop();
                    isOk = 0;
                    //是否发送文件完毕
                    if(sendSize == fileSize){
                        ui->textEdit->append("数据文件发送完毕");
                        file.close();

                        //把客户端断开
                        tcpSocket->disconnectFromHost();
                        tcpSocket->close();
                        ui->ButtonSend->setEnabled(false);
                    }
                }

                if(isOk){
                    //发送数据，读多少，发多少
                    len = tcpSocket->write(buf, len);

                    //发送的数据需要累积
                    sendSize += len;

                    //更新进度条
                    ui->progressBar->setValue(sendSize/1024);
                }
            });

            ui->radioButtonGroup->setCheckable(true);
            ui->radioButtonSin->setCheckable(true);
            ui->ButtonClose->setEnabled(true);
            break;

        case 1:   //选择udp协议
            ui->ButtonConnect->setEnabled(true);
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
