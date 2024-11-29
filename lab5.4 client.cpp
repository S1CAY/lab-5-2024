#include <iostream>
#include <string>
#include <format>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#pragma comment(lib, "Ws2_32.lib")

void ReceiveMessages(SOCKET clientSocket, sockaddr_in serverAddr) {
    sockaddr_in fromAddr{};
    int fromAddrSize = sizeof(fromAddr);
    const int bufLen = 512;
    char recvBuf[bufLen];

    while (true) {
        int bytesReceived = recvfrom(clientSocket, recvBuf, bufLen, 0, (SOCKADDR*)&fromAddr, &fromAddrSize);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << std::format("recvfrom failed: {}", WSAGetLastError()) << std::endl;
            break;
        }

        std::string receivedMessage(recvBuf, bytesReceived);
        std::cout << std::format("\nОтримано: {}", receivedMessage) << std::endl;
        std::cout << "Введіть повідомлення: ";
        std::cout.flush();
    }
}

int main()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::cerr << std::format("WSAStartup failed: {}", iResult) << std::endl;
        return 1;
    }

    SOCKET ClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ClientSocket == INVALID_SOCKET) {
        std::cerr << std::format("socket failed: {}", WSAGetLastError()) << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    std::string serverIp;
    std::cout << "Введіть IP-адресу сервера: ";
    std::getline(std::cin, serverIp);
    inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(27015);


    std::thread recvThread(ReceiveMessages, ClientSocket, serverAddr);

    std::string message;
    while (true) {
        std::cout << "Введіть повідомлення: ";
        std::getline(std::cin, message);

        if (message == "/exit") {
            break;
        }

        int bytesSent = sendto(ClientSocket, message.c_str(), static_cast<int>(message.length()), 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << std::format("sendto failed: {}", WSAGetLastError()) << std::endl;
            break;
        }
    }

    closesocket(ClientSocket);
    WSACleanup();


    recvThread.detach();

    return 0;
}
