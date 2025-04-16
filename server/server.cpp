#define WIN32_LEAN_AND_MEAN // макрос для сетевых вещей

#include <iostream>
#include <thread>
#include <chrono>

#include <vector>
#include <string>
#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

WSADATA wsaData;
ADDRINFO hints;
ADDRINFO *addrResult = NULL;
SOCKET ClientSocket = INVALID_SOCKET;
SOCKET ListenSocket = INVALID_SOCKET;

int result;
int massa[];
const long MAX = 20000000;
vector<char> recvBuffer(MAX);
string sendBuffer = "Hello from SERVER";

vector<bool> clientplace;
int sizeofclients = 0;
vector<thread> CLIENT;
vector<SOCKET> SOCKETS;

int recv_size;

bool receive(SOCKET ClientSocket)
{
    int number = 0;
    DWORD timeout = 1 * 100;

    fill(recvBuffer.begin(), recvBuffer.end(), 0);

    setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    while (1)
    {
        using namespace chrono;
        this_thread::sleep_for(250ms);
        recv_size = recv(ClientSocket, recvBuffer.data(), MAX, 0);
        if (recv_size == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSAETIMEDOUT)
            {
                cout << "Closing the client (Error with setsockopt)" << endl;
                closesocket(ClientSocket);
                return 1;
            }
        }
        if (recv_size > 0)
        {
            cout << " Size message: " << recv_size - 2 << endl;
            sendBuffer.assign(recvBuffer.begin(), recvBuffer.begin() + recv_size);
            number = 0;
            while (number < sizeofclients)
            {
                if (ClientSocket != SOCKETS[number])
                {
                    send(SOCKETS[number], sendBuffer.c_str(), (int)(sendBuffer.size()), 0);
                }
                number++;
            }
        }
    }
}

void work(SOCKET ClientSocket)
{
    if (result == SOCKET_ERROR)
    {
        cout << "Error sending data back to the client" << endl;
        closesocket(ClientSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return;
    }

    receive(ClientSocket);
}

bool listening(SOCKET ClientSocket)
{
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0)
    {
        cout << "Error with WSAStartup" << endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo(NULL, "2626", &hints, &addrResult);

    if (result != 0)
    {
        cout << "Error with getaddrinfo" << endl;
        WSACleanup();
        return 1;
    }

    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        cout << "Error with socket creating" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);

    if (result == SOCKET_ERROR)
    {
        cout << "Error with socket binding" << endl;
        closesocket(ListenSocket);
        ListenSocket = INVALID_SOCKET;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    result = listen(ListenSocket, SOMAXCONN);

    if (result == SOCKET_ERROR)
    {
        cout << "Error with listening the socket" << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    while (1)
    {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        thread T(work, ClientSocket);
        CLIENT.push_back(std::move(T));
        SOCKETS.push_back(ClientSocket);
        clientplace.push_back(true);
        sizeofclients++;
    }
}

int main()
{
    setlocale(LC_ALL, "Russian");

    listening(ClientSocket);

    if (ClientSocket == INVALID_SOCKET)
    {
        cout << "Error accepting the socket" << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    closesocket(ListenSocket);

    result = shutdown(ClientSocket, SD_SEND); // закрываем клиент-сокет

    if (result == SOCKET_ERROR)
    {
        cout << "Error with client socket shutdown" << endl;
        closesocket(ClientSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    closesocket(ClientSocket);
    freeaddrinfo(addrResult);
    WSACleanup();
    return 0;
}