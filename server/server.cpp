#include "server.h"
#include "clientconnection.h"

Server::Server(QObject *parent) : QTcpServer(parent)
{
}

Server::~Server()
{
    for (auto client : clients)
    {
        client->disconnectFromHost();
    }
}

void Server::startServer(quint16 port)
{
    if (!this->listen(QHostAddress::Any, port))
    {
        qDebug() << "Could not start server";
    }
    else
    {
        qDebug() << "Listening to port" << port << "...";
    }
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    ClientConnection *client = new ClientConnection(this);
    client->setSocketDescriptor(socketDescriptor);

    connect(client, &ClientConnection::disconnected, this, [this, client]()
            { onClientDisconnected(client); });

    clients.append(client);
    qDebug() << "New client connected:" << client->getName();
}

void Server::onClientDisconnected(ClientConnection *client)
{
    clients.removeAll(client);
    endVoiceCall(client);
    client->deleteLater();
    qDebug() << "Client disconnected:" << client->getName();
}

void Server::broadcastTextMessage(const QString &message, ClientConnection *sender)
{
    for (auto client : clients)
    {
        if (client != sender)
        {
            client->sendTextMessage(sender->getName(), message);
        }
    }
}

void Server::broadcastVoiceMessage(const QByteArray &audioData, ClientConnection *sender)
{
    for (auto client : clients)
    {
        if (client != sender)
        {
            client->sendVoiceMessage(sender->getName(), audioData);
        }
    }
}

void Server::startVoiceCall(ClientConnection *caller, const QString &calleeName)
{
    for (auto client : clients)
    {
        if (client->getName() == calleeName)
        {
            activeCalls.insert(caller->getName(), caller);
            activeCalls.insert(calleeName, client);

            client->sendCallRequest(caller->getName());
            caller->sendCallResponse(true, calleeName);
            return;
        }
    }

    caller->sendCallResponse(false, calleeName);
}

void Server::endVoiceCall(ClientConnection *caller)
{
    if (activeCalls.contains(caller->getName()))
    {
        ClientConnection *callee = activeCalls.value(caller->getName());
        if (callee)
        {
            callee->sendCallEnded(caller->getName());
        }

        activeCalls.remove(caller->getName());
        if (callee)
        {
            activeCalls.remove(callee->getName());
            caller->sendCallEnded(callee->getName());
        }
    }
}