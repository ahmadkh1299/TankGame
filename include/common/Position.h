//
// Created by Radiant Forest on 21/04/2025.
//

#ifndef POSITION_H
#define POSITION_H

class Position {
public:
    // Default constructor
    Position() : x_(0), y_(0) {}

    // Param constructor (optional)
    Position(int x, int y) : x_(x), y_(y) {}

    // Getters
    int getX() const { return x_; }
    int getY() const { return y_; }

    // Setters
    void setX(int x) { x_ = x; }
    void setY(int y) { y_ = y; }

    // Set both
    void set(int x, int y) { x_ = x; y_ = y; }

    // Wrap around board dimensions
    void wrap(int boardWidth, int boardHeight) {
        if (x_ < 0) x_ += boardWidth;
        if (y_ < 0) y_ += boardHeight;
        if (x_ >= boardWidth) x_ %= boardWidth;
        if (y_ >= boardHeight) y_ %= boardHeight;
    }

    //Moving
    void moveUp() {y_ -= 1;}
    void moveDown() {y_ += 1;}
    void moveLeft() {x_ -= 1;}
    void moveRight() {x_ += 1;}

    // Equality check
    bool operator==(const Position& other) const {
        return x_ == other.x_ && y_ == other.y_;
    }

private:
    int x_;
    int y_;
};


#endif //POSITION_H
