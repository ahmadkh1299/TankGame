#pragma once

#include "Board.h"
#include "Tank.h"
#include "Shell.h"
#include "Mine.h"
#include "common/ActionRequest.h"
#include <vector>
#include <memory>

class ZoneControlAlgo {
public:
    explicit ZoneControlAlgo(int tankId);

    void updateZoneRange(int startX, int endX); // zone the tank is responsible for
    ActionRequest decideNextAction(const Tank& self,
                                   const Board& board,
                                   const std::vector<std::unique_ptr<Tank>>& enemyTanks,
                                   const std::vector<std::unique_ptr<Shell>>& shells,
                                   const std::vector<std::unique_ptr<Mine>>& mines);

private:
    int tankId_;
    int zoneStart_;
    int zoneEnd_;
};
