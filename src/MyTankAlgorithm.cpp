#include "../include/MyTankAlgorithm.h"

MyTankAlgorithm::MyTankAlgorithm(std::unique_ptr<TankAlgorithm> actualAlgo,
                                 int playerIndex, int tankIndex)
        : actualAlgo_(std::move(actualAlgo)),
          playerIndex_(playerIndex),
          tankIndex_(tankIndex) {}

ActionRequest MyTankAlgorithm::getAction() {
    return actualAlgo_->getAction();
}

void MyTankAlgorithm::updateBattleInfo(BattleInfo& info) {
    actualAlgo_->updateBattleInfo(info);
}
