#pragma once

#include "Board.h"
#include "Tank.h"
#include "Mine.h"
#include "Wall.h"
#include "Shell.h"
#include "InputParser.h"
#include "common/Player.h"
#include "common/PlayerFactory.h"
#include "common/TankAlgorithmFactory.h"

#include <fstream>
#include <memory>
#include <vector>

class GameManager {
public:
    GameManager(const PlayerFactory& playerFactory,
                const TankAlgorithmFactory& tankFactory);

    bool readBoard(const std::string& inputFile);
    void run();

private:
    struct TankWrapper {
        std::unique_ptr<Tank> tank;
        std::unique_ptr<TankAlgorithm> algorithm;
        std::string lastActionStr;
        bool isKilled = false;
        bool requestedBattleInfo = false;
    };

    Board board_;
    std::vector<std::unique_ptr<Wall>> walls_;
    std::vector<std::unique_ptr<Mine>> mines_;
    std::vector<std::unique_ptr<Shell>> shells_;

    std::vector<TankWrapper> p1Tanks_;
    std::vector<TankWrapper> p2Tanks_;

    std::unique_ptr<Player> player1_;
    std::unique_ptr<Player> player2_;

    int maxSteps_ = 50000;
    int numShells_ = 20;

    std::ofstream outputFile_;
    std::string outputFileName_ = "game_output.txt";

    const PlayerFactory& playerFactory_;
    const TankAlgorithmFactory& tankFactory_;

    void logStep();
    void resolveActions(std::vector<TankWrapper>& tanks, int playerId);
    void cleanUpShells();
    void handleBattleInfoRequests();
    bool isGameOver() const;
    std::string getFinalResult() const;
    void resolveShellShellCollision(Shell* s1, Shell* s2);
    void resolveShellTankCollision(Shell* shell, Tank* tank);
    void resolveShellWallCollision(Shell* shell, Wall* wall);
    void resolveTankTankCollision(Tank* t1, Tank* t2);

    static constexpr int NO_SHELLS_STEPS_LIMIT = 40;
    int stepsWithoutShells_ = 0;
};
