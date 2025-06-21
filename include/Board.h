//
// Created by Radiant Forest on 22/04/2025.
//

#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <memory>
#include "Tank.h"
#include "Shell.h"
#include "Mine.h"
#include "Wall.h"
#include "Position.h"

class Board {
private:
    int width_, height_;
    std::vector<std::vector<std::vector<GameObject*>>> grid;

public:
    Board(int w, int h);
    ~Board();

    //Getters
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    void addGameObject(GameObject* obj, Position pos);
    const std::vector<GameObject*>& getObjectsAt(Position pos) const;
    void removeAllAt(Position pos);
    void removeObject(GameObject* objToRemove, Position pos);

    bool isWall(Position pos) const {
        // Returns true if the cell at (x,y) contains a wall
        return getObjectsAt(pos).front()->getSymbol() == '#';
    }

    //void cleanDestroyedWalls();
    //void clear();


};

#endif //BOARD_H
