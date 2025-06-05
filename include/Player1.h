//
// Created by 97250 on 5/28/2025.
//

#pragma once

#include "common/Player.h"
#include "common/TankAlgorithm.h"
#include "common/SatelliteView.h"
#include "MyBattleInfo.h"

class Player1 : public Player {
public:
    Player1(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
        : Player(player_index, x, y, max_steps, num_shells)
        , playerIndex_(player_index)
        , boardRows_(y)
        , boardCols_(x) {}

    void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) override {
        // Create a temporary BattleInfo object that will be destroyed when this function returns
        MyBattleInfo battleInfo(satellite_view, playerIndex_, boardRows_, boardCols_);
        
        // Let the tank update its internal state based on the battle info
        // The tank must not store references to battleInfo
        tank.updateBattleInfo(battleInfo);
    }

private:
    int playerIndex_;
    size_t boardRows_;
    size_t boardCols_;
};
