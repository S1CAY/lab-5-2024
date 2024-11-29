#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

void handleClient(SOCKET clientSocket) {
    char buffer[1024];
    while (true) {
        ZeroMemory(buffer, 1024);
        int bytesReceived = recv(clientSocket, buffer, 1024, 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) break;
        std::cout << "Отримано від клієнта: " << std::string(buffer, 0, bytesReceived) << "\n";
        std::string response = "Сервер отримав ваші дані: " + std::string(buffer, 0, bytesReceived);
        send(clientSocket, response.c_str(), response.size() + 1, 0);
    }
    closesocket(clientSocket);
    std::cout << "З'єднання з клієнтом закрито.\n";
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

    SOCKET listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(54000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(listeningSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        closesocket(listeningSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listeningSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Сервер запущено. Очікування з'єднань на порту 54000...\n";

    while (true) {
        sockaddr_in clientAddress;
        int clientSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(listeningSocket, (sockaddr*)&clientAddress, &clientSize);
        if (clientSocket == INVALID_SOCKET) continue;

        char clientIP[NI_MAXHOST];
        ZeroMemory(clientIP, NI_MAXHOST);
        inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, NI_MAXHOST);
        std::cout << "Клієнт підключився з IP-адресою: " << clientIP << " і портом: " << ntohs(clientAddress.sin_port) << "\n";

        handleClient(clientSocket);
    }

    closesocket(listeningSocket);
    WSACleanup();
    return 0;
}
