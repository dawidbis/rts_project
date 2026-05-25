#include "network/Serializer.hpp"
#include "../fbs/rts_protocol_generated.hpp"

namespace rts::network {

bool Serializer::VerifyTcpMessage(std::span<const uint8_t> buffer) {
    flatbuffers::Verifier v(buffer.data(), buffer.size());
    return v.VerifyBuffer<TcpMessage>(nullptr);
}

bool Serializer::VerifyUdpPacket(std::span<const uint8_t> buffer) {
    flatbuffers::Verifier v(buffer.data(), buffer.size());
    return v.VerifyBuffer<UdpStatePacket>(nullptr);
}

std::optional<InternalCommand> Serializer::DeserializeTcpCommand(std::span<const uint8_t> buffer) {
    if (!VerifyTcpMessage(buffer)) return std::nullopt;

    auto msg = flatbuffers::GetRoot<TcpMessage>(buffer.data());

    // U¿ywamy wygenerowanego enuma TcpPayload
    if (msg->payload_type() != TcpPayload_PlayerCommand) return std::nullopt;

    // Pobieramy tabelê PlayerCommand
    auto cmd = msg->payload_as_PlayerCommand();
    if (!cmd) return std::nullopt;

    InternalCommand internal{};
    internal.player_id = cmd->player_id();

    // W Twoim wygenerowanym kodzie unia jest wewn¹trz struktury TcpPayloadUnion
    // Ale w PlayerCommand (tabeli) unia jest sp³aszczona.
    // Sprawdzamy typ przez payload_type()
    auto type = cmd->payload_type();

    switch (type) {
        case CommandPayload_AttackCmd: {
            auto p = cmd->payload_as_AttackCmd();
            internal.type = CommandType::Attack;
            internal.target_player_id = p->target_player_id();
            internal.value = p->percentage();
            break;
        }
        case CommandPayload_RetreatCmd: {
            auto p = cmd->payload_as_RetreatCmd();
            internal.type = CommandType::Retreat;
            internal.target_player_id = p->target_player_id();
            internal.value = 0;
            break;
        }
        case CommandPayload_CounterAttackCmd: {
            auto p = cmd->payload_as_CounterAttackCmd();
            internal.type = CommandType::CounterAttack;
            internal.target_player_id = p->target_player_id();
            internal.value = 0;
            break;
        }
        case CommandPayload_SetRatioCmd: {
            auto p = cmd->payload_as_SetRatioCmd();
            internal.type = CommandType::SetRatio;
            internal.target_player_id = 0;
            internal.value = p->ratio();
            break;
        }
        case CommandPayload_BuyUrbanizationCmd: {
            auto p = cmd->payload_as_BuyUrbanizationCmd();
            internal.type = CommandType::BuyUrbanization;
            internal.target_player_id = 0;
            internal.value = p->amount();
            break;
        }
        default:
            return std::nullopt;
    }

    return internal;
}

std::vector<uint8_t> Serializer::FinalizeBuilder(flatbuffers::FlatBufferBuilder& builder) {
    return std::vector<uint8_t>(builder.GetBufferPointer(),
                                builder.GetBufferPointer() + builder.GetSize());
}

}  // namespace rts::network