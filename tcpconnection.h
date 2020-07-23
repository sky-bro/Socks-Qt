#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QtEndian>

#define cout qDebug() << "(" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__ << "," << this << ")" << ": "

enum ConnectionState {
    AUTH,
    CMD,
    FORWARD
};

enum Socks_Version { VERSION4 = 0x04, VERSION5 = 0x05 };

enum Socks_Auth_Methods {NOAUTH = 0x00, USERPASS = 0x02};

enum Addr_Type { IPv4 = 0x01, DOMAINNAME = 0x03, IPv6 = 0x04 };

class TcpConnection : public QObject
{
    Q_OBJECT
public:
    explicit TcpConnection(QObject *parent = nullptr);
    ~TcpConnection();

    virtual void setClientSocket(QTcpSocket *socket);

    virtual void close();

protected:
    QTcpSocket *client_socket;
    QTcpSocket *server_socket;
    ConnectionState connectionState;

    virtual void setServerSocket(QTcpSocket *socket);

signals:

public slots:
//    virtual void client_connected();
//    virtual void client_disconnected();
    virtual void client_readyRead();
//    virtual void bytesWritten(quint64 bytes);
//    virtual void stateChanged(ConnectionStates connectionState);
    virtual void error(QAbstractSocket::SocketError socketError);

private:
    void socks5_auth(QByteArray &data);
    void socks5_cmd(QByteArray &data);
    void socks5_relay(QTcpSocket* src, QTcpSocket *dst, QByteArray *pdata=nullptr);
    void socks5_relay2server(QByteArray *pdata=nullptr);
    void socks5_relay2client(QByteArray *pdata=nullptr);
};

#endif // TCPCONNECTION_H
