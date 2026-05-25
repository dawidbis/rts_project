#include <flatbuffers/flatbuffers.h>

#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "../../common/include/network/Serializer.hpp"
#include "../fbs/rts_protocol_generated.hpp"

using namespace rts::network;

TEST_CASE("Serializer: Deserialize PlayerCommand (AttackCmd)", "[network]") {
    flatbuffers::FlatBufferBuilder builder;

    auto attack = CreateAttackCmd(builder, 123, 50);

    auto cmd = CreatePlayerCommand(builder, 1, CommandPayload_AttackCmd, attack.Union());

    auto msg = CreateTcpMessage(builder, TcpPayload_PlayerCommand, cmd.Union());
    builder.Finish(msg);

    std::span<const uint8_t> buffer(builder.GetBufferPointer(), builder.GetSize());

    auto internal = Serializer::DeserializeTcpCommand(buffer);

    REQUIRE(internal.has_value());
    CHECK(internal->player_id == 1);
    CHECK(internal->type == CommandType::Attack);
    CHECK(internal->target_player_id == 123);
    CHECK(internal->value == 50);
}

TEST_CASE("Serializer: Deserialize PlayerCommand (BuyUrbanizationCmd)", "[network]") {
    flatbuffers::FlatBufferBuilder builder;

    // 1. Stwórz payload (BuyUrbanization)
    auto urb = CreateBuyUrbanizationCmd(builder, 10);

    // 2. Stwórz PlayerCommand
    auto cmd = CreatePlayerCommand(builder, 5, CommandPayload_BuyUrbanizationCmd, urb.Union());

    // 3. Stwórz TcpMessage
    auto msg = CreateTcpMessage(builder, TcpPayload_PlayerCommand, cmd.Union());
    builder.Finish(msg);

    // 4. Przetestuj
    std::span<const uint8_t> buffer(builder.GetBufferPointer(), builder.GetSize());
    auto internal = Serializer::DeserializeTcpCommand(buffer);

    REQUIRE(internal.has_value());
    CHECK(internal->player_id == 5);
    CHECK(internal->type == CommandType::BuyUrbanization);
    CHECK(internal->value == 10);
}