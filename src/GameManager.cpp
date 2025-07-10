#include "../include/GameManager.h"
#include "../include/SatelliteViewImpl.h"
#include "../include/MyBattleInfo.h"
#include <iostream>
#include <fstream>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <memory>
#include "MyTankAlgorithm.h"


GameManager::GameManager(std::unique_ptr<PlayerFactory> playerFactory,
                         std::unique_ptr<TankAlgorithmFactory> tankFactory)
        : playerFactory_(std::move(playerFactory)),
          tankFactory_(std::move(tankFactory)),
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
    allTanksSorted_ = sortAllTanks(p1Tanks_, p2Tanks_);

    for (size_t i = 0; i < p1Tanks_.size(); ++i) {
        auto algo = tankFactory_->create(1, static_cast<int>(i));
        p1Tanks_[i]->setAlgorithm(std::make_unique<MyTankAlgorithm>(
                std::move(algo), 1, static_cast<int>(i)));
    }

    for (size_t i = 0; i < p2Tanks_.size(); ++i) {
        auto algo = tankFactory_->create(2, static_cast<int>(i));
        p2Tanks_[i]->setAlgorithm(std::make_unique<MyTankAlgorithm>(
                std::move(algo), 2, static_cast<int>(i)));
    }



    if (p1Tanks_.empty() || p2Tanks_.empty()) {
        std::cerr << "One or both players have no tanks.\n";
        return false;
    }

    player1_ = playerFactory_->create(1, boardWidth_, boardHeight_, maxSteps_, numShells_);
    player2_ = playerFactory_->create(2, boardWidth_, boardHeight_, maxSteps_, numShells_);


    //Open output file and name it
    std::string outputFileName = "output_" + inputFile;
    outputFile_.open("../" + outputFileName);
    if (!outputFile_) {
        std::cerr << "Failed to open output file: " << outputFileName << std::endl;
        return false;
    }

    return true;
}

void GameManager::run() {
    int stepCounter = 0;
    stepsLeftWhenShellsOver_ = STEPS_WHEN_SHELLS_OVER;
    int p1Alive;
    int p2Alive;

    while (stepCounter < maxSteps_ && stepsLeftWhenShellsOver_ > 0) {
        printToFile("\n--- Step " + std::to_string(stepCounter) + " ---");

        //reset "setWasKilledThisStep" for all tanks
        for (auto& t : p1Tanks_) {
            t->setWasKilledThisStep(false);
        }

        for (auto& t : p2Tanks_) {
            t->setWasKilledThisStep(false);
        }

        //***counters handling***
        for (auto& t : p1Tanks_) if (!t->isDestroyed()) countersHandler(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed()) countersHandler(*t);

        //***deciding tanks actions***
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

        //***handle get battle info***
        //it's first cuz the view is of the board before the current step
        for (auto& t : p1Tanks_) if (!t->isDestroyed() && t->getNextAction() == ActionRequest::GetBattleInfo) handleRequestBattleInfo(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed() && t->getNextAction() == ActionRequest::GetBattleInfo) handleRequestBattleInfo(*t);

        //***handle shooting***
        //it's before other actions (except get battle info) cuz this is the choice we made in the game logic:
        //first, we move the shells. then, we move the tanks.
        for (auto& t : p1Tanks_) if (!t->isDestroyed() && t->getNextAction() == ActionRequest::Shoot) handleShoot(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed() && t->getNextAction() == ActionRequest::Shoot) handleShoot(*t);

        for (int i = 0; i < shellMovesPerStep_; ++i) {
            shellStep(); //collisions are handled inside this function
            cleanupDestroyedObjects(shells_);
            cleanupDestroyedObjects(walls_);
            //cleanupDestroyedObjects(p1Tanks_); //not cleaning, it's needed for creating output file
            //cleanupDestroyedObjects(p2Tanks_); //not cleaning, it's needed for creating output file
            //no need to clean mines at this point. shells do not hit mines.
        }

        //***handle other actions (not get battle info / shoot)***
        for (auto& t : p1Tanks_) if (!t->isDestroyed() && t->getNextAction() != ActionRequest::Shoot && t->getNextAction() != ActionRequest::GetBattleInfo) handleAction(*t, t->getNextAction());
        for (auto& t : p2Tanks_) if (!t->isDestroyed() && t->getNextAction() != ActionRequest::Shoot && t->getNextAction() != ActionRequest::GetBattleInfo) handleAction(*t, t->getNextAction());

        //***check if we need to move the tank back and move it, if yes***
         for (auto& t : p1Tanks_) if (!t->isDestroyed()) handleAutoMoveTankBack(*t);
         for (auto& t : p2Tanks_) if (!t->isDestroyed()) handleAutoMoveTankBack(*t);

        //***resolve collisions***
        for (auto& t : p1Tanks_) if (!t->isDestroyed()) resolveTankCollisionsAtPosition(*t);
        for (auto& t : p2Tanks_) if (!t->isDestroyed()) resolveTankCollisionsAtPosition(*t);

        //cleanupDestroyedObjects(p1Tanks_); //not cleaning, it's needed for creating output file
        //cleanupDestroyedObjects(p2Tanks_); //not cleaning, it's needed for creating output file
        cleanupDestroyedObjects(mines_);
        //no need to clean shells at this point. shells has been handled before.
        //no need to clean walls at this point, cuz tanks can not hit walls.

        if (checkIfPlayerLostAllTanks(p1Alive, p2Alive)) {break;}  //this returns true if a player, or both, lost all of his tanks.
        //it also counts the alive tanks of each player, and keep it in p1Alive, p2Alive

        if (getTotalShellsLeft() <= 0) --stepsLeftWhenShellsOver_;
        stepCounter++;

        printRoundToFile();
    }

    printGameResult(p1Alive, p2Alive);
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

int GameManager::getTotalShellsLeft() const
{
    int total = 0;
    for (const auto& t : p1Tanks_) total += t->getShellsLeft();
    for (const auto& t : p2Tanks_) total += t->getShellsLeft();
    return total;
}


void GameManager::handleRequestBattleInfo(Tank& tank) {
    //pre-action reset and check
    tank.resetIsRightAfterMoveBack();
    if (tank.getIsWaitingToMoveBack())
    {
      tank.setWasLastActionIgnored(true);
      return;
    }

    //create a 2D char representation of the board
    std::vector<std::vector<char>> view(boardHeight_, std::vector<char>(boardWidth_, ' '));

    for (size_t y = 0; y < boardHeight_; ++y) {
        for (size_t x = 0; x < boardWidth_; ++x) {
            const auto& objects = board_.getObjectsAt({(int)x, (int)y});
            if (!objects.empty()) {
                view[y][x] = objects.front()->getSymbol();  //show only top object
            }
        }
    }

    //create a SatelliteViewImpl from it
    SatelliteViewImpl satellite(view);

    //determine which player owns this tank
    int playerId = tank.getPlayerId();
    Player* player = (playerId == 1) ? player1_.get() : player2_.get();

    //let the player update the tank's algorithm with BattleInfo
    if (player) {
        if (!tank.getAlgorithm()) {
            std::cerr << "Tank has no algorithm! Player: " << tank.getPlayerId()
                      << ", Tank ID: " << tank.getId() << std::endl;
            return;
        }
        player->updateTankWithBattleInfo(*tank.getAlgorithm(), satellite);
    }

    tank.setLastAction(ActionRequest::GetBattleInfo);
    tank.setWasLastActionIgnored(false);
}



void GameManager::handleShoot(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.shoot()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    shells_.push_back(std::make_unique<Shell>(tank.getPosition(), tank.getDirection(), tank.getId()));
    tank.setLastAction(ActionRequest::Shoot);
    tank.setWasLastActionIgnored(false);
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

void GameManager::countersHandler(Tank& tank)
{
    tank.updateWaitToMoveBackCounter(); //decrease counter only if the tank is waiting to move back + counter>0
    tank.resetIsWaitingToMoveBack(); //change waiting_to_move_back to false, only if the tank was in waiting state + counter==0

    tank.updateWaitAfterShootCounter(); //decrease counter only if the tank is after shoot + counter>0
    tank.resetIsWaitingAfterShoot(); //change waiting_after_shoot to false, only if the tank is after shoot + counter==0
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
            tank->setWasKilledThisStep(true);
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
            tank.setWasKilledThisStep(true);
        } else if (auto* anotherTank = dynamic_cast<Tank*>(obj)) {
            if (anotherTank != &tank) {
                anotherTank->destroy();
                anotherTank->setWasKilledThisStep(true);
                tank.destroy();
                tank.setWasKilledThisStep(true);
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
    Position oldPos = tank.getPosition();

    if (!tank.moveForward()) {
        tank.setWasLastActionIgnored(true);
        return;
    }

    Position newPos = tank.getPosition();
    newPos.wrap(boardWidth_, boardHeight_);

    //check if there is wall in the new position.
    //if there is, print bad step (=ignored) and do not move the tank, aka do not change the tank's position
    const auto& objects = board_.getObjectsAt(newPos);
    for (const auto& obj : objects)
    {
        if (dynamic_cast<Wall*>(obj)) {
            tank.setPosition(oldPos);
            tank.setWasLastActionIgnored(true);
            return;
        }
    }

    //if there is no wall
    tank.setPosition(newPos);
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::MoveForward);
}

void GameManager::handleMoveTankBack(Tank& tank) {
    Position oldPos = tank.getPosition();

    //in this case, we treat moving back as action request
    if (tank.getIsRightAfterMoveBack())
    {
        if (!tank.moveBack())
        {
            tank.setWasLastActionIgnored(true);
            return;
        }

        Position newPos = tank.getPosition();
        newPos.wrap(boardWidth_, boardHeight_);

        //check if there is wall in the new position.
        //if there is, print bad step (=ignored) and do not move the tank, aka do not change the tank's position
        const auto& objects = board_.getObjectsAt(newPos);
        for (const auto& obj : objects)
        {
            if (dynamic_cast<Wall*>(obj)) {
                tank.setPosition(oldPos);
                tank.setWasLastActionIgnored(true);
                return;
            }
        }

        //if there is no wall
        tank.setWasLastActionIgnored(false);
        tank.setLastAction(ActionRequest::MoveBackward);
    }

    //in this case, we don't treat moving back as action request (it happens automatically after the waiting)
    if (tank.getIsWaitingToMoveBack() && tank.getWaitToMoveBackCounter() == 0)
    {
       if (!tank.moveBack())
       {
          return;
       }
       Position newPos = tank.getPosition();
       newPos.wrap(boardWidth_, boardHeight_);

       //check if there is wall in the new position.
       //if there is, print bad step (=ignored) and do not move the tank, aka do not change the tank's position
       const auto& objects = board_.getObjectsAt(newPos);
       for (const auto& obj : objects)
       {
         if (dynamic_cast<Wall*>(obj)) {
            tank.setPosition(oldPos);
            return;
         }
       }
    }
}

void GameManager::handleTankAskMoveBack(Tank& tank) {
    if (tank.getIsRightAfterMoveBack()) {
        handleMoveTankBack(tank); //setWasLastActionIgnored happens inside this
        return;
    }
    if (!tank.askToMoveBack()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::MoveBackward);
}

void GameManager::handleRotateEighthLeft(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateEighthLeft()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateLeft45);
}

void GameManager::handleRotateFourthLeft(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateFourthLeft()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateLeft90);
}

void GameManager::handleRotateEighthRight(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateEighthRight()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateRight45);
}

void GameManager::handleRotateFourthRight(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateFourthRight()) {
        tank.setWasLastActionIgnored(true);
        return;
    }
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::RotateRight90);
}

void GameManager::handleDoNothing(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    tank.doNothing();
    tank.setWasLastActionIgnored(false);
    tank.setLastAction(ActionRequest::DoNothing);
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


template void GameManager::cleanupDestroyedObjects<Shell>(std::vector<std::unique_ptr<Shell>>&);
template void GameManager::cleanupDestroyedObjects<Wall>(std::vector<std::unique_ptr<Wall>>&);
template void GameManager::cleanupDestroyedObjects<Tank>(std::vector<std::unique_ptr<Tank>>&);
template void GameManager::cleanupDestroyedObjects<Mine>(std::vector<std::unique_ptr<Mine>>&);

//for creating the output file
std::vector<Tank*> GameManager::sortAllTanks(const std::vector<std::unique_ptr<Tank>>& p1Tanks,
                                const std::vector<std::unique_ptr<Tank>>& p2Tanks)
{
    std::vector<Tank*> allTanks;

    for (const auto& t : p1Tanks)
       allTanks.push_back(t.get());

    for (const auto& t : p2Tanks)
        allTanks.push_back(t.get());

    //sort tanks by birth order (top to bottom (y axis), left to right (x axis))
    std::sort(allTanks.begin(), allTanks.end(), [](Tank* a, Tank* b) {
        const Position& pa = a->getPosition();
        const Position& pb = b->getPosition();
        return (pa.getY() != pb.getY()) ? pa.getY() < pb.getY() : pa.getX() < pb.getX();
    });

    return allTanks;
}

void GameManager::printRoundToFile()
{
    for (size_t i = 0; i < allTanksSorted_.size(); ++i) {
        Tank* tank = allTanksSorted_[i];

        if (tank->isDestroyed()) {
            outputFile_ << "killed";
        } else {
            std::string actionStr = ActionUtils::toString(tank->getNextAction());

            if (tank->getWasLastActionIgnored())
                actionStr += " (ignored)";
            if (tank->getWasKilledThisStep())
                actionStr += " (killed)";

            outputFile_ << actionStr;
        }

        if (i < allTanksSorted_.size() - 1)
            outputFile_ << ", ";
    }

    outputFile_ << "\n";
}


void GameManager::printGameResult(int p1Alive, int p2Alive)
{
    if (p1Alive > 0 && p2Alive == 0) {
        outputFile_ << "Player 1 won with " << p1Alive << " tanks still alive\n";
    } else if (p2Alive > 0 && p1Alive == 0) {
        outputFile_ << "Player 2 won with " << p2Alive << " tanks still alive\n";
    } else if (p1Alive == 0 && p2Alive == 0) {
        outputFile_ << "Tie, both players have zero tanks\n";
    } else if (stepsLeftWhenShellsOver_ >= STEPS_WHEN_SHELLS_OVER) {
        outputFile_ << "Tie, both players have zero shells for <"
                    << STEPS_WHEN_SHELLS_OVER << "> steps\n";
    } else {
        outputFile_ << "Tie, reached max steps = " << maxSteps_
                    << ", player 1 has " << p1Alive
                    << " tanks, player 2 has " << p2Alive << " tanks\n";
    }
}

//returns true if a player, or both, lost all of his tanks.
//else, returns false
//also count the tanks, on the way
bool GameManager::checkIfPlayerLostAllTanks(int& p1Alive, int& p2Alive)
{
    p1Alive = std::count_if(p1Tanks_.begin(), p1Tanks_.end(),
                                [](const std::unique_ptr<Tank>& t) { return !t->isDestroyed(); });

    p2Alive = std::count_if(p2Tanks_.begin(), p2Tanks_.end(),
                                [](const std::unique_ptr<Tank>& t) { return !t->isDestroyed(); });

    if (p1Alive == 0 || p2Alive == 0) {
       return true;
    }

    return false;
}


