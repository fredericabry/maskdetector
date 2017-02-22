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
#include "qfiledialog.h"
#include "qtimer.h"
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "mask_c.h"
#include "qtimer.h"
#include "qstringlist.h"

#include "QtGlobal"

#include "sharedcom.h"


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

void MainWindow::snapAndSave(void)
{

    qDebug()<<"snap";


}

void MainWindow::saveParameters(void)
{
    QFile param( QCoreApplication::applicationDirPath() +"/"+"config.cfg");
    if (!param.open(QIODevice::WriteOnly | QIODevice::Text))
        qDebug()<<"config file not found";


    param.resize(0);

    QTextStream out(&param);

    out << "width="<<QString::number(ui->spinWidth->value())<<"\n";
    out << "height="<<QString::number(ui->spinHeight->value())<<"\n";
    out << "resolution="<<QString::number(ui->spinRes->value())<<"\n";
    out << "threshold_zone="<<QString::number(ui->spinThresholdZone->value())<<"\n";
    out << "threshold_sat="<<QString::number(ui->spinThreshold->value())<<"\n";
    out << "exposition="<<QString::number(ui->spinExposition->value())<<"\n";

    param.close();


    qDebug()<<"saved";

}

void MainWindow::loadParameters(void)
{
    QString line;
    QStringList fields;
    bool test;
    int value;
    QFile param( QCoreApplication::applicationDirPath() +"/"+"config.cfg");
    if (!param.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug()<<"config file not found";

    while(!param.atEnd()) {
        line = param.readLine();
        line.remove("\n");

        fields = line.split("=");

        if(fields.length()==2)
        {
            value = fields[1].toInt(&test);

            if((fields[0].contains("width"))&&(test))
                ui->spinWidth->setValue(value);
            else if(fields[0].contains("height"))
                ui->spinHeight->setValue(value);
            else if(fields[0].contains("resolution"))
                ui->spinRes->setValue(value);
            else if(fields[0].contains("threshold_zone"))
                ui->spinThresholdZone->setValue(value);
            else if(fields[0].contains("threshold_sat"))
                ui->spinThreshold->setValue(value);
            else if(fields[0].contains("exposition"))
                ui->spinExposition->setValue(value);
            else
                qDebug()<<"unknown parameter";

            test = false;
        }
    }

    param.close();

    initialStart = true;
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


    lbl_imageSnap = new Mask(this,resolution,initialStart);
    lbl_imageSnap->setStyleSheet("border: 2px solid black");
    lbl_imageSnap->show();

    initialStart = false;


    connect(ui->bSave,SIGNAL(pressed()),lbl_imageSnap,SLOT(saveZones()));
    connect(ui->bLoad,SIGNAL(pressed()),lbl_imageSnap,SLOT(loadZones()));
    connect(this,SIGNAL(reload()),lbl_imageSnap,SLOT(loadZones()));


    connect(com,SIGNAL(signalStart()),lbl_imageSnap,SLOT(loadZones()));

    connect(ui->bSelect,SIGNAL(pressed()),lbl_imageSnap,SLOT(selectAllZones()));
    connect(ui->bUnselect,SIGNAL(pressed()),lbl_imageSnap,SLOT(unselectAllZones()));

    lbl_imageDiff = new QLabel(this);
    lbl_imageDiff->setStyleSheet("border: 2px solid black");
    lbl_imageDiff->show();

    w = ui->spinWidth->value();
    h = ui->spinHeight->value();

    cam = new camera_c(this,w,h,resolution,threshold,thresholdZone);
    connect(ui->radioEnabled,SIGNAL(clicked(bool)),cam,SLOT(enable(bool)));
    connect(ui->radioEnabled,SIGNAL(clicked(bool)),this,SLOT(clearDetected()));
    connect(com,SIGNAL(signalEnable(bool)),cam,SLOT(enable(bool)));

    cam->start();
    cam->enabled = false;
    connect(cam,SIGNAL(triggerSignal(int)),com,SLOT(triggerSlot(int)));

    connect(cam,SIGNAL(sendZone(std::vector<std::vector<bool> >)),lbl_imageSnap,SLOT(selectZone(std::vector<std::vector<bool> >)));

    setSize(WIDTH,HEIGHT);

    emit testCom(msgPing);
}

void MainWindow::setUpMarkers(void)
{

    double dx = (double)width/resolution;
    double dy = (double)height/resolution;

    //  qDebug()<<"dx marker "<<dx;


    markers.resize( resolution ,std::vector<mark*>( resolution , NULL ) );


    for (unsigned int xi = 0;xi<markers.size();xi++)
        for (unsigned int yi = 0;yi<markers[0].size();yi++)
        {
            markers[xi][yi] = new mark(this,dx/2 + 40+xi*dx,dy/2+20+yi*dy,5);
            markers[xi][yi]->hide();

        }

}

void MainWindow::setSize(int w, int h)
{

    width = w;
    height = h;



    QRect rect2(40 , 20, w,h);
    lbl_imageSnap->setGeometry(rect2);
    QRect rect3(60+w, 20, w,h);
    lbl_imageDiff->setGeometry(rect3);



    setUpMarkers();

    lbl_imageSnap->redraw();

}

void MainWindow::restart()
{
    //clean chain destruction
    cam->shutdown();
    ui->radioEnabled->setChecked(false);
}

void MainWindow::reset(void)
{
    restart();

    QTimer::singleShot(4000,this,SIGNAL(reload()));
    cam->enabled = false;
}
QTimer test;
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),  ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    com = new sharedCom(this);
    connect(this,SIGNAL(testCom(char)),com,SLOT(sendData(char)));
    connect(com,SIGNAL(signalReset()),this,SLOT(reset()));

    com->start();

    connect(ui->bRestart,SIGNAL(pressed()),this,SLOT(restart()));
    connect(ui->bShutdown,SIGNAL(pressed()),this,SLOT(close()));
    connect(ui->bClearDetection,SIGNAL(pressed()),this,SLOT(clearDetected()));
    connect(ui->radioDisplay,SIGNAL(clicked(bool)),this,SLOT(updateDisplay(bool)));

    connect(ui->bSnap,SIGNAL(pressed()),this,SLOT(snapAndSave()));

    connect(ui->bAutoLearn,SIGNAL(pressed()),this,SLOT(autoLearn()));
    autoLearnStatus= false;

    imageFileName = QCoreApplication::applicationDirPath() +"/"+ "snap.png";

    loadParameters();
    prepZoneFile(QCoreApplication::arguments());
    init();



}


void MainWindow::autoLearn(void)
{

    autoLearnStatus= !autoLearnStatus;



    if(autoLearnStatus)
    {
    ui->bAutoLearn->setText("stop");
    cam->startLearning();
    }
    else
    {
    ui->bAutoLearn->setText("start");
    cam->stopLearning();

    double dx = (double)width/resolution;
    double dy = (double)height/resolution;


    for (unsigned int xi = 0;xi<markers.size();xi++)
        for (unsigned int yi = 0;yi<markers[0].size();yi++)
        {
               markers[xi][yi]->hide();

        }

    }



}



void MainWindow::prepZoneFile(QStringList cmdline_args)
{
    int nures;
    QString line;
    QStringList fields;

    if(cmdline_args.size()>1)
    {
        QStringList buf;
        buf = cmdline_args[1].split(".");
        if(buf.size()==2)
        {
            if(buf[1] == "txt")
                zoneFileName = cmdline_args[1];
        }
    }

    zoneFileName = QCoreApplication::applicationDirPath() +"/"+ zoneFileName;





   // zoneFileName = "/home/pi/qt5/test/zones42.txt";
    QFile file(zoneFileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"cannot open zones file";
        return;
    }

    line = file.readLine();
    fields = line.split("=");
    if(fields.length()==2)
    {
        if(fields[0]=="res")
        {
            nures = fields[1].toInt();
            if((nures<=0)||(nures>50))
            {
                qDebug()<<"invalid resolution: "<<nures;
                return;
            }
            resolution = nures;
            ui->spinRes->setValue(resolution);
        }
    }
    else
    {
        qDebug()<<"cannot read resolution";
        return;
    }
    file.close();
}

void MainWindow::setMarkerVisible(unsigned int x,unsigned int y,bool status)
{
    if(x>=markers.size()) return;
    if(y>=markers[0].size()) return;

    if(status)
        markers[x][y]->show();
    else
        markers[x][y]->hide2();
}

void MainWindow::clearDetected(void)
{

    for (unsigned int xi = 0;xi<markers.size();xi++)
        for (unsigned int yi = 0;yi<markers[0].size();yi++)
            markers[xi][yi]->hide();


}

void MainWindow::dataAvailable(int type)
{
    if(type == 1)
    {
        //lbl_image->setPixmap(QPixmap::fromImage(cam->qImage));
    }
    else if(type==2)
    {

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
    saveParameters();
    delete ui;
}
