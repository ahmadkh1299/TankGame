//
// Created by Radiant Forest on 21/04/2025.
//

#ifndef TANK_H
#define TANK_H

#include "MovingGameObject.h"
#include "Position.h"
#include "Direction.h"
#include "ActionRequest.h"

class Tank : public MovingGameObject {
public:
    Tank(Position pos, Direction dir, int playerId, int Id);

    static constexpr int SHELLS_NUMBER = 16;
    static constexpr int AFTER_SHOOT_WAIT_TURNS = 4;
    static constexpr int MOVE_BACK_WAIT_TURNS = 2;



    // Getters
    char getSymbol() const override; //each Tank Object will have a different symbol - "1" or "2", by the player it belongs to
    int getPlayerId() const;
    int getId() const;
    int getShellsLeft() const;
    bool getIsWaitingToMoveBack() const; //the game manager needs this info, cuz it manages the turns. and the turns loop is what changing it.
    bool getIsWaitingAfterShoot() const; //dido
    bool getIsRightAfterMoveBack() const;
    int getWaitToMoveBackCounter() const;
    ActionRequest getNextAction() const; //needed if we want to do all shooting first

    // Setters
    //void setShellsLeft();
    //void setWaitAfterShootCounter();
    //void setWaitToMoveBackCounter();
    void setNextAction(ActionRequest action);


    // Actions
    bool moveForward() override;
    bool askToMoveBack();
    bool moveBack();
    bool shoot();
    bool rotateEighthLeft();
    bool rotateFourthLeft();
    bool rotateEighthRight();
    bool rotateFourthRight();
    void doNothing();

    // Actions utility functions
    void updateWaitToMoveBackCounter();
    void updateWaitAfterShootCounter();
    void resetIsWaitingAfterShoot();

private:
    int playerId_ = 0;
    int id_ = 0;

    //shells and shoot state
    int shellsLeft_ = SHELLS_NUMBER;
    bool isWaitingAfterShoot_ = false; // cool down after shoot
    int waitAfterShootCounter_ = 0;

    //moving-back state
    bool isWaitingToMoveBack_ = false;
    int waitToMoveBackCounter_ = 0;
    bool isRightAfterMoveBack_ = false;

    ActionRequest nextAction_; // not sure if needed

    // private methods - actions utility functions
    void resetIsRightAfterMoveBack();
    void actualRotateEighthLeft();
    void actualRotateEighthRight();
};

#endif //TANK_H
