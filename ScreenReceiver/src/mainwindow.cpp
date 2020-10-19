#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    FunctionTransfer::init(QThread::currentThreadId());

    av_register_all();
    avformat_network_init();
    avdevice_register_all();

    mRtpReceiver = new RtpReceiver();
    mRtpReceiver->setVideoCallBack(this);
//    mRtpReceiver->startReceive();

    connect(ui->pushButton_start, &QPushButton::clicked, this, &MainWindow::slotBtnClicked);
    connect(ui->pushButton_stop,  &QPushButton::clicked, this, &MainWindow::slotBtnClicked);

    ui->pushButton_start->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);

    setWindowTitle(QStringLiteral("屏幕共享-RTP收流端"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

///显示视频数据，此函数不宜做耗时操作，否则会影响播放的流畅性。
void MainWindow::onDisplayVideo(std::shared_ptr<VideoFrame> videoFrame, int frameNum)
{
    FunctionTransfer::runInMainThread([=]
    {
        ui->label_frameNum->setText(QString("%1").arg(frameNum));
        ui->widget_videoPlayer->inputOneFrame(videoFrame);
    });
}

void MainWindow::slotBtnClicked(bool isChecked)
{
    if (QObject::sender() == ui->pushButton_start)
    {
        mRtpReceiver->startReceive(ui->lineEdit_port->text().toInt());
        ui->pushButton_start->setEnabled(false);
        ui->pushButton_stop->setEnabled(true);
    }
    else if (QObject::sender() == ui->pushButton_stop)
    {
        mRtpReceiver->stopReceive();
        ui->pushButton_start->setEnabled(true);
        ui->pushButton_stop->setEnabled(false);
    }

}

