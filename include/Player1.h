#pragma once

#include "common/Player.h"
#include "ZoneControlAlgo.h"
#include "MyBattleInfo.h"
#include "common/SatelliteView.h"
#include <memory>

class Player1 : public Player {
public:
    Player1(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells);
    ~Player1() override = default;

    void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) override;

private:
    std::unique_ptr<TankAlgorithm> algo_;
    int player_index_;
    size_t board_width_;
    size_t board_height_;
};
