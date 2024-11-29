#include <iostream>
#include <string>
#include <format>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

int main()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::cerr << std::format("WSAStartup не вдалося: {}", iResult) << std::endl;
        return 1;
    }

    SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << std::format("socket не вдалося: {}", WSAGetLastError()) << std::endl;
        WSACleanup();
        return 1;
    }

    std::string serverAddress;
    std::cout << "Введіть IP-адресу сервера: ";
    std::getline(std::cin, serverAddress);

    std::string portString;
    std::cout << "Введіть порт сервера: ";
    std::getline(std::cin, portString);
    int port = std::stoi(portString);

    sockaddr_in serverInfo{};
    serverInfo.sin_family = AF_INET;
    inet_pton(AF_INET, serverAddress.c_str(), &(serverInfo.sin_addr));
    serverInfo.sin_port = htons(port);

    iResult = connect(ConnectSocket, (SOCKADDR*)&serverInfo, sizeof(serverInfo));
    if (iResult == SOCKET_ERROR) {
        std::cerr << std::format("connect не вдалося: {}", WSAGetLastError()) << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Підключено до сервера." << std::endl;

    std::string sendbuf;
    std::cout << "Введіть повідомлення для відправки: ";
    std::getline(std::cin, sendbuf);

    iResult = send(ConnectSocket, sendbuf.c_str(), static_cast<int>(sendbuf.length()), 0);
    if (iResult == SOCKET_ERROR) {
        std::cerr << std::format("send не вдалося: {}", WSAGetLastError()) << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << std::format("Відправлено байтів: {}", iResult) << std::endl;

    const int recvbuflen = 512;
    char recvbuf[recvbuflen];

    iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0) {
        std::cout << std::format("Отримано байтів: {}", iResult) << std::endl;
        std::cout << "Отримане повідомлення: ";
        std::cout.write(recvbuf, iResult);
        std::cout << std::endl;
    } else if (iResult == 0) {
        std::cout << "З'єднання закрито" << std::endl;
    } else {
        std::cerr << std::format("recv не вдалося: {}", WSAGetLastError()) << std::endl;
    }

    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
