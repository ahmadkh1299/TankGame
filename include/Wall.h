// Wall.h
#ifndef WALL_H
#define WALL_H

#include "GameObject.h"

class Wall : public GameObject {
public:
    explicit Wall(Position pos) : GameObject(pos) {}
    static constexpr int TIMES_TO_HIT_BEFORE_GONE = 2;

    char getSymbol() const override { return '#'; }

    int getLifeLeft() const { return lifeLeft_; }
    void decreaseLifeLeft() { lifeLeft_--; }

    bool isDestroyed() const { return lifeLeft_ <= 0; }


private:
    int lifeLeft_ = TIMES_TO_HIT_BEFORE_GONE;
};

#endif // WALL_H
