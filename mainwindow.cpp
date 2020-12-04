#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCameraInfo>
#include <QImage>
#include "frametoimage.h"
#include <QThread>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_camera = new QCamera(QCameraInfo::defaultCamera());
    m_cameraCapture = new QtCameraCapture();
    m_camera->setViewfinder(m_cameraCapture);
    m_camera->setCaptureMode(QCamera::CaptureVideo);
//    connect(m_cameraCapture, SIGNAL(frameAvailable(QImage)), this, SLOT(grabImage(QImage)));
    connect(m_cameraCapture,&QtCameraCapture::frameAvailable1,this,&MainWindow::grabFrame);
    m_mediaRecorder=new QMediaRecorder(m_camera);
    QVideoEncoderSettings videosetting = m_mediaRecorder->videoSettings();
    videosetting.setResolution(QSize(640,480));

    m_mediaRecorder->setVideoSettings (videosetting);
    m_camera->start();
    connect(this,&MainWindow::playPic,this,&MainWindow::playPicSlots);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::grabImage(QImage img)
{
    ui->label_normal->setPixmap(QPixmap::fromImage(img));
    update();
}

void MainWindow::grabFrame(QVideoFrame &frame)
{
    qDebug() << "=============ProcessFrame===============";
        qDebug() << "width : " << frame.width() << " height : " << frame.height();
        qDebug() << "start time : " << frame.startTime()/1000 << "ms";
        qDebug() << "end time : " << frame.endTime()/1000 << "ms";
        qDebug() << "pixelFormat :" << frame.pixelFormat();

        QVideoFrame::PixelFormat pixelFormat=frame.pixelFormat();
       QImage::Format xx= QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
    frame.map(QAbstractVideoBuffer::ReadOnly);
//qt_imageFromVideoFrame
    QImage recvImage(frame.bits(), frame.width(), frame.height(), QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()));
   int x=frame.mappedBytes();


    if(recvImage.isNull())
    {
        recvImage = frameToImage::frameFrom(frame);
    }
    m_currentImg=recvImage;

    //        frameToImage::InverseColorImage(&recvImage,&recvImage);
    QThread *th1 = QThread::create([ = ]() {
        QImage img=recvImage;
        img=img.scaled(640,360);
        frameToImage::QImageD_RunBEEPSHorizontalVertical(&img,&img);
        emit playPic(IdBEEPS,img);
    });

    QThread *th2 = QThread::create([ = ]() {
        QImage img=recvImage;
        img=img.scaled(640,360);
        frameToImage::warnImage(&img,&img);
        emit playPic(IdWarn,img);
    });

    QThread *th3 = QThread::create([ = ]() {
        QImage img=recvImage;
        img=img.scaled(640,360);
        frameToImage::coolImage(&img,&img);
        emit playPic(IdCool,img);
    });
    QThread *th4 = QThread::create([ = ]() {
        QImage img=recvImage;
        img=img.scaled(640,360);
        frameToImage::oldImage(&img,&img);
        emit playPic(IdOld,img);
    });
    QThread *th5 = QThread::create([ = ]() {
        QImage img=recvImage;
        img=img.scaled(640,360);
        frameToImage::InverseColorImage(&img,&img);
        emit playPic(IdGrayScale,img);
    });
    QThread *th0 = QThread::create([ = ]() {
        QImage img=recvImage;
        img=img.scaled(640,360);
        emit playPic(IdNormal,img);
    });
    th0->start();
    th1->start();
    th2->start();
    th3->start();
    th4->start();
    th5->start();



    frame.unmap();


}

void MainWindow::playPicSlots(int id, QImage img)
{
    switch (MenuItemId(id)) {
    case IdNormal :
        ui->label_normal->setPixmap(QPixmap::fromImage(img));
        update();
        break;
    case IdCool :
        ui->label_cool->setPixmap(QPixmap::fromImage(img));
        update();
        break;
    case IdWarn :
        ui->label_warn->setPixmap(QPixmap::fromImage(img));
        update();
        break;
    case IdBEEPS :
        ui->label_mopi->setPixmap(QPixmap::fromImage(img));
        update();
        break;
    case IdOld :
        ui->label_old->setPixmap(QPixmap::fromImage(img));
        update();
        break;
    case IdGrayScale :
        ui->label_inverse->setPixmap(QPixmap::fromImage(img));
        update();
        break;
    }
}
