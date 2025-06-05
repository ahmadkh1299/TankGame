#pragma once
#include "common/TankAlgorithmFactory.h"
#include "MyTankAlgorithm.h"
#include "ZoneControlAlgo.h"
#include "HunterAlgo.h"

class MyTankAlgorithmFactory : public TankAlgorithmFactory {
public:
    std::unique_ptr<TankAlgorithm> create(int player_index, int tank_index) const override;
};
