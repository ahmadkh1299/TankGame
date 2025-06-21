// MyPlayerFactory.h
#pragma once

#include "common/PlayerFactory.h"
#include "Player1.h"
#include "Player2.h"
#include <stdexcept>

class MyPlayerFactory : public PlayerFactory {
public:
    std::unique_ptr<Player> create(int player_index, size_t x, size_t y,
                                   size_t max_steps, size_t num_shells) const override {
        if (player_index == 1)
            return std::make_unique<Player1>(player_index, x, y, max_steps, num_shells);
        else if (player_index == 2)
            return std::make_unique<Player2>(player_index, x, y, max_steps, num_shells);
        else
            throw std::invalid_argument("Invalid player index: must be 1 or 2");
    }
};
