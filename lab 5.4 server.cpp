#include <iostream>
#include <string>
#include <format>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#pragma comment(lib, "Ws2_32.lib")

int main()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::cerr << std::format("WSAStartup failed: {}", iResult) << std::endl;
        return 1;
    }

    SOCKET ServerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ServerSocket == INVALID_SOCKET) {
        std::cerr << std::format("socket failed: {}", WSAGetLastError()) << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(27015);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    iResult = bind(ServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        std::cerr << std::format("bind failed: {}", WSAGetLastError()) << std::endl;
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "UDP-сервер запущено та очікує повідомлень..." << std::endl;

    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);
    const int bufLen = 512;
    char recvBuf[bufLen];

    while (true) {
        int bytesReceived = recvfrom(ServerSocket, recvBuf, bufLen, 0, (SOCKADDR*)&clientAddr, &clientAddrSize);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << std::format("recvfrom failed: {}", WSAGetLastError()) << std::endl;
            break;
        }

        std::string receivedMessage(recvBuf, bytesReceived);
        char clientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, INET_ADDRSTRLEN);

        std::cout << std::format("Повідомлення від {}: {}", clientIp, receivedMessage) << std::endl;

        // Відправка відповіді клієнту (ехо)
        int bytesSent = sendto(ServerSocket, recvBuf, bytesReceived, 0, (SOCKADDR*)&clientAddr, clientAddrSize);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << std::format("sendto failed: {}", WSAGetLastError()) << std::endl;
            break;
        }
    }

    closesocket(ServerSocket);
    WSACleanup();

    return 0;
}

