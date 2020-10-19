#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "RtpReceiver/rtpreceiver.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public VideoCallBack
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;

    RtpReceiver *mRtpReceiver;

protected:
    ///播放视频，此函数不宜做耗时操作，否则会影响播放的流畅性。
    void onDisplayVideo(VideoFramePtr videoFrame, int frameNum);

private slots:
    void slotBtnClicked(bool isChecked);

};

#endif // MAINWINDOW_H
