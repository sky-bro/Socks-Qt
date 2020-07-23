#include "tcpconnections.h"

TcpConnections::TcpConnections(QObject *parent) : QObject(parent)
{
    cout << this << "created";
}

TcpConnections::~TcpConnections()
{
    cout << this << "destroyed";
}

int TcpConnections::count()
{
    return m_connections.count();
}

void TcpConnections::removeSocket(QTcpSocket *socket)
{
//    cout << socket << m_connections;
    if (!socket || !m_connections.contains(socket)) return;
    cout << "closing connection:" << m_connections[socket];

    m_connections[socket]->close();
    cout << "removing socket from connections:" << socket;
    m_connections.remove(socket);

    cout << "client count:" << count();
}

void TcpConnections::disconnected()
{
    if (!sender()) return;
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    if (!socket) return;
    cout << this << "disconnecting socket:" << socket;
    removeSocket(socket);
}

void TcpConnections::error(QAbstractSocket::SocketError socketError)
{
    if(!sender()) return;
    cout << "error in socket" << sender() << " error = " << socketError;

    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
//    if(!socket) return;

    removeSocket(socket);
}

void TcpConnections::start()
{
    cout << "starting tcpconnections:" << this;
}

void TcpConnections::quit()
{
    if (!sender()) return;
    cout << this << "quitting...";
    foreach(QTcpSocket *socket, m_connections.keys()) {
        cout << this << "closing socket" << socket;
        removeSocket(socket);
    }
    emit finished();
}

void TcpConnections::accept(qintptr socketDescriptor, TcpConnection *connection)
{
    cout << "accepting new connection";
    QTcpSocket *socket = new QTcpSocket(this);
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        cout << this << "could not accept connection?" << socketDescriptor;
        connection->deleteLater();
        return;
    }

    connect(socket, &QTcpSocket::disconnected, this, &TcpConnections::disconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &TcpConnections::error);

    connection->setClientSocket(socket);
    m_connections.insert(socket, connection);
    cout << this << "now have " << count() << "clients.";
//    emit socket->connected(); ?
}
