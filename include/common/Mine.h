
#ifndef MINE_H
#define MINE_H

#include "GameObject.h"

class Mine : public GameObject {
public:
    explicit Mine(Position pos) : GameObject(pos) {}
    char getSymbol() const override { return '@'; }
};

#endif // MINE_H
