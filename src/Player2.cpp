#include "../include/Player2.h"
#include "../include/HunterAlgo.h"
#include "../include/MyBattleInfo.h"

Player2::Player2(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
    : Player(player_index, x, y, max_steps, num_shells),
      player_index_(player_index),
      board_width_(x),
      board_height_(y) {}

void Player2::updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) {
    auto* hunterAlgo = dynamic_cast<HunterAlgo*>(&tank);
    if (!hunterAlgo) return;

    MyBattleInfo info(satellite_view, player_index_, board_height_, board_width_, {0, 0});
    hunterAlgo->updateBattleInfo(info);
}
