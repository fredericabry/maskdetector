#ifndef MASK_C_H
#define MASK_C_H

#include "qlabel.h"
#include "qthread.h"






class Mask:public QLabel
{
    Q_OBJECT
public:
    int resolution;
    explicit Mask(QWidget* parent=0, int resolution=10 , bool loadFile=false);
    void setImg(QImage);
    void redraw(void);
    void resetDetection(void);
    void markUndetected(int x,int y);
    void markDetected(int x,int y);
    int click_x,click_y;
    double dx,dy;
    bool loadFile;

    std::vector<std::vector<bool> > markedZone;
  //  std::vector<std::vector<bool> > detectedZone;
    bool isZoneMarked(int x, int y);
    QImage image;

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
signals:

    void setRes(int);

private slots:
    void selectAllZones();
    void unselectAllZones();
    void loadZones();
    void saveZones();
    void selectZone(std::vector<std::vector<bool> >);
};


#endif // MASK_C_H
