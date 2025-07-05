#include "GameManager.h"
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <type_traits>

GameManager::GameManager(const PlayerFactory& playerFactory,
                         const TankAlgorithmFactory& tankFactory)
        : playerFactory_(playerFactory),
          tankFactory_(tankFactory),
          board_(1, 1) {}

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

    int p1Count = 0;
    for (const auto& t : parser.getPlayer1Tanks()) {
        auto tank = std::make_unique<Tank>(*t);
        auto algo = tankFactory_.create(1, p1Count);
        p1Tanks_.push_back({std::move(tank), std::move(algo)});

        // Add the tank to the board
    	Tank* rawTankPtr = p1Tanks_.back().tank.get();
    	board_.addGameObject(rawTankPtr, rawTankPtr->getPosition());

        ++p1Count;
    }

    int p2Count = 0;
    for (const auto& t : parser.getPlayer2Tanks()) {
        auto tank = std::make_unique<Tank>(*t);
        auto algo = tankFactory_.create(2, p2Count);
        p2Tanks_.push_back({std::move(tank), std::move(algo)});
        // Add the tank to the board
    	Tank* rawTankPtr = p2Tanks_.back().tank.get();
    	board_.addGameObject(rawTankPtr, rawTankPtr->getPosition());
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
bool gameEndedByBreak = false;
    while (stepsLeftWhenShellsOver_ > 0 && stepCounter_ < MAX_TOTAL_STEPS)
    {
      printToFile("\n--- Step " + std::to_string(stepCounter_) + " ---\n");

      //***counters handling***
      for (Tank& t : p1Tanks_) {
        if (!t.isDestroyed) {countersHandler(t);}
      }
      for (Tank& t : p2Tanks_) {
        if (!t.isDestroyed) {countersHandler(t);}
      }

	  //***deciding tanks actions***
      for (Tank& t : p1Tanks_) {
      	if (!t.isDestroyed) { decideAction(t);}
      }
      for (Tank& t : p2Tanks_) {
      	if (!t.isDestroyed)) { decideAction(t);}
      }

      //***handle shooting***
	  for (Tank& t : p1Tanks_) {
      	if (!t.isDestroyed && t.getNextAction() == ActionRequest::Shoot) {
          handleShoot(t);
        }
      }
       for (Tank& t : p2Tanks_) {
      	if (!t.isDestroyed && t.getNextAction() == ActionRequest::Shoot) {
          handleShoot(t);
        }
      }

      //***move shells and check collisions***
      for (int i = 0; i < SHELL_MOVES_PER_STEP; ++i) {
        shellStep();
      	cleanupDestroyedObjects(shells_);
      	cleanupDestroyedObjects(walls_);
      	cleanupDestroyedObjects(tanks_);
      }

      //***handle other actions (not shooting)***
	  for (Tank& t : p1Tanks_) {
      	if (!t.isDestroyed && t.getNextAction() != ActionRequest::Shoot) {
          handleAction(t, t.getNextAction());
        }
      }
	  for (Tank& t : p2Tanks_) {
      	if (!tw.isDestroyed && t.getNextAction() != ActionRequest::Shoot) {
          handleAction(t, t.getNextAction());
        }
      }

    //***check if we need to move the tank back and move it, if yes***
    for (Tank& t : p1Tanks_) {
        if (!t.isDestroyed) {handleAutoMoveTankBack(t);}
    }
	for (Tank& t : p2Tanks_) {
    	if (!t.isDestroyed) {handleAutoMoveTankBack(t);}
    }

    //***resolve collisions***
    //it's only tank+mine and tank+tank. need to add tanks to the board, and handle tank+tank situation
	for (Tank& t : p1Tanks_) {
        if (!t.isDestroyed) {resolveTankCollisionsAtPosition(t);}
    }
	for (Tank& t : p2Tanks_) {
    	if (!t.isDestroyed) {resolveTankCollisionsAtPosition(t);}
    }

    cleanupDestroyedObjects(mines_);
    cleanupDestroyedObjects(tanks_);


    //***check for winner***
    if (p1Tanks_.empty() && !p2Tanks_.empty())
    {
    	printToFile("\nPlayer 2 wins! (Player 1 has no tanks left)\n)");
        gameEndedByBreak = true;
        break;
    }

    if (!p1Tanks_.empty() && p2Tanks_.empty())
    {
    	printToFile("\nPlayer 1 wins! (Player 2 has no tanks left)\n)");
        gameEndedByBreak = true;
        break;
    }

    if (p1Tanks_.empty() && p2Tanks_.empty())
    {
    	printToFile("\nIt's a tie! (Both players have no tanks left)\n)");
        gameEndedByBreak = true;
        break;
    }

    //***check if we are in no_shells_left situation***
    if (!isPlayWhenShellsOver_) //to avoid count the shells, if it's already true
    {
      if (getTotalShellsLeft() <= 0) isPlayWhenShellsOver_ = true;
    }
    if (isPlayWhenShellsOver_) stepsLeftWhenShellsOver_--;

    stepCounter_++;
    }

//***check if we are out the loop cuz we got to maximum steps or no more shells ***
	if (!gameEndedByBreak) {
   		if (stepCounter_ >= MAX_TOTAL_STEPS) {
            printToFile("\nGame stopped: max steps exceeded\n");
            return;
        } else {
            printToFile("\nit's a tie! (There are no more shells, and 40 turns have passed)\n");
            return;
        }
    }

    return;

}

//Handling tank's actions
void GameManager::handleMoveTankForward(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.moveForward()) {
        printBadStep(tank, ActionRequest::MoveForward);
        return;
    }
    Position newPos = tank.getPosition();
    newPos.wrap(boardWidth_, boardHeight_);

    //check if there is wall in the new position.
    //if there is, print bad step and do not move the tank, aka do not change the tank's position
    const auto& objects = board_.getObjectsAt(newPos);
    for (const auto& obj : objects)
    {
        if (dynamic_cast<Wall*>(obj)) {
            printBadStep(tank, ActionRequest::MoveForward);
            return;
        }
    }

    //if there is no wall
    tank.setPosition(newPos);
    printGoodStep(tank, ActionRequest::MoveForward);
}

void GameManager::handleMoveTankBack(Tank& tank) {
    if (tank.getIsRightAfterMoveBack() || (tank.getIsWaitingToMoveBack() && tank.getWaitToMoveBackCounter() == 0) )
    {
        //can move back

        //extra check, cuz if we call it, it should succeed
        if (!tank.moveBack()) { //this tank's function sets isRightAfterMoveBack = T
          printToFile(("Actual moving back - tank can't move back for some reason"));
          tank.resetIsRightAfterMoveBack(); //isRightAfterMoveBack = F
          return;
        }

        Position newPos = tank.getPosition();
        newPos.wrap(boardWidth_, boardHeight_);

        //check if there is wall in the new position.
        //if there is, print bad step and do not move the tank, aka do not change the tank's position
        const auto& objects = board_.getObjectsAt(newPos);
        for (const auto& obj : objects)
        {
            if (dynamic_cast<Wall*>(obj)) {
                printToFile(("Actual moving back - tank can't move back because there is a wall"));
                tank.resetIsRightAfterMoveBack(); //isRightAfterMoveBack = F
                return;
            }
        }

        //if there is no wall
        tank.setPosition(newPos);
        printToFile(("Actual moving back - success"));
        return;
    }
}


void GameManager::handleTankAskMoveBack(Tank& tank) { //notice tha name of the action is "MoveBackward", but it's just asking
    if (tank.getIsRightAfterMoveBack()){
        //can immediately move, no need to ask
        printGoodStep(tank, MoveBackward::MoveBackward);
        handleMoveTankBack(tank);
        return;
    }
    if (!tank.askToMoveBack()) { //return false if the tank is already in waiting state
        printBadStep(tank, MoveBackward::MoveBackward);
        return;
    }
    printGoodStep(tank, MoveBackward::MoveBackward);
}

void GameManager::handleShoot(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.shoot()) {
        printBadStep(tank, ActionRequest::Shoot);
        return;
    }
    shells_.push_back(std::make_unique<Shell>(tank.getPosition(),tank.getDirection(),tank.getId() ) );

    //no need to add the shell to board_, shellStep() is handling this

    printGoodStep(tank, ActionRequest::Shoot);
}

void GameManager::handleRotateEighthLeft(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateEighthLeft()) {printBadStep(tank, ActionRequest::RotateLeft45);}
    printGoodStep(tank, ActionRequest::RotateLeft45);
}
void GameManager::handleRotateFourthLeft(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateFourthLeft()) {printBadStep(tank, ActionRequest::RotateLeft90);}
    printGoodStep(tank, ActionRequest::RotateLeft90);
}
void GameManager::handleRotateEighthRight(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateEighthRight()) {printBadStep(tank, ActionRequest::RotateRight45);}
    printGoodStep(tank, ActionRequest::RotateRight45);
}
void GameManager::handleRotateFourthRight(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    if (!tank.rotateFourthRight()) printBadStep(tank, ActionRequest::RotateRight90);
    printGoodStep(tank, ActionRequest::RotateRight90);
}
void GameManager::handleDoNothing(Tank& tank) {
    tank.resetIsRightAfterMoveBack();
    tank.doNothing();
    printGoodStep(tank, ActionRequest::DoNothing);
}

void GameManager::handleRequestBattleInfo(Tank& tank) {
  tank.resetIsRightAfterMoveBack(); //change "is right after move back" to false
  //add function here
  //printBadStep(tank, ActionRequest::GetBattleInfo);
  //printGoodStep(tank, ActionRequest::GetBattleInfo);
  return;
}

//not including shoot
void GameManager::handleAction(Tank& tank, ActionRequest action) {
    switch (action) {
        case ActionRequest::MoveForward: handleMoveTankForward(tank); break;
        case ActionRequest::MoveBackward: handleTankAskMoveBack(tank); break;
        case ActionRequest::RotateLeft45: handleRotateEighthLeft(tank); break;
        case ActionRequest::RotateLeft90: handleRotateFourthLeft(tank); break;
        case ActionRequest::RotateRight45: handleRotateEighthRight(tank); break;
        case ActionRequest::RotateRight90: handleRotateFourthRight(tank); break;
        case ActionRequest::DoNothing: handleDoNothing(tank); break;
        case ActionRequest::GetBattleInfo: handleGetBattleInfo(tank); break;
        //case ActionRequest::Shoot: handleShoot(tank); break;
        default: printBadStep(tank, action); break;
    }
}


//helper functions for the run() function
void  GameManager::decideAction(Tank& t) {
//HOW
//std::unique_ptr<TankAlgorithm> algo = t.getAlgorithm();
return;
}

int GameManager::getTotalShellsLeft() const {
    int total = 0;
    for (const Tank& t : p1Tanks_) total += t.getShellsLeft();
    for (const Tank& t : p2Tanks_) total += t.getShellsLeft();
    return total;
}


void  GameManager::countersHandler(Tank& tank)
{

    tank.updateWaitToMoveBackCounter(); //decrease counter only if the tank is waiting to move back + counter>0
    tank.resetIsWaitingToMoveBack(); //change waiting_to_move_back to false, only if the tank was in waiting state + counter==0

    tank.updateWaitAfterShootCounter(); //decrease counter only if the tank is after shoot + counter>0
    tank.resetIsWaitingAfterShoot(); //change waiting_after_shoot to false, only if the tank is after shoot + counter==0
}

//game manager "automatically" moves the tank back, if the tank asked and waited 2 turns. 3rd turn is the move.
//this function is called if !t.isDestroyed, so no need to check
void GameManager::handleAutoMoveTankBack(Tank& tank)
{
    //move tank back, if the tank asked and 2 turns has passed (counter==0)
    if (tank.getIsWaitingToMoveBack() && tank.getWaitToMoveBackCounter()==0 )
    {
        handleMoveTankBack(tan!tank.isDestroyed() && k);
    }
}



void GameManager::resolveShellCollisionsAtPosition(Shell& shell) {
    Position pos = shell.getPosition();
    const auto& objects = board_.getObjectsAt(pos);
    for (const auto& obj : objects) {
        if (Wall* wall = dynamic_cast<Wall*>(obj)) {
            if (wall->getLifeLeft() == 2) {
                wall->decreaseLifeLeft();
            } else if (wall->getLifeLeft() == 1) {
                wall->destroy();
            }
            shell.destroy();
        } if (Shell* anotherShell = dynamic_cast<Shell*>(obj))
        {
            anotherShell->destroy();
            shell.destroy();
        } if (Tank* tank = dynamic_cast<Tank*>(obj))
        {
            tank->destroy();
            shell.destroy();
        }
    }
}

//function to remove the destroyed objects from board_ and also from the specific vector
//each vector is needed, to prevent looping through the whole board
template<typename T>
void GameManager::cleanupDestroyedObjects(std::vector<std::unique_ptr<T>>& vec) {
    vec.erase(std::remove_if(vec.begin(), vec.end(),
        [&](std::unique_ptr<T>& obj) {
            if (obj->isDestroyed()) {
                board_.removeObject(obj.get(), obj->getPosition()); // remove from board
                return true; // remove from vector
            }
            return false;
        }), vec.end());
}

//explicit instantiations
template void GameManager::cleanupDestroyedObjects<Shell>(std::vector<std::unique_ptr<Shell>>& vec);
template void GameManager::cleanupDestroyedObjects<Tank>(std::vector<std::unique_ptr<Tank>>& vec);
template void GameManager::cleanupDestroyedObjects<Mine>(std::vector<std::unique_ptr<Mine>>& vec);
template void GameManager::cleanupDestroyedObjects<Wall>(std::vector<std::unique_ptr<Wall>>& vec);


void GameManager::moveForwardAndWrap(Shell& shell) {
    shell.moveForward();
    Position newPos = shell.getPosition();
    newPos.wrap(boardWidth_, boardHeight_);
    shell.setPosition(newPos);
}

void GameManager::resolveTankCollisionsAtPosition(Tank& tank) {
    Position pos = tank.getPosition();
    const auto& objects = board_.getObjectsAt(pos);

    for (const auto& obj : objects) {
        if (Mine* mine = dynamic_cast<Mine*>(obj)) {
            mine->destroy();
            tank.destroy();
        }
        if (Tank* anotherTank = dynamic_cast<Tank*>(obj)) {
            anotherTank->destroy();
            tank.destroy();
        }
    }
}

void GameManager::shellStep()
{
    //Move shells and update board
    for (auto& shellPtr : shells_)
    {
        moveForwardAndWrap(*shellPtr);
        board_.addGameObject(shellPtr.get(), shellPtr->getPosition());
    }

    //Check for collisions
    for (auto& shellPtr : shells_)
    {
        if (!shellPtr->isDestroyed())
        {
            Position pos = shellPtr->getPosition();
            handleShellAtPosition(*shellPtr, pos);
        }
    }
}


//get pointers
std::vector<Shell*> GameManager::getShellPtrs() const {
    std::vector<Shell*> result;
    for (const auto& shell : shells_)
    {
        result.push_back(shell.get());
    }
    return result;
}


std::vector<Mine*> GameManager::getMinePtrs() const {
    std::vector<Mine*> result;
    for (const auto& mine : mines_)
    {
        result.push_back(mine.get());
    }
    return result;
}

//walls?
//tanks?

//printing functions
void GameManager::printBadStep(Tank& tank, ActionRequest action) {
    printToFile("Player " + std::to_string(tank.getPlayerId()) +
    " asked for a bad step: " + ActionUtils::toString(action));
}

void GameManager::printGoodStep(Tank& tank, ActionRequest action) {
    printToFile("Player " + std::to_string(tank.getPlayerId()) +
                " did step: " + ActionUtils::toString(action));
}

void GameManager::printToFile(const std::string& message){
    // std::cout << message << std::endl; //print to console
    if (outputFile_.is_open()) {
        outputFile_ << message << std::endl;
    }

}

