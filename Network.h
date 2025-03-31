#pragma once

#include <WS2tcpip.h>

#include <iostream>

#pragma comment(lib, "WS2_32.lib")


// RAII
class WsaGuard {
public:
    WsaGuard() {
        start();
    }
    ~WsaGuard() {
        end();
    }

public:
    void start() {
        WSADATA wsadata;
        if(WSAStartup(MAKEWORD(2, 0), &wsadata)) {
            std::cout << "WSAStartup failed" << std::endl;
            exit(1);
        }
    }

    void end() {
        WSACleanup();
    }
};


#pragma pack(push, 1)
struct Packet {
    char size;
    char type;
    char data[1024];

    Packet();
    Packet(char size, char type);
};
#pragma pack(pop)


// 소멸자에서 소켓을 닫음(RAII)  
class TcpConnection {
private:
    static const short SERVER_PORT = 3000;

private:
    SOCKET client_socket;

public:
    TcpConnection();
    ~TcpConnection();

public:
    bool connect(const std::string& ip);
    void close();
    void setNoBlock(bool no_block);

    // 일단 성공여부는 리턴하지 않음(무조건 성공이라고 가정)
    void send(Packet* packet);
    // 일단 성공여부는 리턴하지 않음(무조건 성공이라고 가정)
    int receive(Packet* packet/*out*/);

public:
    static void printErrorMessage(int s_err);
};