#pragma once
#include <cstdint>

namespace rts::game {

constexpr int32_t FIXED_POINT_MULTIPLIER = 1000;

enum class TerrainType : uint8_t {
    Plains = 0,
    Hills = 1,
    Desert = 2,
    Arctic = 3,
    Mountains = 4,
    Water = 5
};

// Typy komend u¿ywane przez Serializer i SimulationLoop
enum class CommandType : uint8_t { Attack, Retreat, CounterAttack, SetRatio, BuyUrbanization };

// ==========================================
// KOMPONENTY KAFELKA
// ==========================================
struct COwner {
    uint16_t player_id;
};
struct CTerrain {
    TerrainType type;
};
struct CGridPosition {
    int16_t x, y;
};

// ==========================================
// KOMPONENTY GRACZA (Globalne Byty)
// ==========================================
struct CPlayer {
    uint16_t id;
    bool is_alive = true;  // false po utracie wszystkich kafelków, dalej obserwuje
    bool has_won = false;  // ustawione gdy gracz osi¹gn¹³ 100% zajmowalnych kafelków
};

struct CPlayerTerritory {
    int32_t num_tiles_owned = 0;
};

struct CPlayerPopulation {
    int64_t total_troops = 0;
    int64_t total_workers = 0;
    int32_t target_troops_ratio_pc = 50;
};

struct CPlayerGold {
    double current_amount = 0.0;
};

struct CPlayerUrbanization {
    double level_pc = 22.5;  // 22.5 = punkt zerowy bonusu; powy¿ej dok³ada do max popa
};

struct CPlayerCombat {
    uint16_t target_player_id = 0;        // wybrany cel ataku (0 = brak)
    int32_t attack_troops_pc = 0;         // % troops zaanga¿owanych w atak (suwak gracza)
    uint16_t last_attacker_id = 0;        // ostatni gracz który zada³ nam stratê w combat
    int32_t last_attacker_troops_pc = 0;  // jego attack_troops_pc — u¿ywane przez CounterAttack
};

// Zaanga¿owanie troops w aktywne walki — odejmowane od populacji do growth
struct CPlayerCommitments {
    int64_t troops_committed_to_attack = 0;
};

}  // namespace rts::game