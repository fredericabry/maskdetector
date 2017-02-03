#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qlabel.h"


class mask_c;
class camera_c;

class Mask;

namespace Ui {
class MainWindow;
}



class mark:public QLabel
{
    Q_OBJECT
public:
    explicit mark(QWidget* parent=0,int x=0,int y=0,int size=1);
    int x;
    int y;
    int size;
public slots:
    void show2(void);
    void hide2(void);

};













class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    QLabel *getLabel(void);
    ~MainWindow();
    void keyPressEvent(QKeyEvent* e);
    QLabel *lbl_imageDiff;
    Mask *lbl_imageSnap;
    std::vector<std::vector<mark*> > markers;
    int resolution,threshold,thresholdZone,exposition;

private:
    Ui::MainWindow *ui;
    camera_c *cam;



private slots:
    void dataAvailable(int)    ;
    void setMarkerVisible(int x,int y,bool status);
    void clearDetected(void);
    void restart(void);
    void init(void);
    void updateDisplay(bool show);
    void setSize(int w, int h);

signals:
    void snap(void)    ;
    void acknowledgeData(void);

};





#endif // MAINWINDOW_H
