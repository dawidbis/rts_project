#pragma once

#include <enet/enet.h>

#include <memory>
#include <span>

namespace rts::network {

// Bezpieczny RAII wrapper wokó³ ENetPacket do czyszczenia zasobów
struct ENetPacketDeleter {
    void operator()(ENetPacket* packet) const {
        if (packet && packet->referenceCount == 0) {
            enet_packet_destroy(packet);
        }
    }
};

using ENetPacketPtr = std::unique_ptr<ENetPacket, ENetPacketDeleter>;

class ENetFactory {
public:
    // Tworzy pakiet ENet z bufora FlatBuffers z odpowiednimi flagami (Reliable vs Unreliable)
    static ENetPacket* CreatePacket(std::span<const uint8_t> data, uint32_t flags) {
        return enet_packet_create(data.data(), data.size(), flags);
    }

    // Tworzy zawodny (unreliable) pakiet UDP dla ci¹g³ego broadcastu 10Hz 
    static ENetPacket* CreateStateBroadcast(std::span<const uint8_t> data) {
        // Zgodnie z M-TRA-01: broadcast UDP jest zawodny, flagi na 0
        return enet_packet_create(data.data(), data.size(), 0);
    }
};

}  // namespace rts::network