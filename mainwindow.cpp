#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "qdebug.h"
#include "opencv2/videoio.hpp"
#include "camera_c.h"
#include "qmessagebox.h"
#include "QKeyEvent"
#include "QPainter"


#include <libv4l2.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "mask_c.h"
#include "qtimer.h"

//#define resolution 20
#define WIDTH 640
#define HEIGHT 480



using namespace cv;



mark::mark(QWidget* parent,int x,int y,int size):QLabel(parent),x(x),y(y),size(size)
{
    QRect rect(x,y,size,size);
    setGeometry(rect);
    setStyleSheet("border: 3px solid red");
}

void mark::show2(void)
{
    show();
    QTimer::singleShot(5000,this,SLOT(hide()));
}

void mark::hide2(void)
{
    hide();
    //QTimer::singleShot(5000,this,SLOT(hide()));
}

void MainWindow::updateDisplay(bool show)
{
    if(show)
    {
        lbl_imageDiff->show();

    }
    else
    {
        lbl_imageDiff->hide();

    }


}




void MainWindow::init()
{
    int w,h;

    exposition = ui->spinExposition->value();
    if(exposition <= 0) exposition = 1;

    // open capture
    int descriptor = v4l2_open("/dev/video0", O_RDWR);

    // manual exposure control
    v4l2_control c;
    c.id = V4L2_CID_EXPOSURE_AUTO;
    c.value = V4L2_EXPOSURE_MANUAL;
    v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c) ;

    c.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    c.value = exposition;
    v4l2_ioctl(descriptor, VIDIOC_S_CTRL, &c);



    resolution = ui->spinRes->value();
    if(resolution <= 0) resolution = 1;

    threshold = ui->spinThreshold->value();
    if(threshold <= 0) threshold = 1;


    thresholdZone = ui->spinThresholdZone->value();
    if(thresholdZone <= 0) thresholdZone = 1;


    lbl_imageSnap = new Mask(this,resolution);
    lbl_imageSnap->setStyleSheet("border: 2px solid black");
    lbl_imageSnap->show();

    connect(ui->bSave,SIGNAL(pressed()),lbl_imageSnap,SLOT(saveZones()));
    connect(ui->bLoad,SIGNAL(pressed()),lbl_imageSnap,SLOT(loadZones()));
    connect(ui->bSelect,SIGNAL(pressed()),lbl_imageSnap,SLOT(selectAllZones()));
    connect(ui->bUnselect,SIGNAL(pressed()),lbl_imageSnap,SLOT(unselectAllZones()));



    lbl_imageDiff = new QLabel(this);
    lbl_imageDiff->setStyleSheet("border: 2px solid black");
    lbl_imageDiff->show();

    markers.resize( resolution ,std::vector<mark*>( resolution , NULL ) );

    w = ui->spinWidth->value();
    h = ui->spinHeight->value();

    cam = new camera_c(this,w,h,resolution,threshold,thresholdZone);
   // cam = new camera_c(this,WIDTH,HEIGHT,resolution,threshold,thresholdZone);
    cam->start();
   // connect(cam,SIGNAL(setSize(int,int)),this,SLOT(setSize(int,int)));

    setSize(WIDTH,HEIGHT);
}



void MainWindow::setSize(int w, int h)
{

    int dx = w/resolution;
    int dy = h/resolution;

    QRect rect2(40 , 20, w,h);
    lbl_imageSnap->setGeometry(rect2);
    QRect rect3(60+w, 20, w,h);
    lbl_imageDiff->setGeometry(rect3);



    for (int xi = 0;xi<resolution;xi++)
    {
        for (int yi = 0;yi<resolution;yi++)
        {
            markers[xi][yi] = new mark(this,dx/2 + 40+xi*dx,dy/2+20+yi*dy,5);
            markers[xi][yi]->hide();

        }
    }

    lbl_imageSnap->redraw();

}



void MainWindow::restart()
{
    //clean chain destruction
    cam->shutdown();

}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),  ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->bRestart,SIGNAL(pressed()),this,SLOT(restart()));
    connect(ui->bShutdown,SIGNAL(pressed()),this,SLOT(close()));
    connect(ui->bClearDetection,SIGNAL(pressed()),this,SLOT(clearDetected()));
    connect(ui->radioDisplay,SIGNAL(clicked(bool)),this,SLOT(updateDisplay(bool)));

    init();


}




void MainWindow::setMarkerVisible(int x,int y,bool status)
{
    if(x>=resolution) return;
    if(y>=resolution) return;

    if(status)
        markers[x][y]->show();
    else
        markers[x][y]->hide2();
}

void MainWindow::clearDetected(void)
{

    for (int xi = 0;xi<resolution;xi++)
    {
        for (int yi = 0;yi<resolution;yi++)
        {

            markers[xi][yi]->hide();

        }
    }

}

void MainWindow::dataAvailable(int type)
{
    if(type == 1)
    {
        //lbl_image->setPixmap(QPixmap::fromImage(cam->qImage));
    }
    else if(type==2)
    {
        //lbl_imageSnap->setText("test");
        QImage img2 = cam->qImageSnap.scaled(WIDTH,HEIGHT,Qt::KeepAspectRatio);
        lbl_imageSnap->setImg(img2);
        clearDetected();

    }
    else if(type==3)
    {

        if(ui->radioDisplay->isChecked())
        {
        QPixmap img2 = cam->consumer->pixmapImageDiff.scaled(WIDTH,HEIGHT,Qt::KeepAspectRatio);
        lbl_imageDiff->setPixmap(img2);

        }
        emit acknowledgeData();


    }
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
    emit snap();
}

MainWindow::~MainWindow()
{
    delete ui;
}
