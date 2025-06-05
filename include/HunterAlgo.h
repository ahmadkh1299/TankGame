#pragma once

#include "common/TankAlgorithm.h"
#include "MyBattleInfo.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include "Position.h"
#include "Direction.h"
#include <optional>

class HunterAlgo : public TankAlgorithm {
public:
    explicit HunterAlgo(int tankId);
    ~HunterAlgo() override = default;

    ActionRequest getAction() override;
    void updateBattleInfo(BattleInfo& info) override;

private:
    int tankId_;
    std::optional<MyBattleInfo> currentInfo_;
    std::vector<Position> currentPath;

    std::vector<Position> runBFS(const Position& start, const Position& goal,
                                 const std::vector<std::vector<char>>& grid);

    Direction getDirectionTo(const Position& from, const Position& to) const;
    ActionRequest rotateToward(Direction current, Direction target) const;
};
