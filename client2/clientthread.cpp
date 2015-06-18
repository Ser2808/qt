//
// Serge Spraiter 20150616
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
        if (!tcpSocket->setSocketDescriptor(socketDescriptor))
        {
            emit error(tcpSocket->error());
            delete tcpSocket;
            return;
        }
    }
    qint64 bufReadSize = 1024;
    char *bufRead = new char[bufReadSize];
    char *bufSend = new char[2]; // word transmit
    qint64 dataSize;
    if (!file->seek(*posInFile))
        return;
    while (!file->atEnd())
    {
        dataSize = file->read(bufRead, bufReadSize);
        for (int i0 = 0; i0 < dataSize; i0++)
        {
            if (*posInFile & 1) // complete word
            {
                bufSend[1] = bufRead[i0];
                tcpSocket->write(bufSend, 2);
                tcpSocket->flush();
            }
            else
                bufSend[0] = bufRead[i0];
            *posInFile = *posInFile + 1;
            if (*stopThread)
                return;
            if (!(*posInFile & 1))
            {
                emit updateStatusBar(QString("%1").arg(*posInFile));
                if (interval == 0) // need to press Send for each word
                    return;
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
    if (*posInFile & 1) // odd number of bytes in file?
    {
        bufSend[1] = 0; // set as 0
        tcpSocket->write(bufSend, 2);
        tcpSocket->flush();
    }
}
