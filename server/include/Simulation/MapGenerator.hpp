#pragma once
#include <entt/entt.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "game/Components.hpp"

namespace rts::server {

class MapGenerator {
public:
    static void generate_test_map(
        entt::registry& registry, int16_t width, int16_t height, int num_players, int num_bots) {
        const std::vector<std::string> layout = {"WWWWWWWWWWWWWWWWWWWW",
                                                 "WPPPPPPPHHHHDDDPPPPW",
                                                 "WP1PPPPHHHHDDDDPP2PW",
                                                 "WPPPPPHHMMMMDDDPPPPW",
                                                 "WPPPPPHHMMMMDDDPPPPW",
                                                 "WHHHHHPPHHHDDDAAAAPW",
                                                 "WHHHHHPPHHHDDDAAAAPW",
                                                 "WDDDPPPPPPPPAAAAAAPW",
                                                 "WDD3PPPPPPPPAAA4AAPW",
                                                 "WDDDPPHHMMMMDDDPPPPW",
                                                 "WDDDPPHHMMMMDDDPPPPW",
                                                 "WPPPPPHHHHDDDPPPPPPW",
                                                 "WP5PPPHHHHDDDPP6PPPW",
                                                 "WPPPPPPPPPPPPPPPPPPW",
                                                 "WWWWWWWWWWWWWWWWWWWW"};

        int16_t map_w = static_cast<int16_t>(layout[0].size());
        int16_t map_h = static_cast<int16_t>(layout.size());

        int total_participants = num_players + num_bots;

        // 1. TWORZENIE GLOBALNYCH ENCJI GRACZY
        for (uint16_t i = 1; i <= static_cast<uint16_t>(total_participants); ++i) {
            auto player_entity = registry.create();
            registry.emplace<game::CPlayer>(player_entity, i);
            registry.emplace<game::CPlayerTerritory>(player_entity, 0);
            // Globalna populacja startowa
            registry.emplace<game::CPlayerPopulation>(player_entity, 5000, 5000, 50);
            registry.emplace<game::CPlayerGold>(player_entity, 0.0);
            registry.emplace<game::CPlayerUrbanization>(player_entity, 22.5);
        }

        // 2. GENEROWANIE SIATKI MAPY
        int current_participant_id = 1;

        for (int16_t y = 0; y < height; ++y) {
            for (int16_t x = 0; x < width; ++x) {
                auto entity = registry.create();
                registry.emplace<game::CGridPosition>(entity, x, y);

                char cell = (y < map_h && x < map_w) ? layout[y][x] : 'W';

                game::TerrainType terrain = game::TerrainType::Water;
                uint16_t owner_id = 0;

                switch (cell) {
                    case 'P':
                        terrain = game::TerrainType::Plains;
                        break;
                    case 'H':
                        terrain = game::TerrainType::Hills;
                        break;
                    case 'D':
                        terrain = game::TerrainType::Desert;
                        break;
                    case 'A':
                        terrain = game::TerrainType::Arctic;
                        break;
                    case 'M':
                        terrain = game::TerrainType::Mountains;
                        break;
                    case 'W':
                        terrain = game::TerrainType::Water;
                        break;
                    default:
                        if (cell >= '1' && cell <= '9' &&
                            current_participant_id <= total_participants) {
                            terrain = game::TerrainType::Plains;
                            owner_id = current_participant_id++;

                            // Zwiêkszamy licznik posiadanych kafelków dla gracza startuj¹cego
                            auto p_view = registry.view<game::CPlayer, game::CPlayerTerritory>();
                            for (auto p : p_view) {
                                if (registry.get<game::CPlayer>(p).id == owner_id) {
                                    registry.get<game::CPlayerTerritory>(p).num_tiles_owned++;
                                }
                            }
                        }
                        break;
                }

                registry.emplace<game::CTerrain>(entity, terrain);
                registry.emplace<game::COwner>(entity, owner_id);
                // UWAGA: Usuniêto CPopulation z kafelka - to teraz dane globalne gracza!
            }
        }
    }
};

}  // namespace rts::server