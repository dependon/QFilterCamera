#ifndef FRAMETOIMAGE_H
#define FRAMETOIMAGE_H

#include <QVideoFrame>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <math.h>
class frameToImage
{
public:
    frameToImage();
    static QImage frameFrom(QVideoFrame &frame);
    static int Bound(int range_left,int data,int range_right);
    static void InverseColorImage(QImage *img,QImage *imgCopy);
    static void QImageD_RunBEEPSHorizontalVertical(QImage *img,QImage *imgCopy,double spatialDecay=0.02,double photometricStandardDeviation=10);
    static void oldImage(QImage *img, QImage *imgCopy);
    static void GrayScaleImage(QImage *img,QImage *imgCopy);
    static void warnImage(QImage *img,QImage *imgCopy,int index=30);
    static void coolImage(QImage *img,QImage *imgCopy,int index=30);

};

#endif // FRAMETOIMAGE_H
