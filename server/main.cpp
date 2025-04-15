#include "server.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server server;
    server.startServer(12345); // Порт можно изменить

    return a.exec();
}