#include "camera_c.h"
#include "mask_c.h"
#include "util.h"




#include "qelapsedtimer.h"


using namespace cv;



QElapsedTimer telapsed;









camera_c::camera_c(QObject *parent, int width, int height, int res, int threshold, int thresholdZone):parent(parent),width(width),height(height),resolution(res),threshold(threshold),thresholdZone(thresholdZone)
{

    isLearning = false;

    connect(parent,SIGNAL(snap()),this,SLOT(snap()));
    connect(this,SIGNAL(dataReady(int)),parent,SLOT(dataAvailable(int)));

    consumer = new cameraConsumer_c(parent,this,width,height,res,threshold,thresholdZone);

    connect(this,SIGNAL(startProcess()),consumer,SLOT(process()));
    connect(consumer,SIGNAL(dataReady(int)),parent,SLOT(dataAvailable(int)));
    connect(((MainWindow*)parent),SIGNAL(acknowledgeData()),consumer,SLOT(dataReceived()));

    connect(consumer,SIGNAL(setMarkerVisible(uint,uint,bool)),parent,SLOT(setMarkerVisible(uint,uint,bool)));

    consumer->start();

    qDebug()<<thread();
    qDebug()<<consumer->thread();

}

void camera_c::run(void)
{
    init();
    while(running)
    {
        update();

    }

    capture->release();
    deleteLater();
}

void camera_c::init()
{
    //NEEDED ON RASPI : sud

    capture = new VideoCapture(0);

    //VideoCapture cap(0); // open the default camera
    if(!capture->isOpened())  // check if we succeeded
    {
        qDebug()<<"camera not found";
        exit(0);
    }
    else
        qDebug()<< "camera ok";


    capture->set(CV_CAP_PROP_FRAME_WIDTH,width);
    capture->set(CV_CAP_PROP_FRAME_HEIGHT,height);

    width = capture->get(CV_CAP_PROP_FRAME_WIDTH);
    height = capture->get(CV_CAP_PROP_FRAME_HEIGHT);

    emit setSize(width,height);

    capture->set(CV_CAP_PROP_FPS,60);

    qDebug()<<"fps:"<<capture->get(CV_CAP_PROP_FPS);

    running = true;

    snap();
}

void camera_c::snap()
{
    if(capture->grab())
    {
        capture->retrieve(imageSnap);

        if(! imageSnap.data )                              // Check for invalid input
        {
            qDebug()<< "Could not open or find the image";
        }
        else
        {
            // Show our image inside it.

            qImageSnap = Mat2QImage(imageSnap);

            qImageSnap.save(((MainWindow*)parent)->imageFileName);




            emit dataReady(2);
        }

    }

}

void camera_c::update(void)
{
    if(capture->grab())
    {
        capture->retrieve(image);

        if(! image.data )                              // Check for invalid input
        {
            qDebug()<< "Could not open or find the image";
        }
        else
        {
            if(imageSnap.data)
            {
                cv::absdiff(imageSnap, image, buf);
                computing.lock(); //check if consumer has processed previous picture
                imageDiff = buf.clone();
                computing.unlock(); //consumer is free to process
                startProcess();
            }

            else
                emit dataReady(1);
        }
    }
}

void camera_c::enable(bool status)
{
    enabled = status;
}

int camera_c::getZoneValue(int X,int Y)
{
    int val;
    Mat subDiff = imageDiff(Rect(dx*X,dy*Y,dx,dy));
    Scalar tempVal = mean( subDiff );

    val = (int)tempVal.val[0];

    return val;
}

void camera_c::shutdown(void)
{
    connect(this,SIGNAL(destroyed(QObject*)),consumer,SLOT(deleteLater()));
    connect(consumer,SIGNAL(destroyed(QObject*)),((MainWindow*)parent)->lbl_imageDiff,SLOT(deleteLater()));
    connect(((MainWindow*)parent)->lbl_imageDiff,SIGNAL(destroyed(QObject*)),((MainWindow*)parent)->lbl_imageSnap,SLOT(deleteLater()));
    connect(((MainWindow*)parent)->lbl_imageSnap,SIGNAL(destroyed(QObject*)),((MainWindow*)parent),SLOT(init()));
    running = false;
}

camera_c::~camera_c(void)
{
    for (unsigned int xi = 0;xi<((MainWindow*)parent)->markers.size();xi++)
        for (unsigned int yi = 0;yi<((MainWindow*)parent)->markers[0].size();yi++)
            ((MainWindow*)parent)->markers[xi][yi] -> deleteLater();



    ((MainWindow*)parent)->markers.clear();
}

cameraConsumer_c::cameraConsumer_c(QObject *parent,camera_c * controler, int width, int height, int resolution, int threshold, int thresholdZone):parent(parent),controler(controler),width(width),height(height),resolution(resolution),threshold(threshold),thresholdZone(thresholdZone)
{
    controler->dx = ((double)width/resolution);
    controler->dy = ((double)height/resolution);
}

void cameraConsumer_c::run()
{
    exec();
}

void cameraConsumer_c::process(void)
{
    if(!controler->enabled)
        return;

    controler->computing.lock();
    imageDiff = controler->buf.clone();
    cv::threshold (imageDiff, imageDiff, threshold, 255, CV_THRESH_BINARY_INV);
    cv::cvtColor(imageDiff, imageDiff, cv::COLOR_BGR2GRAY);
    checkZones();
    qImageDiff = Mat2QImage(imageDiff);
    pixmapImageDiff = QPixmap::fromImage(qImageDiff);

    emit dataReady(3);

}

void cameraConsumer_c::dataReceived(void)
{

    controler->computing.unlock();

}

void cameraConsumer_c::checkZones(void)
{
    int zoneCmpt = 0;

    if(controler->isLearning)
    {
        for (int xi = 0;xi<resolution;xi++)
            for (int yi = 0;yi<resolution;yi++)
            {
                if(((MainWindow*)parent)->lbl_imageSnap->isZoneMarked(xi,yi))
                {

                    if(getZoneValue(xi,yi)<255-thresholdZone)
                    {

                        controler->detectedZone[xi][yi] = true;
                        emit setMarkerVisible(xi,yi,true);

                    }



                }
            }

    }
    else
    {

        for (int xi = 0;xi<resolution;xi++)
            for (int yi = 0;yi<resolution;yi++)
            {
                if(((MainWindow*)parent)->lbl_imageSnap->isZoneMarked(xi,yi))
                {

                    if(getZoneValue(xi,yi)<255-thresholdZone)
                    {


                        emit setMarkerVisible(xi,yi,true);
                        zoneCmpt ++;
                    }
                    else
                        emit setMarkerVisible(xi,yi,false);


                }
            }


        if(zoneCmpt>0)
            emit controler->triggerSignal(zoneCmpt);
    }

}

int cameraConsumer_c::getZoneValue(int X,int Y)
{
    int val;
    Mat subDiff = imageDiff(Rect(controler->dx*X,controler->dy*Y,controler->dx,controler->dy));
    Scalar tempVal = mean( subDiff );

    val = (int)tempVal.val[0];



    return val;
}

void cameraConsumer_c::setSize(int w, int h)
{

    width = w;
    height = h;

    controler->dx = (double)width/resolution;
    controler->dy = (double)height/resolution;


}


void camera_c::startLearning(void)
{
    isLearning = true;

    detectedZone.resize( resolution ,std::vector<bool>( resolution , false ) );

}


void camera_c::stopLearning(void)
{

    isLearning = false;
    sendZone(detectedZone);

}

