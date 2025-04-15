#include "clientconnection.h"
#include <QDataStream>

ClientConnection::ClientConnection(QObject *parent) : QTcpSocket(parent), m_blockSize(0)
{
    connect(this, &QTcpSocket::readyRead, this, &ClientConnection::onReadyRead);
}

ClientConnection::~ClientConnection()
{
}

void ClientConnection::sendTextMessage(const QString &sender, const QString &message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(0) << quint8(1) << sender << message;
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));
    write(block);
}

void ClientConnection::sendVoiceMessage(const QString &sender, const QByteArray &audioData)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(0) << quint8(2) << sender << audioData;
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));
    write(block);
}

void ClientConnection::sendCallRequest(const QString &caller)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(0) << quint8(3) << caller;
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));
    write(block);
}

void ClientConnection::sendCallResponse(bool accepted, const QString &callee)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(0) << quint8(4) << accepted << callee;
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));
    write(block);
}

void ClientConnection::sendCallEnded(const QString &peer)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(0) << quint8(5) << peer;
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));
    write(block);
}

void ClientConnection::sendVoiceData(const QByteArray &data)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint32(0) << quint8(6) << data;
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));
    write(block);
}

void ClientConnection::onReadyRead()
{
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_6_0);

    if (m_blockSize == 0)
    {
        if (bytesAvailable() < sizeof(quint32))
            return;
        in >> m_blockSize;
    }

    if (bytesAvailable() < m_blockSize)
        return;

    quint8 type;
    in >> type;

    switch (type)
    {
    case 1:
    { // Text message
        QString sender, message;
        in >> sender >> message;
        emit textMessageReceived(sender, message);
        break;
    }
    case 2:
    { // Voice message
        QString sender;
        QByteArray audioData;
        in >> sender >> audioData;
        emit voiceMessageReceived(sender, audioData);
        break;
    }
    case 3:
    { // Call request
        QString caller;
        in >> caller;
        emit callRequestReceived(caller);
        break;
    }
    case 4:
    { // Call response
        bool accepted;
        QString callee;
        in >> accepted >> callee;
        emit callResponseReceived(accepted, callee);
        break;
    }
    case 5:
    { // Call ended
        QString peer;
        in >> peer;
        emit callEnded(peer);
        break;
    }
    case 6:
    { // Voice data
        QByteArray data;
        in >> data;
        emit voiceDataReceived(data);
        break;
    }
    default:
        qDebug() << "Unknown message type";
    }

    m_blockSize = 0;
    if (bytesAvailable() > 0)
        onReadyRead();
}