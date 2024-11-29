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

    SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << std::format("socket не вдалося: {}", WSAGetLastError()) << std::endl;
        WSACleanup();
        return 1;
    }

    std::string serverAddress;
    std::cout << "Введіть IP-адресу сервера: ";
    std::getline(std::cin, serverAddress);

    sockaddr_in serverInfo{};
    serverInfo.sin_family = AF_INET;
    inet_pton(AF_INET, serverAddress.c_str(), &(serverInfo.sin_addr));
    serverInfo.sin_port = htons(27015);

    iResult = connect(ConnectSocket, (SOCKADDR*)&serverInfo, sizeof(serverInfo));
    if (iResult == SOCKET_ERROR) {
        std::cerr << std::format("connect не вдалося: {}", WSAGetLastError()) << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Підключено до сервера." << std::endl;

    Json::Value root;
    root["name"] = "Іван";
    root["age"] = 30;
    root["skills"] = Json::arrayValue;
    root["skills"].append("C++");
    root["skills"].append("Python");
    root["skills"].append("JavaScript");

    Json::StreamWriterBuilder writer;
    std::string serializedData = Json::writeString(writer, root);

    iResult = send(ConnectSocket, serializedData.c_str(), static_cast<int>(serializedData.length()), 0);
    if (iResult == SOCKET_ERROR) {
        std::cerr << std::format("send не вдалося: {}", WSAGetLastError()) << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Дані відправлено на сервер." << std::endl;

    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
