//
// Created by 97250 on 5/28/2025.
//

#pragma once

#include "common/Player.h"
#include "common/TankAlgorithm.h"
#include "common/SatelliteView.h"
#include "MyBattleInfo.h"
#include "MyTankAlgorithm.h"

class Player1 : public Player {
public:
    Player1(int player_index, size_t x, size_t y, size_t max_steps, size_t num_shells)
        : Player(player_index, x, y, max_steps, num_shells)
        , playerIndex_(player_index)
        , boardRows_(y)
        , boardCols_(x) {}

    void updateTankWithBattleInfo(TankAlgorithm& tank, SatelliteView& satellite_view) override {
        int tankIdx = 0;
        if (auto* myTank = dynamic_cast<MyTankAlgorithm*>(&tank)) {
            tankIdx = myTank->getTankIndex();
        }

        std::pair<size_t, size_t> selfPos{0, 0};
        int found = 0;
        for (size_t y = 0; y < boardRows_; ++y) {
            for (size_t x = 0; x < boardCols_; ++x) {
                char obj = satellite_view.getObjectAt(x, y);
                if ((obj == '1' || obj == '2') && (obj - '0' == playerIndex_)) {
                    if (found == tankIdx) {
                        selfPos = {x, y};
                    }
                    ++found;
                }
            }
        }

        MyBattleInfo battleInfo(satellite_view, playerIndex_, boardRows_, boardCols_, selfPos);

        // Let the tank update its internal state based on the battle info
        // The tank must not store references to battleInfo
        tank.updateBattleInfo(battleInfo);
    }

private:
    int playerIndex_;
    size_t boardRows_;
    size_t boardCols_;
};
