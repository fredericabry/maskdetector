#ifndef MASK_C_H
#define MASK_C_H

#include "qlabel.h"







class Mask:public QLabel
{
    Q_OBJECT
public:
    int res;
    explicit Mask(QWidget* parent=0, int res=10 );
    void setImg(QImage);
    void redraw(void);
    void showZone(int x,int y);
    bool isZoneMarked(int x,int y);
    bool isZoneDetected(int x,int y);
    void resetDetection(void);
    void markUndetected(int x,int y);
    void markDetected(int x,int y);
private:
    std::vector<std::vector<bool> > markedZone;
  //  std::vector<std::vector<bool> > detectedZone;

    QImage image;






protected:
    void mousePressEvent(QMouseEvent* event);

signals:
    void clickZone(int x,int y);

private slots:
    void selectAllZones();
    void unselectAllZones();
    void loadZones();
    void saveZones();
};


#endif // MASK_C_H
