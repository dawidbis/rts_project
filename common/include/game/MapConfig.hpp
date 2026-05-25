#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace rts::game {

struct MapConfig {
    std::string id;
    std::string display_name;
    int16_t width;
    int16_t height;
    int default_bots;       
    int max_human_players; 
};

// Zestaw 3 predefiniowanych map (S/M/L) inspirowanych rozmiarami openfront.io
namespace maps {

inline const std::vector<MapConfig>& available() {
    static const std::vector<MapConfig> kMaps = {
        // ID                display_name        W      H    bots  humans
        {"small_500x250", "Small (500×250)", 500, 250, 4, 4},
        {"medium_1000x500", "Medium (1000×500)", 1000, 500, 8, 8},
        {"large_2000x1000", "Large (2000×1000)", 2000, 1000, 16, 16},
    };
    return kMaps;
}

inline const MapConfig* find_by_id(const std::string& id) {
    for (const auto& m : available()) {
        if (m.id == id) return &m;
    }
    return nullptr;
}

}  // namespace maps
}  // namespace rts::game