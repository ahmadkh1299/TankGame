#include "GameManager.h"
#include <iostream>

GameManager::GameManager(const PlayerFactory& playerFactory,
                         const TankAlgorithmFactory& tankFactory)
        : playerFactory_(playerFactory),
          tankFactory_(tankFactory),
          board_(1, 1) {}

bool GameManager::readBoard(const std::string& inputFile) {
    InputParser parser(inputFile);

    board_ = parser.getBoard();
    maxSteps_ = parser.getMaxSteps();
    numShells_ = parser.getNumShells();
    walls_ = std::move(parser.getActiveWalls());
    mines_ = std::move(parser.getActiveMines());

    int p1Count = 0;
    for (const auto& t : parser.getPlayer1Tanks()) {
        auto tank = std::make_unique<Tank>(*t);
        auto algo = tankFactory_.create(1, p1Count);
        p1Tanks_.push_back({std::move(tank), std::move(algo)});
        ++p1Count;
    }

    int p2Count = 0;
    for (const auto& t : parser.getPlayer2Tanks()) {
        auto tank = std::make_unique<Tank>(*t);
        auto algo = tankFactory_.create(2, p2Count);
        p2Tanks_.push_back({std::move(tank), std::move(algo)});
        ++p2Count;
    }

    if (p1Tanks_.empty() || p2Tanks_.empty()) {
        std::cerr << "One or both players have no tanks.\n";
        return false;
    }

    player1_ = playerFactory_.create(1, board_.getWidth(), board_.getHeight(), maxSteps_, numShells_);
    player2_ = playerFactory_.create(2, board_.getWidth(), board_.getHeight(), maxSteps_, numShells_);
    outputFile_.open(outputFileName_);
    return true;
}

void GameManager::run() {
    for (int step = 0; step < maxSteps_; ++step) {
        handleBattleInfoRequests();
        resolveActions(p1Tanks_, 1);
        resolveActions(p2Tanks_, 2);
        cleanUpShells();
        logStep();
        if (isGameOver()) break;
    }
    outputFile_ << getFinalResult() << '\n';
}

void GameManager::logStep() {}
void GameManager::resolveActions(std::vector<TankWrapper>&, int) {}
void GameManager::handleBattleInfoRequests() {}
bool GameManager::isGameOver() const { return false; }
std::string GameManager::getFinalResult() const { return "Tie, reached max steps"; }

// GameManager.cpp - Additions for collision handling and shell movement

void GameManager::cleanUpShells() {
    std::vector<std::unique_ptr<Shell>> remainingShells;

    for (auto& shell : shells_) {
        for (int i = 0; i < 2; ++i) {
            if (!shell) break;
            shell->moveForward();
            Position pos = shell->getPosition();

            GameObject* obj = board_.getObjectsAt(pos);
            if (!obj) continue;

            if (Shell* otherShell = dynamic_cast<Shell*>(obj)) {
                resolveShellShellCollision(shell.get(), otherShell);
                shell.reset();  // remove current shell
                break;
            } else if (Tank* tank = dynamic_cast<Tank*>(obj)) {
                resolveShellTankCollision(shell.get(), tank);
                shell.reset();
                break;
            } else if (Wall* wall = dynamic_cast<Wall*>(obj)) {
                resolveShellWallCollision(shell.get(), wall);
                shell.reset();
                break;
            }
        }

        if (shell) {
            remainingShells.push_back(std::move(shell));
        }
    }

    shells_ = std::move(remainingShells);
}

void GameManager::resolveShellShellCollision(Shell* s1, Shell* s2) {
    board_.removeGameObject(s1->getPosition(), s1);
    board_.removeGameObject(s2->getPosition(), s2);
}

void GameManager::resolveShellTankCollision(Shell* shell, Tank* tank) {
    board_.removeGameObject(shell->getPosition(), shell);
    board_.removeGameObject(tank->getPosition(), tank);

    for (auto& t : p1Tanks_) {
        if (t.tank.get() == tank) t.isKilled = true;
    }
    for (auto& t : p2Tanks_) {
        if (t.tank.get() == tank) t.isKilled = true;
    }
}

void GameManager::resolveShellWallCollision(Shell* shell, Wall* wall) {
    board_.removeGameObject(shell->getPosition(), shell);
    wall->weaken();  // assume this decrements hitsLeft or similar logic

    if (wall->isDestroyed()) {
        board_.removeGameObject(wall->getPosition(), wall);
    }
}

void GameManager::resolveTankTankCollision(Tank* t1, Tank* t2) {
    board_.removeGameObject(t1->getPosition(), t1);
    board_.removeGameObject(t2->getPosition(), t2);

    for (auto& t : p1Tanks_) {
        if (t.tank.get() == t1 || t.tank.get() == t2) t.isKilled = true;
    }
    for (auto& t : p2Tanks_) {
        if (t.tank.get() == t1 || t.tank.get() == t2) t.isKilled = true;
    }
}

/*
 * void GameManager::cleanUpShells() {
    std::vector<std::unique_ptr<Shell>> remainingShells;

    for (auto& shell : shells_) {
        bool destroyed = false;

        for (int step = 0; step < 2 && !destroyed; ++step) {
            shell->moveForward();
            Position pos = shell->getPosition();
            const auto& objects = board_.getObjectsAt(pos);

            for (GameObject* obj : objects) {
                if (auto* otherShell = dynamic_cast<Shell*>(obj)) {
                    resolveShellShellCollision(shell.get(), otherShell);
                    destroyed = true;
                    break;
                } else if (auto* tank = dynamic_cast<Tank*>(obj)) {
                    resolveShellTankCollision(shell.get(), tank);
                    destroyed = true;
                    break;
                } else if (auto* wall = dynamic_cast<Wall*>(obj)) {
                    resolveShellWallCollision(shell.get(), wall);
                    destroyed = true;
                    break;
                }
            }
        }

        if (!destroyed) {
            remainingShells.push_back(std::move(shell));
        }
    }

    shells_ = std::move(remainingShells);
}

 void GameManager::resolveShellShellCollision(Shell* s1, Shell* s2) {
    Position pos = s1->getPosition();
    board_.removeObject(s1, pos);
    board_.removeObject(s2, pos);
}

 void GameManager::resolveShellTankCollision(Shell* shell, Tank* tank) {
    Position pos = shell->getPosition();
    board_.removeObject(shell, pos);
    board_.removeObject(tank, pos);

    for (auto& t : p1Tanks_)
        if (t.tank.get() == tank) t.isKilled = true;
    for (auto& t : p2Tanks_)
        if (t.tank.get() == tank) t.isKilled = true;
}

 void GameManager::resolveShellWallCollision(Shell* shell, Wall* wall) {
    Position pos = shell->getPosition();
    board_.removeObject(shell, pos);
    wall->decreaseLifeLeft();
    if (wall->isDestroyed()) {
        board_.removeObject(wall, pos);
    }
}

 void GameManager::resolveTankTankCollision(Tank* t1, Tank* t2) {
    Position pos1 = t1->getPosition();
    Position pos2 = t2->getPosition();
    board_.removeObject(t1, pos1);
    board_.removeObject(t2, pos2);

    for (auto& t : p1Tanks_)
        if (t.tank.get() == t1 || t.tank.get() == t2) t.isKilled = true;
    for (auto& t : p2Tanks_)
        if (t.tank.get() == t1 || t.tank.get() == t2) t.isKilled = true;
}





 */