#ifndef TESTVIDEO_H
#define TESTVIDEO_H

#include <QObject>
#include <QAbstractVideoSurface>
#include <QDebug>
class QtCameraCapture : public QAbstractVideoSurface
{
    Q_OBJECT
public:


    explicit QtCameraCapture(QObject *parent = 0);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;

    bool present(const QVideoFrame &frame) override;

signals:
    void frameAvailable(QImage frame);

    void frameAvailable1(QVideoFrame &frame);

};
#endif // TESTVIDEO_H
