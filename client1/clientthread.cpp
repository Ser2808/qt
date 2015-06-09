//
// Serge Spraiter 20150608
//

#include "clientthread.h"

#include <QtNetwork>

ClientThread::ClientThread(QObject *parent, qintptr socketDescriptor, const uint interval,
                           QFile *cpfile, quint64 *posInFile, int *stopThread)
    : QThread(parent), socketDescriptor(socketDescriptor), tcpSocket(0)
{
    this->interval = interval;
    file = cpfile;
    this->posInFile = posInFile;
    this->stopThread = stopThread;
}

void ClientThread::run()
{
    if (!tcpSocket)
    {
        tcpSocket = new QTcpSocket();
        if (!tcpSocket->setSocketDescriptor(socketDescriptor)) {
            emit error(tcpSocket->error());
            delete tcpSocket;
            return;
        }
    }
    qint64 bufReadSize = 1024;
    char *bufRead = new char[bufReadSize];
    char *bufSend = new char[1];
    qint64 dataSize;
    if (!file->seek(*posInFile))
        return;
    while (!file->atEnd()) {
        dataSize = file->read(bufRead, bufReadSize);
        for (int i0 = 0; i0 < dataSize; i0++)
        {
            bufSend[0] = bufRead[i0];
            tcpSocket->write(bufSend);
            tcpSocket->flush();
            *posInFile = *posInFile + 1;
            if (*stopThread)
                return;
            emit updateStatusBar(QString("%1").arg(*posInFile));
            QDateTime dt = QDateTime::currentDateTime().addMSecs(interval);
            while (dt > QDateTime::currentDateTime())
            {
                if (*stopThread)
                    return;
                msleep(1); // max delay 1 ms
            }
            if (*stopThread)
                return;
        }
    }
}
