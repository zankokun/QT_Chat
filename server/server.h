#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QVector>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioOutput>

class ClientConnection;

class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server(QObject *parent = nullptr);
    ~Server();

    void startServer(quint16 port);
    void broadcastTextMessage(const QString &message, ClientConnection *sender);
    void broadcastVoiceMessage(const QByteArray &audioData, ClientConnection *sender);
    void startVoiceCall(ClientConnection *caller, const QString &calleeName);
    void endVoiceCall(ClientConnection *caller);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientDisconnected(ClientConnection *client);

private:
    QVector<ClientConnection*> clients;
    QHash<QString, ClientConnection*> activeCalls;
};

#endif // SERVER_H