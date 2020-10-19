/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>

#include <QFileDialog>

#ifndef WIN32
#define QStringLiteral QString
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    av_register_all();
    avformat_network_init();
    avdevice_register_all();

    mMediaReader = new MediaReader();

    connect(ui->pushButton_start, &QPushButton::clicked, this, &MainWindow::slotBtnClicked);
    connect(ui->pushButton_stop,  &QPushButton::clicked, this, &MainWindow::slotBtnClicked);

    ui->pushButton_start->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);

    setWindowTitle(QStringLiteral("屏幕共享-RTP推流端"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::startSend()
{
    bool isSucceed = false;
do
{
    if (mMediaReader)
    {
        mMediaReader->stop();
    }

    mMediaReader->setRemoteIp(ui->lineEdit_ip->text().toUtf8().data(), ui->lineEdit_port->text().toInt());
    mMediaReader->setQuantity(ui->comboBox_quantity->currentText().toInt());

    ErroCode code = mMediaReader->init();

    if (code == SUCCEED)
    {
        mMediaReader->start();
    }
    else if (code == VideoOpenFailed)
    {
        QMessageBox::critical(NULL, QStringLiteral("提示"), QStringLiteral("初始化录屏设备失败！"));

        fprintf(stderr, "init device failed! \n");
    }
    else if (code == AudioOpenFailed)
    {
        QMessageBox::critical(NULL, QStringLiteral("提示"), QStringLiteral("初始化录音设备失败！已管理员权限运行插件目录下的reg.bat即可解决。"));

        fprintf(stderr, "init device failed! \n");
    }

    ui->pushButton_start->setEnabled(false);
    ui->pushButton_stop->setEnabled(true);

    isSucceed = true;

    ui->pushButton_start->setEnabled(false);
    ui->pushButton_stop->setEnabled(true);

}while(0);

    return isSucceed;
}

void MainWindow::stopSend()
{
    mMediaReader->stop();
    ui->pushButton_start->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);
}

void MainWindow::slotBtnClicked(bool isChecked)
{
    if (QObject::sender() == ui->pushButton_start)
    {
        startSend();
    }
    else if (QObject::sender() == ui->pushButton_stop)
    {
        stopSend();
    }

}
