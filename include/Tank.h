// Cleaned and integrated version of Tank and GameManager headers and implementation
// with corrected syntax, typos, and consistent handling

#ifndef TANK_H
#define TANK_H

#include <memory>
#include "GameObject.h"
#include "MovingGameObject.h"
#include "Position.h"
#include "Direction.h"
#include "TankAlgorithm.h"
#include "common/ActionRequest.h"

class Tank : public MovingGameObject {
public:
    Tank(Position pos, Direction dir, int playerId, int Id);

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
    ActionRequest getLastAction() const;
    std::unique_ptr<TankAlgorithm>& getAlgorithm();
    bool getIstRequestedBattleInfo() const;

    // Setters
    void setNextAction(ActionRequest action);
    void setLastAction(ActionRequest action);

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

    void decrementWaitToMoveBackCounter();
    void decrementWaitAfterShootCounter();

    // Actions utility functions
    void updateWaitToMoveBackCounter();
    void updateWaitAfterShootCounter();
    void resetIsWaitingAfterShoot();
    void resetIsWaitingToMoveBack();
    void resetIsRightAfterMoveBack();

    void destroy();
    bool isDestroyed() const;
    void setPosition(Position newPos);
    Position getPosition() const;
    Direction getDirection() const;

private:
    int playerId_;
    int id_;
    std::unique_ptr<TankAlgorithm> algorithm_;
    bool requestedBattleInfo_ = false;
    int shellsLeft_ = SHELLS_NUMBER;
    bool isWaitingAfterShoot_ = false;
    int waitAfterShootCounter_ = 0;
    bool isWaitingToMoveBack_ = false;
    int waitToMoveBackCounter_ = 0;
    bool isRightAfterMoveBack_ = false;
    ActionRequest nextAction_;
    ActionRequest lastAction_;

    void actualRotateEighthLeft();
    void actualRotateEighthRight();
};

#endif //TANK_H