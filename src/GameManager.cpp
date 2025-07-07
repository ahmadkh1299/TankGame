#include "GameManager.h"
#include <iostream>
#include <fstream>
#include <utility>
#include <type_traits>
#include <algorithm>

GameManager::GameManager(const PlayerFactory& playerFactory, const TankAlgorithmFactory& tankFactory)
        : playerFactory_(playerFactory),
          tankFactory_(tankFactory),
          board_(1, 1),
          boardWidth_(1),
          boardHeight_(1) {}

GameManager::~GameManager() {
    if (outputFile_.is_open()) {
        outputFile_.close();
    }
}

bool GameManager::readBoard(const std::string& inputFile) {
    InputParser parser(inputFile);
    board_ = parser.getBoard();
    boardWidth_ = board_.getWidth();
    boardHeight_ = board_.getHeight();
    maxSteps_ = parser.getMaxSteps();
    numShells_ = parser.getNumShells();
    walls_ = std::move(parser.getActiveWalls());
    mines_ = std::move(parser.getActiveMines());

    p1Tanks_ = parser.getPlayer1Tanks();
    p2Tanks_ = parser.getPlayer2Tanks();

    if (p1Tanks_.empty() || p2Tanks_.empty()) {
        std::cerr << "One or both players have no tanks.\n";
        return false;
    }

    player1_ = playerFactory_.create(1, boardWidth_, boardHeight_, maxSteps_, numShells_);
    player2_ = playerFactory_.create(2, boardWidth_, boardHeight_, maxSteps_, numShells_);
    outputFile_.open("game_output.txt");
    return true;
}

void GameManager::run() {
    int stepCounter = 0;
    int shellsLeftCounter = STEPS_WHEN_SHELLS_OVER;

    while (stepCounter < maxSteps_ && shellsLeftCounter > 0) {
        printToFile("\n--- Step " + std::to_string(stepCounter) + " ---");

        for (auto& t : p1Tanks_) if (!t->isDestroyed()) countersHandler(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed()) countersHandler(*t);

        for (auto& t : p1Tanks_) {
            if (!t->isDestroyed()) {
                ActionRequest action = decideAction(*t, *t->getAlgorithm());
                t->setNextAction(action);
            }
        }

        for (auto& t : p2Tanks_) {
            if (!t->isDestroyed()) {
                ActionRequest action = decideAction(*t, *t->getAlgorithm());
                t->setNextAction(action);
            }
        }


        for (auto& t : p1Tanks_) if (!t->isDestroyed() && t->getNextAction() == ActionRequest::Shoot) handleShoot(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed() && t->getNextAction() == ActionRequest::Shoot) handleShoot(*t);

        for (int i = 0; i < shellMovesPerStep_; ++i) {
            shellStep();
            cleanupDestroyedObjects(shells_);
            cleanupDestroyedObjects(walls_);
            cleanupDestroyedObjects(walls_);
            cleanupDestroyedObjects(p1Tanks_);
            cleanupDestroyedObjects(p2Tanks_);
        }

        for (auto& t : p1Tanks_) if (!t->isDestroyed() && t->getNextAction() != ActionRequest::Shoot) handleAction(*t, t->getNextAction());
        for (auto& t : p2Tanks_) if (!t->isDestroyed() && t->getNextAction() != ActionRequest::Shoot) handleAction(*t, t->getNextAction());

        for (auto& t : p1Tanks_) if (!t->isDestroyed()) resolveTankCollisionsAtPosition(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed()) resolveTankCollisionsAtPosition(*t);

        if (p1Tanks_.empty() && !p2Tanks_.empty()) {
            printToFile("\nPlayer 2 wins! (Player 1 has no tanks left)");
            return;
        }
        if (!p1Tanks_.empty() && p2Tanks_.empty()) {
            printToFile("\nPlayer 1 wins! (Player 2 has no tanks left)");
            return;
        }
        if (p1Tanks_.empty() && p2Tanks_.empty()) {
            printToFile("\nIt's a tie! (Both players have no tanks left)");
            return;
        }

        if (getTotalShellsLeft() <= 0) --shellsLeftCounter;
        stepCounter++;
    }

    printToFile("\nGame ended by steps or shell exhaustion");
}

ActionRequest GameManager::decideAction(Tank& tank, TankAlgorithm& algo){
    ActionRequest action;

    //we chose to check it for the tank
    if (tank.getIsWaitingToMoveBack()) {
        action = ActionRequest::DoNothing;
    }

    action = algo.getAction();

    return action;
}


void GameManager::handleShoot(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.shoot()) {
        printBadStep(tank, ActionRequest::Shoot);
        return;
    }
    shells_.push_back(std::make_unique<Shell>(tank.getPosition(), tank.getDirection(), tank.getId()));
    printGoodStep(tank, ActionRequest::Shoot);
}

//not including get battle info + shoot
void GameManager::handleAction(Tank& tank, ActionRequest action) {
    switch (action) {
        case ActionRequest::MoveForward: handleMoveTankForward(tank); break;
        case ActionRequest::MoveBackward: handleTankAskMoveBack(tank); break;
        case ActionRequest::RotateLeft45: handleRotateEighthLeft(tank); break;
        case ActionRequest::RotateLeft90: handleRotateFourthLeft(tank); break;
        case ActionRequest::RotateRight45: handleRotateEighthRight(tank); break;
        case ActionRequest::RotateRight90: handleRotateFourthRight(tank); break;
        case ActionRequest::DoNothing: handleDoNothing(tank); break;
        //case ActionRequest::GetBattleInfo: handleGetBattleInfo(tank); break;
        //case ActionRequest::Shoot: handleShoot(tank); break;
        default: printBadStep(tank, action); break;
    }
}

void GameManager::handleAutoMoveTankBack(Tank& tank) {
    if (tank.getIsWaitingToMoveBack() && tank.getWaitToMoveBackCounter() == 0) {
        handleMoveTankBack(tank);
    }
}

void GameManager::moveForwardAndWrap(Shell& shell) {
    shell.moveForward();
    Position newPos = shell.getPosition();
    newPos.wrap(boardWidth_, boardHeight_);
    shell.setPosition(newPos);
}

void GameManager::shellStep() {
    for (auto& shell : shells_) {
        moveForwardAndWrap(*shell);
        board_.addGameObject(shell.get(), shell->getPosition());
    }

    for (auto& shell : shells_) {
        if (!shell->isDestroyed()) {
            resolveShellCollisionsAtPosition(*shell);
        }
    }
}

void GameManager::resolveShellCollisionsAtPosition(Shell& shell) {
    Position pos = shell.getPosition();
    const auto& objects = board_.getObjectsAt(pos);
    for (const auto& obj : objects) {
        if (auto* wall = dynamic_cast<Wall*>(obj)) {
            wall->decreaseLifeLeft();
            shell.destroy();
        } else if (auto* anotherShell = dynamic_cast<Shell*>(obj)) {
            anotherShell->destroy();
            shell.destroy();
        } else if (auto* tank = dynamic_cast<Tank*>(obj)) {
            tank->destroy();
            shell.destroy();
        }
    }
}

void GameManager::resolveTankCollisionsAtPosition(Tank& tank) {
    Position pos = tank.getPosition();
    const auto& objects = board_.getObjectsAt(pos);
    for (const auto& obj : objects) {
        if (auto* mine = dynamic_cast<Mine*>(obj)) {
            mine->destroy();
            tank.destroy();
        } else if (auto* anotherTank = dynamic_cast<Tank*>(obj)) {
            if (anotherTank != &tank) {
                anotherTank->destroy();
                tank.destroy();
            }
        }
    }
}

std::vector<Shell*> GameManager::getShellPtrs() const {
    std::vector<Shell*> result;
    for (const auto& shell : shells_) {
        result.push_back(shell.get());
    }
    return result;
}

std::vector<Mine*> GameManager::getMinePtrs() const {
    std::vector<Mine*> result;
    for (const auto& mine : mines_) {
        result.push_back(mine.get());
    }
    return result;
}

void GameManager::handleMoveTankForward(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.moveForward()) {
        printBadStep(tank, ActionRequest::MoveForward);
        return;
    }
    printGoodStep(tank, ActionRequest::MoveForward);
}

void GameManager::handleMoveTankBack(Tank& tank) {
    if (tank.getIsRightAfterMoveBack() ||
        (tank.getIsWaitingToMoveBack() && tank.getWaitToMoveBackCounter() == 0)) {
        if (!tank.moveBack()) {
            printToFile("Failed to move tank back despite flag conditions.");
            tank.resetIsRightAfterMoveBack();
            return;
        }
        printToFile("Tank moved back successfully.");
    }
}

void GameManager::handleTankAskMoveBack(Tank& tank) {
    if (tank.getIsRightAfterMoveBack()) {
        printGoodStep(tank, ActionRequest::MoveBackward);
        handleMoveTankBack(tank);
        return;
    }
    if (!tank.askToMoveBack()) {
        printBadStep(tank, ActionRequest::MoveBackward);
        return;
    }
    printGoodStep(tank, ActionRequest::MoveBackward);
}

void GameManager::handleRotateEighthLeft(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateEighthLeft()) {
        printBadStep(tank, ActionRequest::RotateLeft45);
        return;
    }
    printGoodStep(tank, ActionRequest::RotateLeft45);
}

void GameManager::handleRotateFourthLeft(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateFourthLeft()) {
        printBadStep(tank, ActionRequest::RotateLeft90);
        return;
    }
    printGoodStep(tank, ActionRequest::RotateLeft90);
}

void GameManager::handleRotateEighthRight(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateEighthRight()) {
        printBadStep(tank, ActionRequest::RotateRight45);
        return;
    }
    printGoodStep(tank, ActionRequest::RotateRight45);
}

void GameManager::handleRotateFourthRight(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateFourthRight()) {
        printBadStep(tank, ActionRequest::RotateRight90);
        return;
    }
    printGoodStep(tank, ActionRequest::RotateRight90);
}

void GameManager::handleDoNothing(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    tank.doNothing();
    printGoodStep(tank, ActionRequest::DoNothing);
}

void GameManager::handleRequestBattleInfo(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    printGoodStep(tank, ActionRequest::GetBattleInfo);
}

void GameManager::printToFile(const std::string& message) {
    if (outputFile_.is_open()) {
        outputFile_ << message << std::endl;
    }
}

void GameManager::printBadStep(Tank& tank, ActionRequest action) {
    printToFile("Player " + std::to_string(tank.getPlayerId()) +
                " asked for a bad step: " + ActionUtils::toString(action));
}

void GameManager::printGoodStep(Tank& tank, ActionRequest action) {
    printToFile("Player " + std::to_string(tank.getPlayerId()) +
                " did step: " + ActionUtils::toString(action));
}