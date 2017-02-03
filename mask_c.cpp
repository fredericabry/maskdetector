#include "mask_c.h"
#include "QMouseEvent"
#include "qimage.h"
#include "qdebug.h"
#include "qpainter.h"








Mask::Mask(QWidget *parent,int res) : QLabel(parent),res(res)
{
    markedZone.resize( res ,std::vector<bool>( res , false ) );
    //detectedZone.resize( res ,std::vector<bool>( res , false ) );
}


void Mask::mousePressEvent(QMouseEvent *event)
{
    int x=event->x();
    int y=event->y();

    if (res<=0)
    {

        return;

    }

    int dx = width()/res;
    int dy = height()/res;

    if((dx > 0)&&(dy>0))
    {
        int X,Y;

        X = x/dx;
        Y = y/dy;
        emit clickZone(X,Y);

        if((X<res)&&(Y<res))
        {

            markedZone[X][Y]=!markedZone[X][Y];
            redraw();

        }




    }

}

void Mask::selectAllZones(void)
{
    for (int xi = 0;xi<res;xi++)
        for (int yi = 0;yi<res;yi++)
        {

            markedZone[xi][yi] = true;

        }
    redraw();
}

void Mask::unselectAllZones(void)
{
    for (int xi = 0;xi<res;xi++)
        for (int yi = 0;yi<res;yi++)
        {

            markedZone[xi][yi] = false;

        }
    redraw();
}

void Mask::setImg(QImage img)
{

    image = img.copy();



    /* for (int xi = 0;xi<res;xi++)
        for(int yi = 0;yi<res;yi++)
            markedZone[xi][yi] = false;*/

    resetDetection( );
    redraw();
}

void Mask::resetDetection(void)
{

}


void Mask::saveZones(void)
{

    QFile file("zones.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug()<<"cannot open zones file";
        return;
    }

    file.resize(0);

    QTextStream out(&file);
    for (int xi = 0;xi<res;xi++)
        for (int yi = 0;yi<res;yi++)
        {

            if(markedZone[xi][yi])
                out << xi<<","<<yi<<"\n";

        }




}

void Mask::loadZones(void)
{
    int x,y;

    unselectAllZones();


    QFile file("zones.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"cannot open zones file";
        return;
    }
    while(!file.atEnd()) {
        QString line = file.readLine();
        line.remove("\n");
        QStringList fields = line.split(",");

        if(fields.length()==2)
        {
            x = fields[0].toInt();
            y = fields[1].toInt();

            if((x>=0)&&(y>=0)&&(x<res)&&(y<<res))
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

    if (res<=0)
    {
        qDebug()<<"bad resolution";
        return;
    }

    int dy = rect.height()/res;
    int dx = rect.width()/res;

    for (int i = 1;i<= res;i++)
    {
        QLineF line(rect.topLeft().x(), rect.topLeft().y()+dy*i,rect.topRight().x(), rect.topRight().y()+dy*i);
        painter.drawLine(line);
        QLineF line2(rect.topLeft().x()+dx*i, rect.topLeft().y(),rect.topLeft().x()+dx*i, rect.bottomLeft().y());
        painter.drawLine(line2);
    }


    for (int xi = 0;xi<res;xi++)
        for (int yi = 0;yi<res;yi++)
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

void Mask::showZone(int x, int y)
{
    if (res<=0)
        return;

    int dx = width()/res;
    int dy = height()/res;


    QPixmap pix = pixmap()->copy();
    QPainter painter(&pix);
    QRect rect = pix.rect();


    QLineF line(rect.topLeft().x()+dx*x, rect.topLeft().y()+dy*y,rect.topLeft().x()+dx*(x+1), rect.topLeft().y()+dy*(y+1));
    painter.drawLine(line);
    QLineF line2(rect.topLeft().x()+dx*(x+1),rect.topLeft().y()+dy*y,rect.topLeft().x()+dx*(x), rect.topLeft().y()+dy*(y+1));
    painter.drawLine(line2);

    setPixmap(pix);

}

bool Mask::isZoneMarked(int x, int y)
{
    if((x>=res)|| (y>=res))
    {
        qDebug()<<"iszonemarked bug";
        return false;
    }

    return markedZone[x][y];

}

