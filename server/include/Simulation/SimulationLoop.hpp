#pragma once
#include <algorithm>
#include <cmath>
#include <entt/entt.hpp>
#include <unordered_map>
#include <vector>

#include "game/Components.hpp"
#include "network/Serializer.hpp"
#include "simulation/MapGenerator.hpp"

namespace rts::server {

// =====================================================================
// MATHFORMULAS — wszystkie wzory game design w jednym miejscu
// =====================================================================
struct MathFormulas {
    // ---- Population cap ----
    // basePop = numTilesOwned^0.6 * 570  (kalibracja: 52 kafelki ? ~2.5k basePop, ~5k *2)
    // urb_bonus = max(0, urb_pc - 22.5) * 10_000   (22.5% = punkt zerowy bonusu)
    // maxPop = 2 * basePop + urb_bonus
    // Start (52 kafelki, urb 22.5%): maxPop ? 12_110
    static int64_t calculate_max_cap(int32_t num_tiles, double urbanization_pc) {
        if (num_tiles <= 0) return 0;
        const double base_pop = std::pow(static_cast<double>(num_tiles), 0.6) * 570.0;
        const double urb_bonus = std::max(0.0, urbanization_pc - 22.5) * 10000.0;
        return static_cast<int64_t>(2.0 * base_pop + urb_bonus);
    }

    // ---- Population growth ----
    // f(r) = (10 + (M*r)^0.73 / 4) * (1 - r), gdzie r = current/max
    // Maximum przy ~42% capa (zgodne z openfront v23)
    static double calculate_growth(double current_pop, double max_pop) {
        if (current_pop >= max_pop || max_pop <= 0.0) return 0.0;
        const double r = current_pop / max_pop;
        return (10.0 + std::pow(current_pop, 0.73) / 4.0) * (1.0 - r);
    }

    // ---- Urbanizacja: nieliniowy koszt 1 punktu % ----
    // cost(level) = 5_000 * (1 + (level/100)^4 * 200)
    // Œciana wzrostu kosztu po ~60%, asymptotyczna do 100%
    static double get_urbanization_cost(double current_level_pc) {
        const double normalized = current_level_pc / 100.0;
        return 5000.0 * (1.0 + std::pow(normalized, 4.0) * 200.0);
    }

    // ---- Terrain: obrona (mno¿nik strat atakuj¹cego) ----
    // Plains: 0.85 (defender slaby ? mniej strat atakera). Wiki openfronta: Plains -15% loss.
    // Hills: 1.0 baseline.
    // Desert/Arctic: 1.2 (defender silny ? wiêcej strat atakera). Wiki openfronta: Mountains +20%.
    static double get_terrain_def_multiplier(game::TerrainType type) {
        switch (type) {
            case game::TerrainType::Plains:
                return 0.85;
            case game::TerrainType::Hills:
                return 1.0;
            case game::TerrainType::Desert:
            case game::TerrainType::Arctic:
                return 1.2;
            default:
                return 1.0;
        }
    }

    // ---- Terrain: bazowa "szybkoœæ" (cooldown w tickach do zajêcia kafelka) ----
    // Ni¿sza = szybciej. Modyfikowana przez size_speed_multiplier i attack_speed_bonus_from_ratio.
    static double get_terrain_base_speed(game::TerrainType type) {
        switch (type) {
            case game::TerrainType::Plains:
                return 16.5;  // +10% szybciej (mno¿nik 0.91)
            case game::TerrainType::Hills:
                return 20.0;  // baseline
            case game::TerrainType::Desert:
            case game::TerrainType::Arctic:
                return 25.0;  // -25% wolniej
            default:
                return 20.0;
        }
    }

    // ---- Troop Ratio Factor (wiki openfronta) ----
    // factor = clamp(defender_troops / attacker_troops, 0.6, 2.0)
    // attacker > 166% defender ? 0.6× (minimum strat), attacker < 50% defender ? 2.0× (max strat)
    static double get_troop_ratio_factor(int64_t attacker_troops, int64_t defender_troops) {
        if (attacker_troops <= 0) return 2.0;
        const double ratio =
            static_cast<double>(defender_troops) / static_cast<double>(attacker_troops);
        return std::clamp(ratio, 0.6, 2.0);
    }

    // ---- Size Penalty (atakuj¹cy/obroñca) — wiki openfronta ----
    // Imperia >100_000 tiles: straty redukowane przez ?(100_000 / tiles)
    static double get_size_loss_reduction(int32_t num_tiles) {
        if (num_tiles <= 100000) return 1.0;
        return std::sqrt(100000.0 / static_cast<double>(num_tiles));
    }

    // Imperia >75_000 tiles: speed redukowany przez (75_000 / tiles)^0.6
    static double get_size_speed_multiplier(int32_t num_tiles) {
        if (num_tiles <= 75000) return 1.0;
        return std::pow(75000.0 / static_cast<double>(num_tiles), 0.6);
    }

    // ---- Attack speed bonus z ratio rozmiarów armii (wiki openfronta) ----
    // size_ratio = attacker_troops / defender_troops
    // ratio < 0.05 ? 0 (za s³aby)
    // 0.05 <= ratio < 10 ? bonus 0.5% per 1% wzrostu ratio
    // ratio >= 10 ? bonus 0.1% per 1% wzrostu ratio
    // Zwraca mno¿nik do speed (im ni¿szy, tym szybciej; bonus 50% ? 0.5x)
    static double get_attack_speed_bonus_multiplier(int64_t attacker_troops,
                                                    int64_t defender_troops) {
        if (defender_troops <= 0) return 0.5;  // brak obrony — atak ekspresowy
        const double size_ratio =
            static_cast<double>(attacker_troops) / static_cast<double>(defender_troops);
        if (size_ratio < 0.05) return 1.0;  // za s³aby, brak bonusu
        double speed_bonus_pc;
        if (size_ratio < 10.0) {
            speed_bonus_pc = (size_ratio - 0.05) * 100.0 * 0.5;
        } else {
            // do 10× ratio: bonus zbudowany w pierwszym zakresie
            const double base_bonus = (10.0 - 0.05) * 100.0 * 0.5;
            const double extra_bonus = (size_ratio - 10.0) * 100.0 * 0.1;
            speed_bonus_pc = base_bonus + extra_bonus;
        }
        // konwersja "bonus%" ? mno¿nik czasu: -50% ? 0.5x, -83% ? 0.17x, cap 0.1x
        const double multiplier = 1.0 - speed_bonus_pc / 100.0;
        return std::max(0.1, multiplier);
    }

    // ---- Gold rate ----
    // 1 gold per worker per tick (wiki openfronta)
    static constexpr double GOLD_PER_WORKER_PER_TICK = 1.0;
};

// =====================================================================
// SIMULATION LOOP
// =====================================================================
class SimulationLoop {
private:
    entt::registry registry_;
    uint32_t current_tick_ = 0;
    int16_t width_, height_;
    std::vector<entt::entity> grid_;
    std::vector<double> tile_cooldowns_;
    std::unordered_map<uint16_t, entt::entity> player_entities_;
    std::vector<network::InternalCommand> command_queue_;
    int32_t total_conquerable_tiles_ = 0;  // u¿ywane przez win condition

public:
    SimulationLoop(int16_t width, int16_t height) : width_(width), height_(height) {
        grid_.resize(width * height, entt::null);
        tile_cooldowns_.resize(width * height, 0.0);
    }

    void init_world(int num_players, int num_bots) {
        MapGenerator::generate_test_map(registry_, width_, height_, num_players, num_bots);
        auto view = registry_.view<game::CGridPosition>();
        for (auto e : view) {
            auto& p = view.get<game::CGridPosition>(e);
            grid_[p.y * width_ + p.x] = e;
        }
        for (auto e : registry_.view<game::CPlayer>()) {
            const auto pid = registry_.get<game::CPlayer>(e).id;
            player_entities_[pid] = e;
            // Inicjalizujemy nowe komponenty
            registry_.emplace<game::CPlayerCombat>(e);
            registry_.emplace<game::CPlayerCommitments>(e);
        }
        // Policz ca³kowit¹ liczbê zajmowalnych kafelków (nie-water, nie-mountains)
        total_conquerable_tiles_ = 0;
        for (auto [e, terrain] : registry_.view<game::CTerrain>().each()) {
            if (terrain.type != game::TerrainType::Water &&
                terrain.type != game::TerrainType::Mountains) {
                ++total_conquerable_tiles_;
            }
        }
    }

    void push_command(const network::InternalCommand& cmd) { command_queue_.push_back(cmd); }

    void step_simulation() {
        current_tick_++;
        system_process_commands();
        system_update_alive_status();
        system_global_economy();
        system_population_conversion();
        system_compute_commitments();
        system_population_growth();
        system_combat();
        // Win condition sprawdzamy lazy: tylko gdy jakiœ gracz jest blisko 100% — tania œcie¿ka.
        system_check_victory();
    }

    bool is_game_over() const {
        for (auto e : registry_.view<game::CPlayer>()) {
            if (registry_.get<game::CPlayer>(e).has_won) return true;
        }
        return false;
    }

private:
    entt::entity get_entity_at(int16_t x, int16_t y) const {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) return entt::null;
        return grid_[y * width_ + x];
    }

    int count_friendly_neighbors(int16_t x, int16_t y, uint16_t pid) const {
        int count = 0;
        const entt::entity ns[] = {get_entity_at(x, y - 1),
                                   get_entity_at(x, y + 1),
                                   get_entity_at(x - 1, y),
                                   get_entity_at(x + 1, y)};
        for (auto n : ns) {
            if (n != entt::null && registry_.get<game::COwner>(n).player_id == pid) ++count;
        }
        return count;
    }

    int get_terrain_priority(game::TerrainType t) const {
        if (t == game::TerrainType::Plains) return 3;
        if (t == game::TerrainType::Hills) return 2;
        if (t == game::TerrainType::Desert || t == game::TerrainType::Arctic) return 1;
        return 0;
    }

    bool is_conquerable(game::TerrainType t) const {
        return t != game::TerrainType::Water && t != game::TerrainType::Mountains;
    }

    // -----------------------------------------------------------------
    // System: process commands
    // -----------------------------------------------------------------
    void system_process_commands() {
        for (const auto& cmd : command_queue_) {
            auto it = player_entities_.find(cmd.player_id);
            if (it == player_entities_.end()) continue;
            auto player_ent = it->second;
            // Wyeliminowani gracze nie mog¹ wydawaæ komend (obserwuj¹)
            if (!registry_.get<game::CPlayer>(player_ent).is_alive) continue;

            switch (cmd.type) {
                case network::CommandType::SetRatio: {
                    auto& pop = registry_.get<game::CPlayerPopulation>(player_ent);
                    pop.target_troops_ratio_pc = std::clamp((int)cmd.value, 0, 100);
                    break;
                }
                case network::CommandType::BuyUrbanization: {
                    auto& gold = registry_.get<game::CPlayerGold>(player_ent);
                    auto& urb = registry_.get<game::CPlayerUrbanization>(player_ent);
                    if (urb.level_pc >= 100.0) break;
                    const double cost = MathFormulas::get_urbanization_cost(urb.level_pc);
                    if (gold.current_amount >= cost) {
                        gold.current_amount -= cost;
                        urb.level_pc = std::min(100.0, urb.level_pc + 1.0);
                    }
                    break;
                }
                case network::CommandType::Attack: {
                    auto& combat = registry_.get<game::CPlayerCombat>(player_ent);
                    // Walidacja: nie mo¿na atakowaæ siebie ani gracza nieistniej¹cego
                    if (cmd.target_player_id == cmd.player_id) break;
                    if (player_entities_.find(cmd.target_player_id) == player_entities_.end())
                        break;
                    combat.target_player_id = cmd.target_player_id;
                    combat.attack_troops_pc = std::clamp((int)cmd.value, 0, 100);
                    break;
                }
                case network::CommandType::Retreat: {
                    auto& combat = registry_.get<game::CPlayerCombat>(player_ent);
                    combat.attack_troops_pc = 0;
                    combat.target_player_id = 0;
                    break;
                }
                case network::CommandType::CounterAttack: {
                    auto& combat = registry_.get<game::CPlayerCombat>(player_ent);
                    // CounterAttack: ustaw cel = ostatni napastnik, troops_pc = jego
                    // attack_troops_pc (lustro si³y zgodnie z game designem: "wyrównaæ potencja³y
                    // ¿eby nie zajmowa³ kafelków")
                    if (combat.last_attacker_id != 0 &&
                        player_entities_.count(combat.last_attacker_id)) {
                        combat.target_player_id = combat.last_attacker_id;
                        combat.attack_troops_pc =
                            std::clamp(combat.last_attacker_troops_pc, 1, 100);
                    }
                    break;
                }
            }
        }
        command_queue_.clear();
    }

    // -----------------------------------------------------------------
    // System: alive status (po utracie wszystkich kafelków ? eliminacja, ale obserwuje)
    // -----------------------------------------------------------------
    void system_update_alive_status() {
        for (auto [e, player, terr] :
             registry_.view<game::CPlayer, game::CPlayerTerritory>().each()) {
            if (player.is_alive && terr.num_tiles_owned == 0) {
                player.is_alive = false;
                // Wyzeruj walkê zeliminowanego gracza
                if (auto* combat = registry_.try_get<game::CPlayerCombat>(e)) {
                    combat->attack_troops_pc = 0;
                    combat->target_player_id = 0;
                }
            }
        }
    }

    // -----------------------------------------------------------------
    // System: global economy (gold)
    // -----------------------------------------------------------------
    void system_global_economy() {
        for (auto [e, player, gold, pop] :
             registry_.view<game::CPlayer, game::CPlayerGold, game::CPlayerPopulation>().each()) {
            if (!player.is_alive) continue;
            gold.current_amount +=
                static_cast<double>(pop.total_workers) * MathFormulas::GOLD_PER_WORKER_PER_TICK;
        }
    }

    // -----------------------------------------------------------------
    // System: population conversion (asymetria 5%/2% zgodnie z game design)
    // -----------------------------------------------------------------
    void system_population_conversion() {
        for (auto [e, pop] : registry_.view<game::CPlayerPopulation>().each()) {
            const int64_t total = pop.total_troops + pop.total_workers;
            if (total == 0) continue;
            const double current_ratio = (static_cast<double>(pop.total_troops) / total) * 100.0;
            if (std::abs(current_ratio - pop.target_troops_ratio_pc) < 1.0) continue;

            const bool too_many_troops = current_ratio > pop.target_troops_ratio_pc;
            // Demilitaryzacja szybsza (5%) ni¿ mobilizacja (2%) — zgodnie z game design
            const double step_pc = too_many_troops ? 0.05 : 0.02;
            const int64_t step = static_cast<int64_t>(total * step_pc);
            if (too_many_troops) {
                pop.total_troops -= step;
                pop.total_workers += step;
            } else {
                pop.total_workers -= step;
                pop.total_troops += step;
            }
            pop.total_troops = std::max<int64_t>(0, pop.total_troops);
            pop.total_workers = std::max<int64_t>(0, pop.total_workers);
        }
    }

    // -----------------------------------------------------------------
    // System: compute committed troops (B2 — troops w walce nie rosn¹)
    // -----------------------------------------------------------------
    void system_compute_commitments() {
        for (auto [e, pop, combat, commit] :
             registry_
                 .view<game::CPlayerPopulation, game::CPlayerCombat, game::CPlayerCommitments>()
                 .each()) {
            commit.troops_committed_to_attack = (pop.total_troops * combat.attack_troops_pc) / 100;
        }
    }

    // -----------------------------------------------------------------
    // System: population growth (workers x1.3 bias, troops_committed pomijane)
    // -----------------------------------------------------------------
    void system_population_growth() {
        for (auto [e, p, pop, terr, urb, commit] : registry_
                                                       .view<game::CPlayer,
                                                             game::CPlayerPopulation,
                                                             game::CPlayerTerritory,
                                                             game::CPlayerUrbanization,
                                                             game::CPlayerCommitments>()
                                                       .each()) {
            if (!p.is_alive) continue;

            // Tylko idle populacja kontrybuuje do growth (troops_committed pomijane)
            const int64_t idle_pop = std::max<int64_t>(
                0, pop.total_troops + pop.total_workers - commit.troops_committed_to_attack);
            if (idle_pop == 0) continue;

            const int64_t max_cap =
                MathFormulas::calculate_max_cap(terr.num_tiles_owned, urb.level_pc);
            const double growth = MathFormulas::calculate_growth(static_cast<double>(idle_pop),
                                                                 static_cast<double>(max_cap));
            // Workers rosn¹ ~30% szybciej; mno¿nik proporcjonalny do udzia³u workers w idle
            const int64_t idle_workers =
                std::max<int64_t>(0, pop.total_workers - 0);  // wszyscy workers s¹ zawsze idle
            const double workers_share =
                static_cast<double>(idle_workers) / static_cast<double>(idle_pop);
            const double growth_mult = 1.0 + workers_share * 0.30;
            const int64_t total_added = static_cast<int64_t>(growth * growth_mult);

            // Nowy przyrost dzielony wg target ratio
            const int64_t troops_added = (total_added * pop.target_troops_ratio_pc) / 100;
            const int64_t workers_added = total_added - troops_added;
            pop.total_troops += troops_added;
            pop.total_workers += workers_added;
        }
    }

    // -----------------------------------------------------------------
    // System: combat (auto-target, ale priorytet target_player_id)
    // -----------------------------------------------------------------
    void system_combat() {
        std::vector<std::pair<entt::entity, uint16_t>> conquests;

        for (auto [tile, owner, pos] : registry_.view<game::COwner, game::CGridPosition>().each()) {
            if (owner.player_id == 0) continue;
            auto attacker_it = player_entities_.find(owner.player_id);
            if (attacker_it == player_entities_.end()) continue;
            const auto attacker_ent = attacker_it->second;
            if (!registry_.get<game::CPlayer>(attacker_ent).is_alive) continue;

            const auto& attacker_combat = registry_.get<game::CPlayerCombat>(attacker_ent);
            // Brak agresji = brak ataku
            if (attacker_combat.attack_troops_pc == 0) continue;

            // Wybierz wrogiego s¹siada — preferuj target_player_id, fallback: priorytet terenu
            entt::entity best = entt::null;
            int best_prio = -1;
            const entt::entity ns[] = {get_entity_at(pos.x, pos.y - 1),
                                       get_entity_at(pos.x, pos.y + 1),
                                       get_entity_at(pos.x - 1, pos.y),
                                       get_entity_at(pos.x + 1, pos.y)};
            for (auto n : ns) {
                if (n == entt::null) continue;
                const auto& n_o = registry_.get<game::COwner>(n);
                if (n_o.player_id == owner.player_id || n_o.player_id == 0) continue;
                const auto& n_terrain = registry_.get<game::CTerrain>(n).type;
                if (!is_conquerable(n_terrain)) continue;

                // Priorytet: jeœli s¹siad to target_player_id ? ogromny bonus
                const int target_bonus =
                    (attacker_combat.target_player_id == n_o.player_id) ? 1000 : 0;
                const int prio = target_bonus + get_terrain_priority(n_terrain) * 10 +
                                 count_friendly_neighbors(pos.x, pos.y, owner.player_id);
                if (prio > best_prio) {
                    best_prio = prio;
                    best = n;
                }
            }
            if (best == entt::null) continue;

            auto& atk_p = registry_.get<game::CPlayerPopulation>(attacker_ent);
            const auto& atk_t = registry_.get<game::CPlayerTerritory>(attacker_ent);
            auto& def_o = registry_.get<game::COwner>(best);
            const auto defender_it = player_entities_.find(def_o.player_id);
            if (defender_it == player_entities_.end()) continue;
            const auto defender_ent = defender_it->second;
            auto& def_p = registry_.get<game::CPlayerPopulation>(defender_ent);
            const auto& def_t = registry_.get<game::CPlayerTerritory>(defender_ent);
            const auto& def_terrain = registry_.get<game::CTerrain>(best).type;

            // Zaanga¿owane troops atakuj¹cego (% z suwaka)
            const int64_t attacker_committed =
                (atk_p.total_troops * attacker_combat.attack_troops_pc) / 100;
            if (attacker_committed <= 0) continue;

            // Speed: bazowy terenowy * size_speed_mult atakera * bonus z ratio armii
            const double base_speed = MathFormulas::get_terrain_base_speed(def_terrain);
            const double size_speed_mult =
                MathFormulas::get_size_speed_multiplier(atk_t.num_tiles_owned);
            const double ratio_speed_mult = MathFormulas::get_attack_speed_bonus_multiplier(
                attacker_committed, def_p.total_troops);
            const double effective_speed = base_speed * size_speed_mult * ratio_speed_mult;

            tile_cooldowns_[pos.y * width_ + pos.x] += 1.0;
            if (tile_cooldowns_[pos.y * width_ + pos.x] < effective_speed) continue;
            tile_cooldowns_[pos.y * width_ + pos.x] = 0.0;

            // Straty (wiki openfronta):
            // Attacker loss = TroopRatioFactor × TerrainDefMult × 0.8 × SizeLossReduction(attacker)
            // Defender loss = (def_troops / def_tiles), aby broniæ "rozproszonej" populacji
            const double troop_ratio =
                MathFormulas::get_troop_ratio_factor(attacker_committed, def_p.total_troops);
            const double terrain_def = MathFormulas::get_terrain_def_multiplier(def_terrain);
            const double atk_size_red =
                MathFormulas::get_size_loss_reduction(atk_t.num_tiles_owned);
            const double def_size_red =
                MathFormulas::get_size_loss_reduction(def_t.num_tiles_owned);

            const double a_loss_base = troop_ratio * terrain_def * 0.8 * atk_size_red;
            const double d_loss_base = static_cast<double>(def_p.total_troops) /
                                       std::max(1, def_t.num_tiles_owned) * def_size_red;

            const int64_t a_loss = std::max<int64_t>(1, static_cast<int64_t>(a_loss_base));
            const int64_t d_loss = std::max<int64_t>(1, static_cast<int64_t>(d_loss_base));

            atk_p.total_troops = std::max<int64_t>(0, atk_p.total_troops - a_loss);
            def_p.total_troops = std::max<int64_t>(0, def_p.total_troops - d_loss);

            // Rejestracja "last_attacker" u obroñcy (na potrzeby CounterAttack)
            auto& def_combat = registry_.get<game::CPlayerCombat>(defender_ent);
            def_combat.last_attacker_id = owner.player_id;
            def_combat.last_attacker_troops_pc = attacker_combat.attack_troops_pc;

            // Defender bez troops ? kafelek zmienia w³aœciciela
            if (def_p.total_troops <= 0) {
                conquests.push_back({best, owner.player_id});
            }
        }

        // Aplikujemy zmiany w³aœciciela (deferred, ¿eby nie modyfikowaæ w trakcie iteracji)
        for (auto& c : conquests) {
            auto& o = registry_.get<game::COwner>(c.first);
            const auto old_owner = o.player_id;
            if (old_owner != 0 && player_entities_.count(old_owner)) {
                --registry_.get<game::CPlayerTerritory>(player_entities_[old_owner])
                      .num_tiles_owned;
            }
            o.player_id = c.second;
            ++registry_.get<game::CPlayerTerritory>(player_entities_[c.second]).num_tiles_owned;
        }
    }

    // -----------------------------------------------------------------
    // System: check victory (lazy — sprawdza tylko graczy z >90% kafelków)
    // -----------------------------------------------------------------
    void system_check_victory() {
        if (total_conquerable_tiles_ == 0) return;
        const int32_t threshold_90pc = (total_conquerable_tiles_ * 90) / 100;
        for (auto [e, player, terr] :
             registry_.view<game::CPlayer, game::CPlayerTerritory>().each()) {
            if (player.has_won) continue;
            if (terr.num_tiles_owned >= threshold_90pc) {
                if (terr.num_tiles_owned >= total_conquerable_tiles_) {
                    player.has_won = true;
                }
            }
        }
    }
};
}  // namespace rts::server