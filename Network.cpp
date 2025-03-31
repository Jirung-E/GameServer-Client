#include "Network.h"

#include <iostream>


TcpConnection::TcpConnection():
    client_socket { }
{

}

TcpConnection::~TcpConnection() {
    close();
}

bool TcpConnection::connect(const std::string& ip) {
    // WSAStartup 이후에 소켓 생성
    client_socket = WSASocket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP,
        NULL,
        NULL,
        NULL
    );

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    auto res = WSAConnect(
        client_socket,
        reinterpret_cast<sockaddr*>(&addr),
        sizeof(SOCKADDR_IN),
        NULL,
        NULL,
        NULL,
        NULL
    );
    if(res == SOCKET_ERROR) {
        printErrorMessage(WSAGetLastError());
        exit(1);
    }
    std::cout << "connected" << std::endl;

    return true;
}

void TcpConnection::close() {
    closesocket(client_socket);
}

void TcpConnection::setNoBlock(bool no_block) {
    u_long noblock = static_cast<u_long>(no_block);
    int n_ret = ioctlsocket(client_socket, FIONBIO, &noblock);
}


void TcpConnection::send(Packet* packet) {
    // WSABUF는 나중에 분리해야함
    WSABUF wsabuf[1];
    wsabuf[0].buf = reinterpret_cast<char*>(packet);
    wsabuf[0].len = static_cast<ULONG>(packet->size);
    DWORD size_sent;
    WSASend(client_socket, wsabuf, 1, &size_sent, NULL, NULL, NULL);
}

int TcpConnection::receive(Packet* packet) {
    // WSABUF는 나중에 분리해야함
    WSABUF recv_wsabuf[1];
    recv_wsabuf[0].buf = reinterpret_cast<char*>(packet);
    recv_wsabuf[0].len = sizeof(Packet);
    DWORD size_recv;
    DWORD recv_flag = 0;
    return WSARecv(client_socket, recv_wsabuf, 1, &size_recv, &recv_flag, NULL, NULL);
}


void TcpConnection::printErrorMessage(int s_err) {
    WCHAR* lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, s_err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    std::wcout << lpMsgBuf << std::endl;
    while(true); // 디버깅 용
    LocalFree(lpMsgBuf);
}

Packet::Packet():
    size { 0 },
    type { 0 },
    data { }
{

}

Packet::Packet(char size, char type):
    size { size },
    type { type },
    data { }
{

}
