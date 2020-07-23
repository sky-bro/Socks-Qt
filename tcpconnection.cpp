#include "tcpconnection.h"

TcpConnection::TcpConnection(QObject *parent) : QObject(parent)
{
    client_socket = nullptr;
    server_socket = nullptr;
    cout << this << "created";
}

TcpConnection::~TcpConnection()
{
    cout << this << "destroyed";
}

void TcpConnection::setClientSocket(QTcpSocket *socket)
{
    client_socket = socket;
    connectionState = AUTH;
    connect(client_socket, &QTcpSocket::connected, [=](){
        cout << "client_socket:" << client_socket << " connected.";
    });
    connect(client_socket, &QTcpSocket::disconnected, [=](){
        cout << "client_socket:" << client_socket << " disconnected, closing connection...";
        close();
    });
    connect(client_socket, &QTcpSocket::readyRead, this, &TcpConnection::client_readyRead);
}

void TcpConnection::close()
{
    if (server_socket) {
        if (server_socket->isOpen()){ server_socket->disconnect(); server_socket->close();}
        server_socket->deleteLater();
        server_socket = nullptr;
    }
    if (client_socket->isOpen()) {
        client_socket->disconnect();
        client_socket->close();
    }
    client_socket->deleteLater();
    client_socket = nullptr;
}

void TcpConnection::setServerSocket(QTcpSocket *socket)
{
    server_socket = socket;
    connect(server_socket, &QTcpSocket::disconnected, [=](){
        cout << "server_socket:" << server_socket << " disconnected, closing connection...";
        close();
    });
    connect(server_socket, &QTcpSocket::readyRead, [=](){
        QByteArray data = server_socket->readAll();
        cout << server_socket << " (server_socket) received:" << "(" << data.length() << "bytes )"; //  << data;
        socks5_relay2client(&data);
    });
    connect(server_socket, &QTcpSocket::errorOccurred, this, &TcpConnection::error);
}

//void TcpConnection::client_connected()
//{
//    cout << this << "connected" << client_socket;
//}

//void TcpConnection::client_disconnected()
//{
//    cout << this << "disconnected" << client_socket;
//}

void TcpConnection::client_readyRead()
{
    if (!sender()) return;
    QByteArray data = client_socket->readAll();
    cout << client_socket << "(client_socket) received: " << "(" << data.length() << "bytes )"; // << data;
    switch (connectionState) {
    case AUTH:
    {
            cout << "connectionState: AUTH";
            socks5_auth(data);
            break;
    }
    case CMD:
    {
            cout << "connectionState: CMD";
            socks5_cmd(data);
            break;
    }
    case FORWARD:
    {
            cout << "connectionState: FORWARD";
            socks5_relay2server(&data);
            break;
    }
    }
}

void TcpConnection::error(QAbstractSocket::SocketError socketError)
{
    if(!sender()) return;
    cout << "error in socket" << sender() << " error = " << socketError;
    close();
}

void TcpConnection::socks5_auth(QByteArray &data)
{
    qint8 ver = data.at(0);
    quint8 methods_cnt = data.at(1);
    if (ver != 0x05 && ver != 0x04) {
        cout << "unsupported socks version!";
        return;
    }
    bool method_supported = false;
    Socks_Auth_Methods auth_type = NOAUTH;
    switch (ver) {
    case VERSION5: {
        cout << "socks version 5 detected!";
        for (int i = 0; i < methods_cnt; ++i) {
            if (data.at(i+2) == auth_type) {
                method_supported = true;
                break;
            }
        }
        if (method_supported) {
            switch (auth_type) {
            case NOAUTH: {
                connectionState = CMD;
                client_socket->write("\x05\x00", 2);
                cout << "responded with NOAUTH";
                break;
            }
            case USERPASS: {
                cout << "TODO: USERPASS auth method...";
                close();
                break;
            }
            }
        } else {
            cout << "auth methods not supported!";
            close();
        }
        break;
    }
    case VERSION4: {
        cout << "socks version 4 detected!";
        cout << "not implemented yet!";
        close();
        break;
    }
    }
}

void TcpConnection::socks5_cmd(QByteArray &data)
{
    // CONNECT (0x01)
//    QByteArray data = client_socket->readAll();
    if (data.at(0) != VERSION5 || data.at(1) != 0x01) {
        cout << "CMD error:" << data;
        close();
        return;
    }

    switch (data.at(3)) {
    case IPv4: {
        cout << "got addrss type: IPv4...";
        QTcpSocket* socket = new QTcpSocket;
        // extract server ip
        quint32 server_addr = 0;
        for (int i = 0; i < 4; ++i) {
            server_addr <<= 8;
            server_addr |= data.at(i+4);
        }
        // extract server port
        quint16 port = quint8(data.at(8));
        cout << "port0:" << QString::number(port, 16);
        port <<= 8;
        cout << "port:" << QString::number(port, 16) << port;
        port += quint8(data.at(9));

        socket->connectToHost(QHostAddress(server_addr), port);
        connect(socket, &QTcpSocket::connected, [=](){
            cout << "connected to server, set connectionState as FORWARD...";
            connectionState = FORWARD;
            char buf[3] = {0x05, 0x00, 0x00};
            client_socket->write(buf, 3);
            client_socket->write(data.mid(3, 7));
        });
        setServerSocket(socket);
        break;
    }
    case DOMAINNAME: {
        cout << "got addrss type: DOMAINNAME...";
        QTcpSocket* socket = new QTcpSocket;
        // extract server domainname
        quint8 domain_len = data.at(4);
        QString hostName(data.mid(5, domain_len));
        cout << "hostName:" << hostName;
        // extract server port
        quint16 port = quint8(data.at(5+domain_len));
        port <<= 8;
//        cout << "port0:" << QString::number(port, 16);
        port += quint8(data.at(6+domain_len));
        cout << "port:" << QByteArray::number(port, 16) << port;

        socket->connectToHost(hostName, port);
        connect(socket, &QTcpSocket::connected, [=](){
            cout << "connected to server, set connectionState as FORWARD...";
            connectionState = FORWARD;
            char buf[3] = {0x05, 0x00, 0x00};
            client_socket->write(buf, 3);
            client_socket->write(data.mid(3, domain_len+4));
        });
        setServerSocket(socket);
        break;
    }
    case IPv6: {
        cout << "got addrss type: IPv6...";
        QTcpSocket *socket = new QTcpSocket;
        // extract server ip
        QHostAddress server_addr(data.mid(4, 16).data());
        // extract server port
        quint16 port = quint8(data.at(20));
        port <<= 8;
        port += quint8(data.at(21));
        cout << "port:" << QString::number(port, 16) << port;

        socket->connectToHost(server_addr, port);
        connect(socket, &QTcpSocket::connected, [=](){
            cout << "connected to server, set connectionState as FORWARD...";
            connectionState = FORWARD;
            char buf[3] = {0x05, 0x00, 0x00};
            client_socket->write(buf, 3);
            client_socket->write(data.mid(3, 19));
        });
        setServerSocket(socket);
        break;
    }

    }

    // TODO: BIND (0x02), UDP Associate (0x03)

}

void TcpConnection::socks5_relay(QTcpSocket* src, QTcpSocket *dst, QByteArray *pdata)
{
    // TODO: better way to do this?
    if (pdata){
        dst->write(*pdata);
    } else {
        QByteArray data = src->readAll();
        dst->write(data);
    }
}

void TcpConnection::socks5_relay2server(QByteArray *pdata)
{
    socks5_relay(client_socket, server_socket, pdata);
}

void TcpConnection::socks5_relay2client(QByteArray *pdata)
{
    socks5_relay(server_socket, client_socket, pdata);
}
