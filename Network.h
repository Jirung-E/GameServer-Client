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


// �Ҹ��ڿ��� ������ ����(RAII)  
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

    // WSASend�� ����� ����
    int send(Packet* packet);
    // WSARecv�� ����� ����
    int receive(char* buf/*out*/, ULONG size, DWORD* size_recv/*out*/);

public:
    static void printErrorMessage(int s_err);
};