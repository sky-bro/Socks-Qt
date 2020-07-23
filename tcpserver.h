#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QDebug>
#include <QThread>

#include "tcpconnections.h"
#include "tcpconnection.h"

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);
    ~TcpServer();

    virtual bool listen(const QHostAddress &addr, quint16 port);
    virtual void close();
    virtual qint64 port();

protected:
    QThread *m_thread;
    TcpConnections *m_connections;
    virtual void incomingConnection(qintptr descriptor);
    virtual void accept(qintptr descriptor, TcpConnection *connection);

signals:
    void accepting(qintptr socketDescriptor, TcpConnection *connection);
    void closing();

public slots:
    void finished();
};

#endif // TCPSERVER_H
