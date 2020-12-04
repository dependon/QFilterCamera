#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QImage>
#include <QVideoWidget>
#include <QCameraImageCapture>
#include "testvideo.h"
#include <QVideoFrame>
#include <QVideoWidget>
#include <QVideoWidgetControl>
#include <QMediaRecorder>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    enum MenuItemId {
        IdNormal,
        IdCool,
        IdWarn,
        IdBEEPS,
        IdOld,
        IdGrayScale,
    };
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void playPic(int,QImage);
private slots:
    void grabImage(QImage img);
    void grabFrame(QVideoFrame &frame);
    void playPicSlots(int id,QImage img);
private:
    Ui::MainWindow *ui;
    QCamera *m_camera{nullptr};
    QtCameraCapture *m_cameraCapture{nullptr};
    QVideoWidget *m_videoWidget{nullptr};
//    QCameraImageCapture *image_capture_{nullptr};
    QImage m_currentImg{nullptr};
    QMediaRecorder *m_mediaRecorder{nullptr};
};

#endif // MAINWINDOW_H
