#include "mask_c.h"
#include "QMouseEvent"
#include "qimage.h"
#include "qdebug.h"
#include "qpainter.h"
#include "mainwindow.h"
#include "QtGlobal"






Mask::Mask(QWidget *parent, int res,bool loadFile) : QLabel(parent),resolution(res),loadFile(loadFile)
{
    markedZone.resize( res ,std::vector<bool>( res , false ) );
}

void Mask::mouseReleaseEvent(QMouseEvent *event)
{

    int x=event->x();
    int y=event->y();

    if((x<0)||(y<0)||(x>width())||(y>height()))
        return;



    if((dx <= 0)||(dy<=0)) return;

    int X0,Y0,X1,Y1;

    X0 = click_x/dx;
    Y0 = click_y/dy;

    X1 = x/dx;
    Y1 = y/dy;

    bool state = markedZone[X0][Y0];

    if((X1<resolution)&&(Y1<resolution)&&(X0<resolution)&&(Y0<resolution))
    {

        if(X1>=X0)
            for (int i = X0;i<=X1;i++)
            {
                if(Y1>=Y0)
                    for (int j = Y0;j<=Y1;j++)
                        markedZone[i][j] = state;
                else
                    for (int j = Y1;j<=Y0;j++)
                        markedZone[i][j] = state;
            }
        else
            for (int i = X1;i<=X0;i++)
            {
                if(Y1>=Y0)
                    for (int j = Y0;j<=Y1;j++)
                        markedZone[i][j] = state;
                else
                    for (int j = Y1;j<=Y0;j++)
                        markedZone[i][j] = state;
            }

        //   markedZone[X][Y]=!markedZone[X][Y];
        redraw();

    }



}

void Mask::mousePressEvent(QMouseEvent *event)
{
    click_x=event->x();
    click_y=event->y();

    if (resolution<=0)
        return;



    if((dx > 0)&&(dy>0))
    {
        int X,Y;

        X = click_x/dx;
        Y = click_y/dy;

        if((X<resolution)&&(Y<resolution))
        {
            markedZone[X][Y]=!markedZone[X][Y];
            redraw();
        }
    }
}

void Mask::selectAllZones(void)
{
    for (int xi = 0;xi<resolution;xi++)
        for (int yi = 0;yi<resolution;yi++)
        {

            markedZone[xi][yi] = true;

        }
    redraw();
}



void Mask::selectZone(std::vector<std::vector<bool>> Zone)
{

    qDebug()<<"select";

    for (int xi = 0;xi<resolution;xi++)
        for (int yi = 0;yi<resolution;yi++)
        {

            markedZone[xi][yi] = !Zone[xi][yi];

        }
    redraw();

}








void Mask::unselectAllZones(void)
{
    for (int xi = 0;xi<resolution;xi++)
        for (int yi = 0;yi<resolution;yi++)
        {

            markedZone[xi][yi] = false;

        }
    redraw();
}

void Mask::setImg(QImage img)
{

    image = img.copy();

    if(loadFile)
    loadZones();


    redraw();
}

void Mask::saveZones(void)
{
    QFile file(((MainWindow*)parent())->zoneFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug()<<"cannot open zones file";
        return;
    }

    file.resize(0);

    QTextStream out(&file);

    out << "res="<<QString::number(resolution)<<"\n";

    for (int xi = 0;xi<resolution;xi++)
        for (int yi = 0;yi<resolution;yi++)
        {
            if(markedZone[xi][yi])
                out << xi<<","<<yi<<"\n";
        }
}

void Mask::loadZones(void)
{
    int x,y;
    QString line;
    QStringList fields;



    QFile file(((MainWindow*)parent())->zoneFileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"cannot open zones file";
        return;
    }

   unselectAllZones();



    line = file.readLine();

    markedZone.clear();
    markedZone.resize( resolution ,std::vector<bool>( resolution , false ) );

    while(!file.atEnd()) {
        line = file.readLine();
        line.remove("\n");

        fields = line.split(",");

        if(fields.length()==2)
        {
            x = fields[0].toInt();
            y = fields[1].toInt();

            if((x>=0)&&(y>=0)&&(x<resolution)&&(y<resolution))
                markedZone[x][y] = true;

        }
    }


    redraw();
}

void Mask::redraw()
{
    QPixmap pix = QPixmap::fromImage(image);

    QPainter painter(&pix);
    QRect rect = pix.rect();

    if (resolution<=0)
    {
        qDebug()<<"bad resolution";
        return;
    }

    dy = ((double)height()/resolution);
    dx = ((double)width()/resolution);

    // qDebug()<<"dx redraw "<<dx;

    for (int i = 1;i<= resolution;i++)
    {
        QLineF line(rect.topLeft().x(), rect.topLeft().y()+dy*i,rect.topRight().x(), rect.topRight().y()+dy*i);
        painter.drawLine(line);
        QLineF line2(rect.topLeft().x()+dx*i, rect.topLeft().y(),rect.topLeft().x()+dx*i, rect.bottomLeft().y());
        painter.drawLine(line2);
    }
    for (int xi = 0;xi<resolution;xi++)
        for (int yi = 0;yi<resolution;yi++)
        {
            if(markedZone[xi][yi])
            {

                QLineF line(rect.topLeft().x()+dx*xi, rect.topLeft().y()+dy*yi,rect.topLeft().x()+dx*(xi+1), rect.topLeft().y()+dy*(yi+1));
                painter.drawLine(line);
                QLineF line2(rect.topLeft().x()+dx*(xi+1),rect.topLeft().y()+dy*yi,rect.topLeft().x()+dx*(xi), rect.topLeft().y()+dy*(yi+1));
                painter.drawLine(line2);
            }
        }

    setPixmap(pix);

}




bool Mask::isZoneMarked(int x, int y)
{
    if((x>=resolution)|| (y>=resolution))
    {
        qDebug()<<"is zonemarked bug";
        return false;
    }
    return markedZone[x][y];
}

