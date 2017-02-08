#ifndef SHAREDCOM_H
#define SHAREDCOM_H


#include "qthread.h"
#include "qtimer.h"
#include "semaphore.h"

#define msgPing 1
#define msgPong 2
#define msgReset 3
#define msgQuit 4
#define msgLose 5
#define msgEnable 6
#define msgDisable 7



class sharedCom:public QThread
{
    Q_OBJECT
public:
    explicit sharedCom(QObject*);
    void run() Q_DECL_OVERRIDE;
    bool running;
    ~sharedCom();
private:
    QObject *parent;
    QTimer *watchTimer;
    char *inMemory,*outMemory;
    int shmidOut,shmidIn;
    char bufOut,bufIn;
    void createsem(int *sid, key_t key);
    void locksem(int sid);
    void unlocksem(int sid);
    void removesem(int sid);
    int sem_id;
    sem_t *semOut,*semIn;
private slots:
    void sendData(char data);
    void watchData();
    void triggerSlot();
signals:
    void signalReset(void);
    void signalStart(void);
    void signalEnable(bool);
};

#endif // SHAREDCOM_H
