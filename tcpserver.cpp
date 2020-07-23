#include "tcpserver.h"


TcpServer::TcpServer(QObject *parent): QTcpServer(parent)
{
    cout << "creating tcpserver:" << this << "on" << QThread::currentThread();
}

TcpServer::~TcpServer()
{
    cout << this << "destroyed";
}

bool TcpServer::listen(const QHostAddress &addr, quint16 port)
{
    if (!QTcpServer::listen(addr, port)) return false;

    m_thread = new QThread(this);
    m_connections = new TcpConnections();

    connect(m_thread, &QThread::started, m_connections, &TcpConnections::start, Qt::QueuedConnection);
    connect(this, &TcpServer::accepting, m_connections, &TcpConnections::accept, Qt::QueuedConnection);
    connect(this, &TcpServer::closing, m_connections, &TcpConnections::quit, Qt::QueuedConnection);
    connect(m_connections, &TcpConnections::finished, this, &TcpServer::finished, Qt::QueuedConnection);

    m_connections->moveToThread(m_thread);
    m_thread->start();

    return true;
}

void TcpServer::close()
{
    cout << "closing server" << this;
    emit closing();
    QTcpServer::close();
}

qint64 TcpServer::port()
{
    if (isListening()) return this->serverPort();
    return 0; // ?
}

void TcpServer::incomingConnection(qintptr descriptor)
{
    TcpConnection *connection = new TcpConnection();
    accept(descriptor, connection);
}

void TcpServer::accept(qintptr descriptor, TcpConnection *connection)
{
    connection->moveToThread(m_thread);
    emit accepting(descriptor, connection);
}

void TcpServer::finished()
{
    if (!m_thread) return;
    delete m_connections;
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
}


