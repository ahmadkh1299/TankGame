#ifndef TANK_H
#define TANK_H

#include "MovingGameObject.h"
#include "Position.h"
#include "Direction.h"
#include "../include/common/ActionRequest.h"

class Tank : public MovingGameObject {
public:
    Tank(Position pos, Direction dir, int playerId, int id);

    static constexpr int SHELLS_NUMBER = 16;
    static constexpr int AFTER_SHOOT_WAIT_TURNS = 4;
    static constexpr int MOVE_BACK_WAIT_TURNS = 2;

    // Getters
    char getSymbol() const override;
    int getPlayerId() const;
    int getId() const;
    int getShellsLeft() const;
    bool getIsWaitingToMoveBack() const;
    bool getIsWaitingAfterShoot() const;
    bool getIsRightAfterMoveBack() const;
    int getWaitToMoveBackCounter() const;
    ActionRequest getNextAction() const;

    // Setters
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

    // Utility Functions
    void updateWaitToMoveBackCounter();
    void updateWaitAfterShootCounter();
    void resetIsWaitingAfterShoot();
    void resetIsWaitingToMoveBack();  // <- Added as required by GameManager

private:
    int playerId_ = 0;
    int id_ = 0;

    // Shooting state
    int shellsLeft_ = SHELLS_NUMBER;
    bool isWaitingAfterShoot_ = false;
    int waitAfterShootCounter_ = 0;

    // Backward movement state
    bool isWaitingToMoveBack_ = false;
    int waitToMoveBackCounter_ = 0;
    bool isRightAfterMoveBack_ = false;

    ActionRequest nextAction_;

    // Internal helpers
    void resetIsRightAfterMoveBack();
    void actualRotateEighthLeft();
    void actualRotateEighthRight();
};

#endif // TANK_H
