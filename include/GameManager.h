#pragma once

#include <algorithm>

#include "Board.h"
#include "Tank.h"
#include "Mine.h"
#include "Wall.h"
#include "Shell.h"
#include "InputParser.h"
#include "common/Player.h"
#include "common/PlayerFactory.h"
#include "common/TankAlgorithmFactory.h"
#include "common/ActionRequest.h"


#include <type_traits>
#include <fstream>
#include <memory>
#include <vector>

class GameManager {
public:
    static constexpr int STEPS_WHEN_SHELLS_OVER = 40;
    static constexpr int SHELL_MOVES_PER_STEP = 2;
  	static constexpr int NUM_SHELLS = 20;
    const int MAX_TOTAL_STEPS = 50000;

    GameManager(std::unique_ptr<PlayerFactory> playerFactory,
                std::unique_ptr<TankAlgorithmFactory> tankFactory);

    ~GameManager(); // to close the file

    bool readBoard(const std::string& inputFile);
    void run();



private:
    std::unique_ptr<PlayerFactory> playerFactory_;
    std::unique_ptr<TankAlgorithmFactory> tankFactory_;
    Board board_;
    std::vector<std::unique_ptr<Wall>> walls_;
    std::vector<std::unique_ptr<Mine>> mines_;
    std::vector<std::unique_ptr<Shell>> shells_;

    std::vector<std::unique_ptr<Tank>> p1Tanks_;
    std::vector<std::unique_ptr<Tank>> p2Tanks_;

    std::unique_ptr<Player> player1_;
    std::unique_ptr<Player> player2_;

    int maxSteps_ = MAX_TOTAL_STEPS; //default; can get another number from use
    int numShells_ = NUM_SHELLS; //default; can get another number from use
    int stepsLeftWhenShellsOver_ = STEPS_WHEN_SHELLS_OVER;
    int shellMovesPerStep_ = SHELL_MOVES_PER_STEP;
    int stepCounter_ = 0;

    int boardWidth_;
    int boardHeight_;

    std::ofstream outputFile_;

    //Handling tank's actions
    void handleMoveTankForward(Tank& tank);
    void handleMoveTankBack(Tank& tank);
    void handleTankAskMoveBack(Tank& tank);
    void handleShoot(Tank& tank);
    void handleRotateEighthLeft(Tank& tank);
    void handleRotateFourthLeft(Tank& tank);
    void handleRotateEighthRight(Tank& tank);
    void handleRotateFourthRight(Tank& tank);
    void handleDoNothing(Tank& tank);
    void handleRequestBattleInfo(Tank& tank);

    void handleAction(Tank& tank, ActionRequest action);

    //helper functions for the run() function
    ActionRequest decideAction(Tank& t, TankAlgorithm& algo);

    int getTotalShellsLeft() const;
    void countersHandler(Tank& tank);
    void handleAutoMoveTankBack(Tank& tank);

    void resolveShellCollisionsAtPosition(Shell& shell);

    template<typename T>
    void cleanupDestroyedObjects(std::vector<std::unique_ptr<T>>& vec) {
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [&](std::unique_ptr<T>& obj) {
                if (obj->isDestroyed()) {
                    board_.removeObject(obj.get(), obj->getPosition()); // remove from board
                    return true; // remove from vector
                }
                return false;
            }), vec.end());
    }

    void moveForwardAndWrap(Shell& shell);
    void resolveTankCollisionsAtPosition(Tank& tank);
    void shellStep();

    //get pointers
    std::vector<Shell*> getShellPtrs() const;
    std::vector<Mine*> getMinePtrs() const;
    //walls?
    //tanks?

    //more helper functions
    void printBadStep(Tank& tank,ActionRequest action);
    void printGoodStep(Tank& tank, ActionRequest action);
    void printToFile(const std::string& message);





};
