//
// Serge Spraiter 20150608
//

#ifndef CLIENTTHREAD
#define CLIENTTHREAD

#include <QThread>
#include <QTcpSocket>
#include <QFile>

class ClientThread : public QThread
{
    Q_OBJECT

public:
    ClientThread(QObject *parent, qintptr socketDescriptor, const uint interval,
                 QFile *cpfile, quint64 *posInFile, int *stopThread);

    void run() Q_DECL_OVERRIDE;

signals:
    void error(QTcpSocket::SocketError socketError);
    void updateStatusBar(const QString &str);

private:
    qintptr socketDescriptor;
    uint interval;
    quint64 *posInFile;
    QFile *file;
    int *stopThread;
    QTcpSocket *tcpSocket;
};

#endif // CLIENTTHREAD
