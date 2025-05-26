//
// Created by Radiant Forest on 21/04/2025.
//

#ifndef MOVINGGAMEOBJECT_H
#define MOVINGGAMEOBJECT_H

#include "GameObject.h"
#include "Position.h"
#include "Direction.h"

class MovingGameObject : public GameObject {
public:
    MovingGameObject(Position pos, Direction dir)
        : GameObject(pos), dir_(dir) {}
    Direction getDirection() const { return dir_; }
    void setDirection(Direction dir) { dir_ = dir;  }
    virtual bool moveForward () = 0;

protected:
    Direction dir_;
};

#endif //MOVINGGAMEOBJECT_H
