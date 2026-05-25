#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <vector>
#include <flatbuffers/flatbuffers.h>

namespace rts::network {

enum class CommandType : uint8_t { Attack, Retreat, CounterAttack, SetRatio, BuyUrbanization };

struct InternalCommand {
    uint16_t player_id;
    CommandType type;
    uint16_t target_player_id;
    int32_t value;
};

class Serializer {
public:
    static bool VerifyTcpMessage(std::span<const uint8_t> buffer);
    static bool VerifyUdpPacket(std::span<const uint8_t> buffer);

    static std::optional<InternalCommand> DeserializeTcpCommand(std::span<const uint8_t> buffer);

    static std::vector<uint8_t> FinalizeBuilder(flatbuffers::FlatBufferBuilder& builder);
};

} // namespace rts::network