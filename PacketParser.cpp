#include "PacketParser.h"


PacketParser::PacketParser(): bytes { } {
    bytes.reserve(1024);
}

void PacketParser::push(char* data, size_t data_size) {
    if(data_size <= 0) {
        return;
    }

    size_t offset = 0;
    while(offset < data_size) {
        size_t packet_size = static_cast<size_t>(data[offset]);

        if(packet_size > data_size - offset) {
            for(size_t i=offset; i<data_size; ++i) {
                bytes.push_back(data[i]);
            }
            break;
        }

        packets.emplace(packet_size, data[offset + 1]);
        memcpy(packets.back().data, data + offset + 2, packet_size - 2);

        offset += packet_size;
    }
}

Packet PacketParser::pop() {
    if(packets.empty()) {
        throw std::out_of_range("No packets to pop");
    }
    Packet packet = std::move(packets.front());
    packets.pop();
    return packet;
}

bool PacketParser::canPop() {
    return !packets.empty();
}
