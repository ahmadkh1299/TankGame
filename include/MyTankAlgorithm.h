#pragma once

#include "common/TankAlgorithm.h"
#include <memory>

class MyTankAlgorithm : public TankAlgorithm {
public:
    MyTankAlgorithm(std::unique_ptr<TankAlgorithm> actualAlgo,
                    int playerIndex, int tankIndex);

    ActionRequest getAction() override;
    void updateBattleInfo(BattleInfo& info) override;

    int getPlayerIndex() const { return playerIndex_; }
    int getTankIndex() const { return tankIndex_; }

private:
    std::unique_ptr<TankAlgorithm> actualAlgo_;
    int playerIndex_;
    int tankIndex_;
};
