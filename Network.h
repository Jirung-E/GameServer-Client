#pragma once

#include <WS2tcpip.h>

#include <iostream>

#include "../GameServer-Server/game_header.h"

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

    // WSASend의 결과를 리턴
    int send(Packet* packet);
    // WSARecv의 결과를 리턴
    int receive(char* buf/*out*/, ULONG size, DWORD* size_recv/*out*/);

public:
    static void printErrorMessage(int s_err);
};