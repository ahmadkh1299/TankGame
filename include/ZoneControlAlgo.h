#pragma once

#include "Board.h"
#include "Tank.h"
#include "Shell.h"
#include "Mine.h"
#include "common/ActionRequest.h"
#include "common/TankAlgorithm.h"
#include "MyBattleInfo.h"
#include <vector>
#include <memory>
#include <optional>

class ZoneControlAlgo : public TankAlgorithm {
public:
    explicit ZoneControlAlgo(int tankId);
    ~ZoneControlAlgo() override = default;

    ActionRequest getAction() override;
    void updateBattleInfo(BattleInfo& info) override;

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
    bool forceUpdateNextTurn_ = false;
    std::optional<MyBattleInfo> currentInfo_;
    int turnsSinceLastUpdate_;
    static constexpr int UPDATE_INTERVAL = 4; // Update every 4 turns
    size_t lastKnownEnemyCount_;
    size_t lastKnownAllyCount_;  // Track number of ally tanks
    int totalBoardWidth_;        // Store total board width for zone calculations
};
