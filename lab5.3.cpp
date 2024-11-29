#include <iostream>
#include <string>
#include <format>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#pragma comment(lib, "Ws2_32.lib")

unsigned __stdcall ClientHandler(void* data);
CRITICAL_SECTION cs;
int clientCount = 0;

int main()
{
    InitializeCriticalSection(&cs);

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::cerr << std::format("WSAStartup не вдалося: {}", iResult) << std::endl;
        return 1;
    }

    SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << std::format("socket не вдалося: {}", WSAGetLastError()) << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverInfo{};
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(27015);

    iResult = bind(ListenSocket, (SOCKADDR*)&serverInfo, sizeof(serverInfo));
    if (iResult == SOCKET_ERROR) {
        std::cerr << std::format("bind не вдалося: {}", WSAGetLastError()) << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cerr << std::format("listen не вдалося: {}", WSAGetLastError()) << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Сервер запущено та очікує з'єднань..." << std::endl;

    while (true) {
        SOCKET ClientSocket = accept(ListenSocket, nullptr, nullptr);
        if (ClientSocket == INVALID_SOCKET) {
            std::cerr << std::format("accept не вдалося: {}", WSAGetLastError()) << std::endl;
            closesocket(ListenSocket);
            WSACleanup();
            DeleteCriticalSection(&cs);
            return 1;
        }

        EnterCriticalSection(&cs);
        clientCount++;
        std::cout << std::format("Клієнт підключено. Кількість клієнтів: {}", clientCount) << std::endl;
        LeaveCriticalSection(&cs);

        uintptr_t hThread = _beginthreadex(nullptr, 0, ClientHandler, (void*)ClientSocket, 0, nullptr);
        if (hThread == 0) {
            std::cerr << "Не вдалося створити потік для клієнта." << std::endl;
            closesocket(ClientSocket);
        } else {
            CloseHandle((HANDLE)hThread);
        }
    }

    closesocket(ListenSocket);
    WSACleanup();
    DeleteCriticalSection(&cs);

    return 0;
}

unsigned __stdcall ClientHandler(void* data)
{
    SOCKET ClientSocket = (SOCKET)data;
    const int recvbuflen = 512;
    char recvbuf[recvbuflen];

    int iResult;
    do {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            send(ClientSocket, recvbuf, iResult, 0);
        } else if (iResult == 0) {
            std::cout << "З'єднання закрито клієнтом." << std::endl;
        } else {
            std::cerr << std::format("recv не вдалося: {}", WSAGetLastError()) << std::endl;
        }
    } while (iResult > 0);

    closesocket(ClientSocket);

    EnterCriticalSection(&cs);
    clientCount--;
    std::cout << std::format("Клієнт відключено. Кількість клієнтів: {}", clientCount) << std::endl;
    LeaveCriticalSection(&cs);

    return 0;
}
