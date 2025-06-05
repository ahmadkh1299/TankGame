#include "../include/MyTankAlgorithmFactory.h"
#include "../include/common/TankAlgorithm.h"
#include "../include/ZoneControlAlgo.h"
#include "../include/HunterAlgo.h"
#include <memory>

std::unique_ptr<TankAlgorithm> MyTankAlgorithmFactory::create(int player_index, int tank_index) const {
    std::unique_ptr<TankAlgorithm> algo;

    if (player_index == 1) {
        // Player 1 uses ZoneControlAlgo
        algo = std::make_unique<ZoneControlAlgo>(tank_index);
    } else if (player_index == 2) {
        // Player 2 uses HunterAlgo
        algo = std::make_unique<HunterAlgo>(tank_index);
    } else {
        // Fallback or default algorithm (optional)
        algo = std::make_unique<ZoneControlAlgo>(tank_index);
    }

    return algo;
}
