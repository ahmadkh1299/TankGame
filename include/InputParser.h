#pragma once

#include "Board.h"
#include "Tank.h"
#include "Mine.h"
#include "Wall.h"
#include <vector>
#include <string>
#include <memory>

class InputParser {
public:
    explicit InputParser(const std::string& filename);

    Board getBoard() const;
    const std::vector<Tank>& getPlayer1Tanks() const;
    const std::vector<Tank>& getPlayer2Tanks() const;
    std::vector<std::unique_ptr<Mine>>& getActiveMines();
    std::vector<std::unique_ptr<Wall>>& getActiveWalls();

    int getMaxSteps() const;
    int getNumShells() const;

private:
    Board board_;
    std::vector<Tank> player1Tanks_;
    std::vector<Tank> player2Tanks_;
    std::vector<std::unique_ptr<Mine>> activeMines_;
    std::vector<std::unique_ptr<Wall>> activeWalls_;

    int maxSteps_;
    int numShells_;
};
