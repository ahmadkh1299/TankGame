#pragma once

#include "common/BattleInfo.h"
#include "common/SatelliteView.h"
#include <utility>
#include <vector>

class MyBattleInfo : public BattleInfo {
public:
    // Constructor copies the current battlefield state
    MyBattleInfo(const SatelliteView& view, int playerIndex, size_t rows, size_t cols,
                 std::pair<size_t, size_t> selfPos);

    // Get the character at a specific position as defined in the assignment:
    // '#' - Wall
    // '1' - Tank owned by player 1
    // '2' - Tank owned by player 2
    // '%' - The requesting tank
    // '@' - Mine
    // '*' - Artillery shell
    // ' ' - Empty space
    // '&' - Outside battlefield
    char getObjectAt(size_t x, size_t y) const;

    // Battlefield dimensions
    size_t getRows() const { return rows_; }
    size_t getCols() const { return cols_; }
    int getPlayerIndex() const { return playerIndex_; }

    // Current tank's position
    std::pair<size_t, size_t> getSelfPosition() const { return selfPos_; }

private:
    // Store the battlefield as a 2D vector of chars exactly as seen by satellite
    std::vector<std::vector<char>> battlefield_;
    size_t rows_;
    size_t cols_;
    int playerIndex_;
    std::pair<size_t, size_t> selfPos_;
};
