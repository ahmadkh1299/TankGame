#include "../include/Tank.h"
#include "../include/Direction.h"
#include "../include/Position.h"
#include "../include/MovingGameObject.h"

Tank::Tank(Position pos, Direction dir, int playerId, int id)
        : MovingGameObject(pos, dir), playerId_(playerId), id_(id) {}

bool Tank::moveForward() {
    if (isWaitingToMoveBack_) {
        isWaitingToMoveBack_ = false;
        return true;
    }

    switch (dir_) {
        case Direction::Up: pos_.moveUp(); break;
        case Direction::Down: pos_.moveDown(); break;
        case Direction::Left: pos_.moveLeft(); break;
        case Direction::Right: pos_.moveRight(); break;
        case Direction::UpLeft: pos_.moveUp(); pos_.moveLeft(); break;
        case Direction::UpRight: pos_.moveUp(); pos_.moveRight(); break;
        case Direction::DownLeft: pos_.moveDown(); pos_.moveLeft(); break;
        case Direction::DownRight: pos_.moveDown(); pos_.moveRight(); break;
        default: return false;
    }

    resetIsRightAfterMoveBack();
    return true;
}

bool Tank::askToMoveBack() {
    if (!isWaitingToMoveBack_) {
        isWaitingToMoveBack_ = true;
        waitToMoveBackCounter_ = MOVE_BACK_WAIT_TURNS;
        return true;
    }
    return false;
}

bool Tank::moveBack() {
    if (isRightAfterMoveBack_ || (isWaitingToMoveBack_ && waitToMoveBackCounter_ == 0)) {
        switch (dir_) {
            case Direction::Up: pos_.moveDown(); break;
            case Direction::Down: pos_.moveUp(); break;
            case Direction::Left: pos_.moveRight(); break;
            case Direction::Right: pos_.moveLeft(); break;
            case Direction::UpLeft: pos_.moveDown(); pos_.moveRight(); break;
            case Direction::UpRight: pos_.moveDown(); pos_.moveLeft(); break;
            case Direction::DownLeft: pos_.moveUp(); pos_.moveRight(); break;
            case Direction::DownRight: pos_.moveUp(); pos_.moveLeft(); break;
            default: return false;
        }

        isRightAfterMoveBack_ = true;
        isWaitingToMoveBack_ = false;
        return true;
    }
    return false;
}

void Tank::updateWaitToMoveBackCounter() {
    if (isWaitingToMoveBack_ && waitToMoveBackCounter_ > 0)
        waitToMoveBackCounter_--;
}

void Tank::resetIsWaitingToMoveBack() {
    if (isWaitingToMoveBack_ && waitToMoveBackCounter_ == 0)
        isWaitingToMoveBack_ = false;
}

void Tank::resetIsRightAfterMoveBack() {
    isRightAfterMoveBack_ = false;
}

int Tank::getWaitToMoveBackCounter() const {
    return waitToMoveBackCounter_;
}

void Tank::actualRotateEighthLeft() {
    dir_ = static_cast<Direction>((static_cast<int>(dir_) + 7) % 8);
}

void Tank::actualRotateEighthRight() {
    dir_ = static_cast<Direction>((static_cast<int>(dir_) + 1) % 8);
}

bool Tank::rotateEighthLeft() {
    if (isWaitingToMoveBack_) return false;
    actualRotateEighthLeft();
    resetIsRightAfterMoveBack();
    return true;
}

bool Tank::rotateFourthLeft() {
    if (isWaitingToMoveBack_) return false;
    actualRotateEighthLeft();
    actualRotateEighthLeft();
    resetIsRightAfterMoveBack();
    return true;
}

bool Tank::rotateEighthRight() {
    if (isWaitingToMoveBack_) return false;
    actualRotateEighthRight();
    resetIsRightAfterMoveBack();
    return true;
}

bool Tank::rotateFourthRight() {
    if (isWaitingToMoveBack_) return false;
    actualRotateEighthRight();
    actualRotateEighthRight();
    resetIsRightAfterMoveBack();
    return true;
}

bool Tank::shoot() {
    if (isWaitingToMoveBack_ || (isWaitingAfterShoot_ && waitAfterShootCounter_ > 0) || shellsLeft_ <= 0)
        return false;

    shellsLeft_--;
    isWaitingAfterShoot_ = true;
    waitAfterShootCounter_ = AFTER_SHOOT_WAIT_TURNS;
    resetIsRightAfterMoveBack();
    return true;
}

void Tank::updateWaitAfterShootCounter() {
    if (isWaitingAfterShoot_ && waitAfterShootCounter_ > 0)
        waitAfterShootCounter_--;
}

void Tank::resetIsWaitingAfterShoot() {
    if (isWaitingAfterShoot_ && waitAfterShootCounter_ == 0)
        isWaitingAfterShoot_ = false;
}

void Tank::doNothing() {
    resetIsRightAfterMoveBack();
}

char Tank::getSymbol() const {
    return (playerId_ < 1 || playerId_ > 2) ? '?' : static_cast<char>('0' + playerId_);
}

int Tank::getPlayerId() const { return playerId_; }
int Tank::getId() const { return id_; }
int Tank::getShellsLeft() const { return shellsLeft_; }
bool Tank::getIsWaitingToMoveBack() const { return isWaitingToMoveBack_; }
bool Tank::getIsWaitingAfterShoot() const { return isWaitingAfterShoot_; }
bool Tank::getIsRightAfterMoveBack() const { return isRightAfterMoveBack_; }
ActionRequest Tank::getNextAction() const { return nextAction_; }
void Tank::setNextAction(ActionRequest action) { nextAction_ = action; }
