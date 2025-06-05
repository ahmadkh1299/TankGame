#include "../include/MyBattleInfo.h"

MyBattleInfo::MyBattleInfo(const SatelliteView& view, int playerIndex, size_t rows,
                           size_t cols, std::pair<size_t, size_t> selfPos)
    : battlefield_(rows, std::vector<char>(cols, ' '))  // Initialize with spaces
    , rows_(rows)
    , cols_(cols)
    , playerIndex_(playerIndex)
    , selfPos_(selfPos) {
    
    // Copy the battlefield state and find our tank
    for (size_t y = 0; y < rows; ++y) {
        for (size_t x = 0; x < cols; ++x) {
            char obj = view.getObjectAt(x, y);

            if (x == selfPos_.first && y == selfPos_.second) {
                battlefield_[y][x] = '%';
            } else {
                battlefield_[y][x] = obj;
            }
        }
    }
}

char MyBattleInfo::getObjectAt(size_t x, size_t y) const {
    if (x >= cols_ || y >= rows_) {
        return '&';  // Outside battlefield
    }
    return battlefield_[y][x];
}
