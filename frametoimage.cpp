#include "frametoimage.h"
#include <QDebug>
#include <qimage.h>
#include <qpair.h>
#include <qsize.h>
#include <qvariant.h>
#include <qvector.h>
#include <qmutex.h>
#include <qabstractvideobuffer.h>
#include <QDebug>
#include "frametoimage.h"
#include "qvideoframeconversionhelper_p.h"

extern void QT_FASTCALL qt_convert_BGRA32_to_ARGB32(const QVideoFrame&, uchar*);
extern void QT_FASTCALL qt_convert_BGR24_to_ARGB32(const QVideoFrame&, uchar*);
extern void QT_FASTCALL qt_convert_BGR565_to_ARGB32(const QVideoFrame&, uchar*);
extern void QT_FASTCALL qt_convert_BGR555_to_ARGB32(const QVideoFrame&, uchar*);
extern void QT_FASTCALL qt_convert_AYUV444_to_ARGB32(const QVideoFrame&, uchar*);
extern void QT_FASTCALL qt_convert_YUV444_to_ARGB32(const QVideoFrame&, uchar*);
extern void QT_FASTCALL qt_convert_YUV420P_to_ARGB32(const QVideoFrame&, uchar*);
extern void QT_FASTCALL qt_convert_YV12_to_ARGB32(const QVideoFrame&, uchar*);
extern void QT_FASTCALL qt_convert_UYVY_to_ARGB32(const QVideoFrame&, uchar*);
extern void QT_FASTCALL qt_convert_YUYV_to_ARGB32(const QVideoFrame&, uchar*);
extern void QT_FASTCALL qt_convert_NV12_to_ARGB32(const QVideoFrame&, uchar*);
extern void QT_FASTCALL qt_convert_NV21_to_ARGB32(const QVideoFrame&, uchar*);

static VideoFrameConvertFunc qConvertFuncs[QVideoFrame::NPixelFormats] = {
    /* Format_Invalid */                nullptr, // Not needed
    /* Format_ARGB32 */                 nullptr, // Not needed
    /* Format_ARGB32_Premultiplied */   nullptr, // Not needed
    /* Format_RGB32 */                  nullptr, // Not needed
    /* Format_RGB24 */                  nullptr, // Not needed
    /* Format_RGB565 */                 nullptr, // Not needed
    /* Format_RGB555 */                 nullptr, // Not needed
    /* Format_ARGB8565_Premultiplied */ nullptr, // Not needed
    /* Format_BGRA32 */                 qt_convert_BGRA32_to_ARGB32,
    /* Format_BGRA32_Premultiplied */   qt_convert_BGRA32_to_ARGB32,
    /* Format_BGR32 */                  qt_convert_BGRA32_to_ARGB32,
    /* Format_BGR24 */                  qt_convert_BGR24_to_ARGB32,
    /* Format_BGR565 */                 qt_convert_BGR565_to_ARGB32,
    /* Format_BGR555 */                 qt_convert_BGR555_to_ARGB32,
    /* Format_BGRA5658_Premultiplied */ nullptr,
    /* Format_AYUV444 */                qt_convert_AYUV444_to_ARGB32,
    /* Format_AYUV444_Premultiplied */  nullptr,
    /* Format_YUV444 */                 qt_convert_YUV444_to_ARGB32,
    /* Format_YUV420P */                qt_convert_YUV420P_to_ARGB32,
    /* Format_YV12 */                   qt_convert_YV12_to_ARGB32,
    /* Format_UYVY */                   qt_convert_UYVY_to_ARGB32,
    /* Format_YUYV */                   qt_convert_YUYV_to_ARGB32,
    /* Format_NV12 */                   qt_convert_NV12_to_ARGB32,
    /* Format_NV21 */                   qt_convert_NV21_to_ARGB32,
    /* Format_IMC1 */                   nullptr,
    /* Format_IMC2 */                   nullptr,
    /* Format_IMC3 */                   nullptr,
    /* Format_IMC4 */                   nullptr,
    /* Format_Y8 */                     nullptr,
    /* Format_Y16 */                    nullptr,
    /* Format_Jpeg */                   nullptr, // Not needed
    /* Format_CameraRaw */              nullptr,
    /* Format_AdobeDng */               nullptr,

};




frameToImage::frameToImage()
{

}

QImage frameToImage::frameFrom(QVideoFrame &frame)
{
    QImage result;

    if (!frame.isValid() || !frame.map(QAbstractVideoBuffer::ReadOnly))
        return result;

    // Formats supported by QImage don't need conversion
    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
    if (imageFormat != QImage::Format_Invalid) {
        result = QImage(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), imageFormat).copy();
    }

    // Load from JPG
    else if (frame.pixelFormat() == QVideoFrame::Format_Jpeg) {
        result.loadFromData(frame.bits(), frame.mappedBytes(), "JPG");
    }

    // Need conversion
    else {
        static bool initAsmFuncsDone = false;
        if (!initAsmFuncsDone) {
            initAsmFuncsDone = true;
        }
        VideoFrameConvertFunc convert = qConvertFuncs[frame.pixelFormat()];
        if (!convert) {
            qWarning() << Q_FUNC_INFO << ": unsupported pixel format" << frame.pixelFormat();
        } else {
            result = QImage(frame.width(), frame.height(), QImage::Format_ARGB32);
            convert(frame, result.bits());
        }
    }

    frame.unmap();

    return result;
}

int frameToImage::Bound(int range_left, int data, int range_right)
{
    int index=data;
    if(data>range_right)
    {
        index=range_right;
    }
    else if (data<range_left) {
        index=range_left;
    }
    return index;
}
void frameToImage::InverseColorImage(QImage *img,QImage *imgCopy)
{
    if(!img||!imgCopy){
        return ;
    }

    QRgb * line;
    for(int y = 0; y<imgCopy->height(); y++){
        line = (QRgb *)imgCopy->scanLine(y);
        for(int x = 0; x<imgCopy->width(); x++){

            imgCopy->setPixel(x,y, qRgb(255-qRed(line[x]), 255-qGreen(line[x]), 255-qBlue(line[x])));
        }

    }
}

//qimage磨皮
void frameToImage::QImageD_RunBEEPSHorizontalVertical(QImage *img,QImage *imgCopy,double spatialDecay,double photometricStandardDeviation)
{
    if(!img||!imgCopy){
        return ;
    }

    double c=-0.5/(photometricStandardDeviation * photometricStandardDeviation); //-1/2 *光度标准偏差的平方
    double mu=spatialDecay/(2-spatialDecay);

    double *exptable=new double[256];
    double *g_table=new double[256];
    for (int i=0;i<=255;i++)
    {
        exptable[i]=(1-spatialDecay)* exp(c*i*i);
        g_table[i]=mu*i;
    }
    int width=img->width();
    int height=img->height();
    int length=width*height;
    double* data2Red= new double[length];
    double* data2Green= new double[length];
    double* data2Blue= new double[length];

    int i=0;

    for(int y=0;y<height;y++)
    {
        for(int x=0;x<width;x++)
        {
            QRgb rgb=imgCopy->pixel(x,y);
            data2Red[i]=qRed(rgb);
            data2Green[i]=qGreen(rgb);
            data2Blue[i]=qBlue(rgb);
            i++;
        }
    }


    double* gRed = new double[length];
    double* pRed = new double[length];
    double* rRed = new double[length];

    double* gGreen = new double[length];
    double* pGreen = new double[length];
    double* rGreen = new double[length];

    double* gBlue = new double[length];
    double* pBlue = new double[length];
    double* rBlue = new double[length];
    memcpy(pRed,data2Red, sizeof(double) * length);
    memcpy(rRed,data2Red, sizeof(double) * length);

    memcpy(pGreen,data2Green, sizeof(double) * length);
    memcpy(rGreen,data2Green, sizeof(double) * length);

    memcpy(pBlue,data2Blue, sizeof(double) * length);
    memcpy(rBlue,data2Blue, sizeof(double) * length);


    double rho0=1.0/(2-spatialDecay);
    for (int k2 = 0;k2 < height;++k2)
    {
        int startIndex=k2 * width;
        double mu=0.0;
        for (int k=startIndex+1,K=startIndex+width;k<K;++k)
        {
            int div0Red=fabs(pRed[k]-pRed[k-1]);
            mu =exptable[div0Red];
            pRed[k] = pRed[k - 1] * mu + pRed[k] * (1.0 - mu);//公式1

            int div0Green=fabs(pGreen[k]-pGreen[k-1]);
            mu =exptable[div0Green];
            pGreen[k] = pGreen[k - 1] * mu + pGreen[k] * (1.0 - mu);//公式1

            int div0Blue=fabs(pBlue[k]-pBlue[k-1]);
            mu =exptable[div0Blue];
            pBlue[k] = pBlue[k - 1] * mu + pBlue[k] * (1.0 - mu);//公式1

        }

        for (int k =startIndex + width - 2;startIndex <= k;--k)
        {
            int div0Red=fabs(rRed[k]-rRed[k+1]);
            double mu =exptable[div0Red];
            rRed[k] = rRed[k + 1] * mu + rRed[k] * (1.0 - mu);//公式3

            int div0Green=fabs(rGreen[k]-rGreen[k+1]);
            mu =exptable[div0Green];
            rGreen[k] = rGreen[k + 1] * mu + rGreen[k] * (1.0 - mu);//公式3

            int div0Blue=fabs(rBlue[k]-rBlue[k+1]);
            mu =exptable[div0Blue];
            rBlue[k] = rBlue[k + 1] * mu + rBlue[k] * (1.0 - mu);//公式3
        }
        for (int k =startIndex,K=startIndex+width;k<K;k++)
        {
            rRed[k]=(rRed[k]+pRed[k])*rho0- g_table[(int)data2Red[k]];
            rGreen[k]=(rGreen[k]+pGreen[k])*rho0- g_table[(int)data2Green[k]];
            rBlue[k]=(rBlue[k]+pBlue[k])*rho0- g_table[(int)data2Blue[k]];
        }
    }

    int m = 0;
    for (int k2=0;k2<height;k2++)
    {
        int n = k2;
        for (int k1=0;k1<width;k1++)
        {
            gRed[n] = rRed[m];
            gGreen[n] = rGreen[m];
            gBlue[n] = rBlue[m];
            m++;
            n += height;
        }
    }

    memcpy(pRed, gRed, sizeof(double) * height * width);
    memcpy(rRed, gRed, sizeof(double) * height * width);

    memcpy(pGreen, gGreen, sizeof(double) * height * width);
    memcpy(rGreen, gGreen, sizeof(double) * height * width);

    memcpy(pBlue, gBlue, sizeof(double) * height * width);
    memcpy(rBlue, gBlue, sizeof(double) * height * width);

    for (int k1=0;k1<width;++k1)
    {
        int startIndex=k1 * height;
        double mu = 0.0;
        for (int k =startIndex+1,K =startIndex+height;k<K;++k)
        {
            int div0Red=fabs(pRed[k]-pRed[k-1]);
            mu =exptable[div0Red];
            pRed[k] = pRed[k - 1] * mu + pRed[k] * (1.0 - mu);

            int div0Green=fabs(pGreen[k]-pGreen[k-1]);
            mu =exptable[div0Green];
            pGreen[k] = pGreen[k - 1] * mu + pGreen[k] * (1.0 - mu);

            int div0Blue=fabs(pBlue[k]-pBlue[k-1]);
            mu =exptable[div0Blue];
            pBlue[k] = pBlue[k - 1] * mu + pBlue[k] * (1.0 - mu);
        }
        for (int k=startIndex+height-2;startIndex<=k;--k)
        {
            int div0Red=fabs(rRed[k]-rRed[k+1]);
            mu =exptable[div0Red];
            rRed[k] = rRed[k + 1] * mu + rRed[k] * (1.0 - mu);

            int div0Green=fabs(rGreen[k]-rGreen[k+1]);
            mu =exptable[div0Green];
            rGreen[k] = rGreen[k + 1] * mu + rGreen[k] * (1.0 - mu);

            int div0Blue=fabs(rBlue[k]-rBlue[k+1]);
            mu =exptable[div0Blue];
            rBlue[k] = rBlue[k + 1] * mu + rBlue[k] * (1.0 - mu);
        }
    }

    double init_gain_mu=spatialDecay/(2-spatialDecay);
    for (int k = 0;k <length;++k)
    {
        rRed[k]= (rRed[k]+pRed[k])*rho0- gRed[k]*init_gain_mu;

        rGreen[k]= (rGreen[k]+pGreen[k])*rho0- gGreen[k]*init_gain_mu;

        rBlue[k]= (rBlue[k]+pBlue[k])*rho0- gBlue[k]*init_gain_mu;

    }

    m = 0;
    for (int k1=0;k1<width;++k1)
    {
        int n = k1;
        for (int k2=0;k2<height;++k2)
        {

            data2Red[n]=rRed[m];
            data2Green[n]=rGreen[m];
            data2Blue[n]=rBlue[m];
            imgCopy->setPixel(k1,k2,qRgb(data2Red[n],data2Green[n],data2Blue[n]));
            m++;
            n += width;
        }
    }
    delete []data2Red;
    data2Red=nullptr;
    delete []data2Green ;
    data2Green=nullptr;
    delete []data2Blue;
    data2Blue=nullptr;

    delete []pRed;
    pRed=nullptr;
    delete []rRed;
    rRed=nullptr;
    delete []gRed;
    gRed=nullptr;

    delete []pGreen;
    pGreen=nullptr;
    delete []rGreen;
    rGreen=nullptr;
    delete []gGreen;
    gGreen=nullptr;

    delete []pBlue;
    pBlue=nullptr;
    delete []rBlue;
    rBlue=nullptr;
    delete []gBlue;
    gBlue=nullptr;

    delete []exptable;
    exptable=nullptr;
    delete []g_table;
    g_table=nullptr;
}

void frameToImage::oldImage(QImage *img,QImage *imgCopy)
{
    if(!img||!imgCopy){
        return ;
    }

    QRgb * line;
    for(int y = 0; y<imgCopy->height(); y++){
        line = (QRgb *)imgCopy->scanLine(y);
        for(int x = 0; x<imgCopy->width(); x++){
            float r=0.393 *qRed(line[x])+0.769 *qGreen(line[x])+0.189 *qBlue(line[x]);
            float g=0.349 *qRed(line[x])+0.686 *qGreen(line[x])+0.168 *qBlue(line[x]);
            float b=0.272 *qRed(line[x])+0.534 *qGreen(line[x])+0.131 *qBlue(line[x]);
            r=Bound(0,r,255);
            g=Bound(0,g,255);
            b=Bound(0,b,255);
            imgCopy->setPixel(x,y, qRgb(r, g, b));
        }

    }
}

void frameToImage::GrayScaleImage(QImage *img, QImage *imgCopy)
{
    if(!img||!imgCopy){
        return ;
    }

    QRgb * line;
    for(int y = 0; y<imgCopy->height(); y++){
        line = (QRgb *)imgCopy->scanLine(y);
        for(int x = 0; x<imgCopy->width(); x++){
            int average = (qRed(line[x]) + qGreen(line[x]) + qBlue(line[x]))/3;
            imgCopy->setPixel(x,y, qRgb(average, average, average));
        }

    }
}

void frameToImage::warnImage(QImage *img, QImage *imgCopy, int index)
{
    if(!img||!imgCopy){
        return ;
    }

    QRgb * line;
    QColor frontColor;
    for(int y = 0; y<imgCopy->height(); y++){
        line = (QRgb *)imgCopy->scanLine(y);
        for(int x = 0; x<imgCopy->width(); x++){
            frontColor=line[x];
            float r=frontColor.red()+index;
            float g=frontColor.green()+index;
            float b=frontColor.blue();
            r=Bound(0,r,255);
            g=Bound(0,g,255);
            imgCopy->setPixel(x,y, qRgb(r, g, b));
        }

    }
}

void frameToImage::coolImage(QImage *img, QImage *imgCopy, int index)
{
    if(!img||!imgCopy){
        return ;
    }

    QRgb * line;
    QColor frontColor;
    for(int y = 0; y<imgCopy->height(); y++){
        line = (QRgb *)imgCopy->scanLine(y);
        for(int x = 0; x<imgCopy->width(); x++){
            frontColor=line[x];
            float r=frontColor.red();
            float g=frontColor.green();
            float b=frontColor.blue()+index;
            b=Bound(0,b,255);
            imgCopy->setPixel(x,y, qRgb(r, g, b));
        }

    }
}

