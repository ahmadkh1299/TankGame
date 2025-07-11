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
    std::vector<std::unique_ptr<Tank>> getPlayer1Tanks();
    std::vector<std::unique_ptr<Tank>> getPlayer2Tanks();

    std::vector<std::unique_ptr<Mine>>& getActiveMines();
    std::vector<std::unique_ptr<Wall>>& getActiveWalls();

    int getMaxSteps() const;
    int getNumShells() const;

private:
    Board board_;
    std::vector<std::unique_ptr<Tank>> player1Tanks_;
    std::vector<std::unique_ptr<Tank>> player2Tanks_;
    std::vector<std::unique_ptr<Mine>> activeMines_;
    std::vector<std::unique_ptr<Wall>> activeWalls_;

    int maxSteps_;
    int numShells_;
};
