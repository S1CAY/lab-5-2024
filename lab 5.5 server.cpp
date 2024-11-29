#include <iostream>
#include <string>
#include <format>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <json/json.h>
#pragma comment(lib, "Ws2_32.lib")

int main()
{
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

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(27015);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    iResult = bind(ListenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
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

    SOCKET ClientSocket = accept(ListenSocket, nullptr, nullptr);
    if (ClientSocket == INVALID_SOCKET) {
        std::cerr << std::format("accept не вдалося: {}", WSAGetLastError()) << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Клієнт підключено." << std::endl;

    const int recvbuflen = 512;
    char recvbuf[recvbuflen];

    iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0) {
        std::string receivedData(recvbuf, iResult);

        Json::CharReaderBuilder readerBuilder;
        Json::Value root;
        std::string errors;

        std::istringstream iss(receivedData);
        if (Json::parseFromStream(readerBuilder, iss, &root, &errors)) {
            std::cout << "Отримані дані:" << std::endl;
            std::cout << root.toStyledString() << std::endl;

            if (root.isMember("name") && root.isMember("age")) {
                std::string name = root["name"].asString();
                int age = root["age"].asInt();
                std::cout << std::format("Ім'я: {}, Вік: {}", name, age) << std::endl;
            }
        } else {
            std::cerr << "Помилка при парсингу JSON: " << errors << std::endl;
        }
    } else if (iResult == 0) {
        std::cout << "З'єднання закрито клієнтом." << std::endl;
    } else {
        std::cerr << std::format("recv не вдалося: {}", WSAGetLastError()) << std::endl;
    }

    closesocket(ClientSocket);
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}
