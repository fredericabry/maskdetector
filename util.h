#ifndef UTIL_H
#define UTIL_H



#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "qdebug.h"
#include "opencv2/videoio.hpp"






QImage Mat2QImage(cv::Mat const& src);
cv::Mat QImage2Mat(QImage const& src);

//QImage  cvMatToQImage( const cv::Mat &inMat );





#endif // UTIL_H
