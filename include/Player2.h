#pragma once

#include "common/Player.h"
#include "HunterAlgo.h"
#include "MyBattleInfo.h"
#include "common/SatelliteView.h"

class Player2 : public Player {
public:
    Player2(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells);
    ~Player2() override = default;

    void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) override;

private:
    int player_index_;
    size_t board_width_;
    size_t board_height_;
};
