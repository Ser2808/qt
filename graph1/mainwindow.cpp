//
// Serge Spraiter 20150609
//

#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphwidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), tcpSocket(0)
{
    ui->setupUi(this);
    // graph
    graph = new GraphWidget(ui->tabGraph);
    ui->tabGraph->layout()->addWidget(graph);
    // server
    tcpServer = new QTcpServer(this);
    tcpServer->setMaxPendingConnections(1);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(on_newConnection()));
}

MainWindow::~MainWindow()
{
    if (tcpSocket)
    {
        tcpSocket->close();
        delete tcpSocket;
    }
    delete tcpServer;
    delete ui;
}

void MainWindow::on_pushButton_Listen_clicked()
{
    if (ui->pushButton_Listen->text() == "Listen")
    {
        if (ui->lineEditIP->text().indexOf(':') < 0)
        {
            QMessageBox::critical(this, "Server1", "Missing ':' in IP:Port");
            return;
        }
        QStringList split = ui->lineEditIP->text().split( ":" );
        bool ok;
        quint32 port = split[1].toUInt(&ok);
        if (!ok || (port > 65535))
        {
            QMessageBox::critical(this, "Server1", "Invalid Port, must be 0..65535");
            return;
        }
        QHostAddress adr;
        if (!adr.setAddress(split[0]))
        {
            QMessageBox::critical(this, "Server1", "Invalid IP Address");
            return;
        }
        if (!tcpServer->listen(adr, port))
        {
            QMessageBox::critical(this, "Server1", "Unable to start listening");
            return;
        }
        statusBar()->showMessage(QString("Listening on %1:%2...").arg(adr.toString(), QString("%1").arg(port)));
        ui->labelIP->setEnabled(false);
        ui->lineEditIP->setEnabled(false);
        ui->checkBox_RawData->setEnabled(false);
        ui->pushButton_Listen->setText("Stop listening");
        if (ui->checkBox_Clear->isChecked())
        {
            ui->textEdit->clear();
            graph->clear();
        }
        QString str;
        if (ui->checkBox_RawData->isChecked())
            str = QString("Address  00   01   02   03  . 04   05   06   07  ");
        else
            str = QString("Address  00     01     02     03    . 04     05     06     07    ");
        // set size
        QFontMetrics fm(ui->textEdit->font());
        int width = fm.width(str + "000"); // 000-diff is not working?
        this->resize(width + 48, this->height()); // 48-diff is not working?
        ui->textEdit->append(str);
        ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
        ui->tabWidget->setCurrentIndex(1);  // tabLog
    }
    else
    {
        if (tcpServer && tcpServer->isListening())
        {
            if (tcpSocket)
                tcpSocket->disconnectFromHost();
            tcpServer->close();
            statusBar()->showMessage("Stops listening");
        }
        ui->labelIP->setEnabled(true);
        ui->lineEditIP->setEnabled(true);
        ui->checkBox_RawData->setEnabled(true);
        ui->pushButton_Listen->setText("Listen");
    }
}

void MainWindow::on_newConnection()
{
    if (tcpSocket)
    {
        tcpSocket->disconnectFromHost();   // only 1 connection
        delete tcpSocket;
        tcpSocket = 0;
    }
    tcpSocket = tcpServer->nextPendingConnection();
    adr = 0;
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(on_readyRead()));
    statusBar()->showMessage("Client connected");
}

void MainWindow::on_disconnected()
{
    statusBar()->showMessage("Client disconnected");
}

QString MainWindow::getDataString(quint16 val)
{
    if (ui->checkBox_RawData->isChecked())
        return (QString(" %1").arg(val, 4, 16, QChar('0')));
    else
    {
        if (val & 0x8000) // twos complement
        {
            quint16 tmp = (val & 0x7fff) + 1;
            return (QString(" -%1").arg(tmp, -5));
        }
        else
            return (QString(" +%1").arg(val, -5));
    }
}

void MainWindow::on_readyRead()
{
    QByteArray byteArray;
    while (tcpSocket->bytesAvailable())
    {
        byteArray = tcpSocket->readAll();
        QString str;
        quint16 val, tmp, pos;
        for (int i0 = 0; i0 < byteArray.size(); i0++)
        {
            tmp = byteArray[i0] & 0xff;
            if (adr & 1) // little endian
            {
                val |= (tmp << 8);
                pos = (quint16)(adr >> 1);
                if (!(pos & 0x07))
                    str.append(QString("\n%1").arg(pos, 8, 16, QChar('0')));
                else if (!(pos & 0x03))
                    str.append(" ");
                str.append(getDataString(val));
                graph->addData(val);
            }
            else
                val = tmp;
            adr++;
        }
        ui->textEdit->insertPlainText(str);
        ui->textEdit->verticalScrollBar()->setValue(ui->textEdit->verticalScrollBar()->maximum());
    }
}
