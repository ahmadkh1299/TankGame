#include "../include/Player1.h"

Player1::Player1(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
        : Player(player_index, x, y, max_steps, num_shells),
          player_index_(player_index),
          board_width_(x),
          board_height_(y) {}

void Player1::updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) {
    auto* algo = dynamic_cast<ZoneControlAlgo*>(&tank);
    if (!algo) return;

    MyBattleInfo info(satellite_view, player_index_, board_height_, board_width_, {0, 0});
    algo->updateBattleInfo(info);
}
