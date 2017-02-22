#include "sharedcom.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "qdebug.h"
#include <fcntl.h>
#include "sys/stat.h"
#include "mainwindow.h"
#include "sys/sem.h"
#include "semaphore.h"




sharedCom::sharedCom(QObject *parent):parent(parent)
{
}

void sharedCom::run()
{
    watchTimer = new QTimer;
    watchTimer->start(200);
    connect(watchTimer,SIGNAL(timeout()),this,SLOT(watchData()));

    // give your shared memory an id, anything will do
    key_t keyIn = 111222;
    key_t keyOut = 111333;
    key_t keyData = 111444;

    //C++ to python:
    // Setup shared memory, 1 is the size
    if ((shmidOut = shmget(keyOut, 1, IPC_CREAT | 0666)) < 0)
    {
        qDebug()<<"Error getting shared memory id c2p";
        exit(1);
    }
    // Attached shared memory
    if ((outMemory = (char*)shmat(shmidOut, NULL, 0)) == (char *) -1)
    {
        qDebug()<<"Error attaching shared memory id c2p";
        exit(1);
    }



    //data
    // Setup shared memory, 1 is the size
    if ((shmidData = shmget(keyData, 1, IPC_CREAT | 0666)) < 0)
    {
        qDebug()<<"Error getting shared memory id c2p";
        exit(1);
    }
    // Attached shared memory
    if ((dataMemory = (char*)shmat(shmidData, NULL, 0)) == (char *) -1)
    {
        qDebug()<<"Error attaching shared memory id c2p";
        exit(1);
    }





    //Python to C++
    // Setup shared memory, 1 is the size
    if ((shmidIn = shmget(keyIn, 1, IPC_CREAT | 0666)) < 0)
    {
        qDebug()<<"Error getting shared memory id";
        exit(1);
    }
    // Attached shared memory
    if ((inMemory = (char*)shmat(shmidIn, NULL, 0)) == (char *) -1)
    {
        qDebug()<<"Error attaching shared memory id";
        exit(1);
    }

    sem_unlink("/semaQtPy");
    sem_unlink("/semaPyQt");

    semOut = sem_open("/semaQtPy", O_CREAT,0777,1);
    semIn = sem_open("/semaPyQt", O_CREAT, 0777, 1);


    if(semOut == SEM_FAILED)
    {
        qDebug()<<"sem out failed";
        qDebug()<<errno;
        exit(1);

    }

    if(semIn == SEM_FAILED)
    {
        qDebug()<<"sem in failed";
        qDebug()<<errno;
        exit(1);
    }


    exec();


}

void sharedCom::sendMsg(char data)
{
    sem_wait(semOut);
    bufOut = data;
    memcpy(outMemory,&bufOut, sizeof(char));
    sem_post(semOut);

}


void sharedCom::sendData(char data)
{
    sem_wait(semOut);
    bufData = data;
    memcpy(dataMemory,&bufData, sizeof(char));
    sem_post(semOut);

}


void sharedCom::watchData(void)
{

    sem_wait(semIn);
    memcpy(&bufIn,inMemory,sizeof(char));

    if(bufIn!=0)
    {
        switch(bufIn)
        {
        case msgPing:sendMsg(msgPong);break;
        case msgPong:qDebug()<<"pong";break;
        case msgQuit:((MainWindow*)parent)->close();break;
        case msgReset:emit signalReset();break;
        case msgEnable:emit signalEnable(true);break;
        case msgDisable:emit signalEnable(false);break;
        default : qDebug()<<"received: "<<(int)bufIn;break;
        }

        memset(inMemory,0,sizeof(char));
    }

    sem_post(semIn);

}

void sharedCom::triggerSlot(int nb)
{

    sendData(nb);
    sendMsg(msgLose);

}

sharedCom::~sharedCom()
{
    // Detach and remove shared memory
    shmdt((const void*)shmidOut);
    shmctl(shmidOut, IPC_RMID, NULL);
}
