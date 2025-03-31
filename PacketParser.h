#pragma once

#include <vector>
#include <queue>

#include "Network.h"


class PacketParser {
private:
    std::vector<char> bytes;
    std::queue<Packet> packets;

public:
    PacketParser();
    
public:
    void push(char* data, size_t data_size);
    Packet pop();
    bool canPop();
};
