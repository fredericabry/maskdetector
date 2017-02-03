#include "util.h"
#include "qimage.h"
#include "qpixmap.h"
#include "qdebug.h"




QImage  Mat2QImage( const cv::Mat &inMat )
{

    switch ( inMat.type() )
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_ARGB32 );


        return image;
    }

        // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_RGB888 );

        return image.rgbSwapped();
    }

        // 8-bit, 1 channel
    case CV_8UC1:
    {
   /*     static QVector<QRgb>  sColorTable( 256 );

        // only create our color table the first time
        if ( sColorTable.isEmpty() )
        {
            for ( int i = 0; i < 256; ++i )
            {
                sColorTable[i] = qRgb( i, i, i );
            }
        }

        QImage image( inMat.data,
                      inMat.cols, inMat.rows,
                      static_cast<int>(inMat.step),
                      QImage::Format_Indexed8 );

        image.setColorTable( sColorTable );
*/


        QImage image(inMat.data, inMat.cols, inMat.rows,
                             static_cast<int>(inMat.step),
                             QImage::Format_Grayscale8);


        return image;
    }

    default:
        qDebug() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
        break;
    }

    return QImage();
}











/*
QImage Mat2QImage(cv::Mat const& src)
{
    cv::Mat temp; // make the same cv::Mat

    cvtColor(src, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need

    QImage dest((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);

    dest.bits(); // enforce deep copy, see documentation

    // of QImage::QImage ( const uchar * data, int width, int height, Format format )
    return dest;
}*/





cv::Mat QImage2Mat(QImage const& src)
{

    cv::Mat tmp(src.height(),src.width(),CV_8UC3,(uchar*)src.bits(),src.bytesPerLine());


    cv::Mat result; // deep copy just in case (my lack of knowledge with open cv)


    cvtColor(tmp, result,CV_BGR2RGB);



    return result;
}

