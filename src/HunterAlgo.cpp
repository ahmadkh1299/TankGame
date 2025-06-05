#include "../include/HunterAlgo.h"
#include <cmath>
#include <climits>
#include <cstdlib>
#include "Direction.h"
#include "Position.h"

using namespace std;

HunterAlgo::HunterAlgo(int tankId) : tankId_(tankId) {}

void HunterAlgo::updateBattleInfo(BattleInfo& info) {
    currentInfo_ = info;
}

ActionRequest HunterAlgo::getAction() {
    // Step 1: Get battle info if needed
    if (currentInfo_.needsUpdate) {
        return ActionRequest::GetBattleInfo;
    }

    // Step 2: Find positions
    Position myPos;
    std::vector<Position> enemyPositions;
    Direction currentDir = Direction::Up; // Add explicit Direction usage
    
    // Scan satellite view to find tanks
    for (int y = 0; y < currentInfo_.height; y++) {
        for (int x = 0; x < currentInfo_.width; x++) {
            char cell = currentInfo_.satelliteView[y][x];
            if (cell == '%') {
                myPos = Position(x, y);
            }
            else if (cell == '1' || cell == '2') {
                if ((tankId_ == 1 && cell == '2') || 
                    (tankId_ == 2 && cell == '1')) {
                    enemyPositions.push_back(Position(x, y));
                }
            }
        }
    }

    // Step 3: Find closest enemy
    Position targetPos;
    int minDist = INT_MAX;
    for (const auto& enemyPos : enemyPositions) {
        int dist = abs(myPos.getX() - enemyPos.getX()) + 
                  abs(myPos.getY() - enemyPos.getY());
        if (dist < minDist) {
            minDist = dist;
            targetPos = enemyPos;
        }
    }

    if (enemyPositions.empty()) {
        return ActionRequest::MoveForward; // No enemies visible
    }

    // Step 4: Decide action
    // Check if aligned horizontally or vertically
    bool horizontallyAligned = (myPos.getY() == targetPos.getY());
    bool verticallyAligned = (myPos.getX() == targetPos.getX());

    if (horizontallyAligned) {
        if (targetPos.getX() > myPos.getX()) {
            return (currentInfo_.myDirection == Direction::Right) ? 
                   ActionRequest::Shoot : ActionRequest::RotateRight90;
        } else {
            return (currentInfo_.myDirection == Direction::Left) ? 
                   ActionRequest::Shoot : ActionRequest::RotateLeft90;
        }
    }
    
    if (verticallyAligned) {
        if (targetPos.getY() > myPos.getY()) {
            return (currentInfo_.myDirection == Direction::Down) ? 
                   ActionRequest::Shoot : ActionRequest::RotateRight90;
        } else {
            return (currentInfo_.myDirection == Direction::Up) ? 
                   ActionRequest::Shoot : ActionRequest::RotateRight90;
        }
    }

    // Not aligned - move closer using greedy approach
    int dx = targetPos.getX() - myPos.getX();
    int dy = targetPos.getY() - myPos.getY();

    if ((dx < 0 ? -dx : dx) > (dy < 0 ? -dy : dy)) {
        // Prioritize horizontal movement
        if (dx > 0) {
            return (currentInfo_.myDirection == Direction::Right) ? 
                   ActionRequest::MoveForward : ActionRequest::RotateRight90;
        } else {
            return (currentInfo_.myDirection == Direction::Left) ? 
                   ActionRequest::MoveForward : ActionRequest::RotateLeft90;
        }
    } else {
        // Prioritize vertical movement
        if (dy > 0) {
            return (currentInfo_.myDirection == Direction::Down) ? 
                   ActionRequest::MoveForward : ActionRequest::RotateRight90;
        } else {
            return (currentInfo_.myDirection == Direction::Up) ? 
                   ActionRequest::MoveForward : ActionRequest::RotateRight90;
        }
    }
}
