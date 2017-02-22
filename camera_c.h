#ifndef CAMERA_C_H
#define CAMERA_C_H

#include "qthread.h"
#include "qobject.h"
#include "qlabel.h"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "qdebug.h"
#include "opencv2/videoio.hpp"
#include "mainwindow.h"
#include "qmutex.h"


class Mask;


using namespace cv;

class camera_c;

class cameraConsumer_c:public QThread
{
    Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
    cameraConsumer_c(QObject *parent,camera_c *controler, int width, int heigth, int resolution, int threshold, int thresholdZone);
    void checkZones(void);
    int getZoneValue(int X,int Y);
    Mat imageDiff;
    QImage qImageDiff;
    QPixmap pixmapImageDiff;


private:
    QObject *parent;
    camera_c *controler;
    int width;
    int height;
    const int resolution;
    int threshold;
    int thresholdZone;


signals:
    void setMarkerVisible(uint,uint,bool);
    void dataReady(int);
private slots:
    void process(void);
    void dataReceived(void);
    void setSize(int w, int h);



};


class camera_c:public QThread
{
    Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
    camera_c(QObject *parent, int width, int heigth, int resolution, int threshold, int thresholdZone);
    ~camera_c(void);
    QObject *parent;
    bool running;
    VideoCapture *capture;
    void update(void);
    void init(void);

    Mat imageSnap;
    Mat imageDiff;
    Mat image;
    cv::Mat buf;
    cameraConsumer_c *consumer;
    QImage qImageSnap;
    QImage qImageDiff;
    QPixmap pixmapImageDiff;
    QMutex computing;
    void shutdown(void);
    double dx;
    double dy;
    bool enabled;
    void startLearning(void);
    void stopLearning(void);
    bool isLearning;
    std::vector<std::vector<bool> > detectedZone;
private:

    int width;
    int height;

    const int resolution;
    int threshold;
    int thresholdZone;




signals:
    void dataReady(int);
    void triggerSignal(int);
    void setSize(int,int);
    void startProcess();
    void sendZone(std::vector<std::vector<bool> >);
public slots:
    int getZoneValue(int X,int Y);

private slots:
    void snap(void)    ;
    void enable(bool);


};
#endif // CAMERA_C_H
