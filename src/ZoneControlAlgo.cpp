#include "ZoneControlAlgo.h"
#include "Wall.h"
#include <cmath>
#include <climits>

ZoneControlAlgo::ZoneControlAlgo(int tankId)
        : tankId_(tankId), zoneStart_(0), zoneEnd_(0) {}

void ZoneControlAlgo::updateZoneRange(int startX, int endX) {
    zoneStart_ = startX;
    zoneEnd_ = endX;
}

Direction getDirectionTo(const Position& from, const Position& to) {
    if (to.getX() > from.getX()) return Direction::Right;
    if (to.getX() < from.getX()) return Direction::Left;
    if (to.getY() > from.getY()) return Direction::Down;
    if (to.getY() < from.getY()) return Direction::Up;
    return Direction::Up;
}

Position findWallCoverInZone(const Tank& self, const Board& board, int zoneStart, int zoneEnd) {
    Position myPos = self.getPosition();
    int minDistance = INT_MAX;
    Position bestCover(-1, -1);

    for (int y = 0; y < board.getHeight(); ++y) {
        for (int x = zoneStart; x <= zoneEnd; ++x) {
            Position wallPos(x, y);
            for (GameObject* obj : board.getObjectsAt(wallPos)) {
                if (!dynamic_cast<Wall*>(obj)) continue;

                std::vector<Position> adjacent = {
                        Position(x + 1, y), Position(x - 1, y),
                        Position(x, y + 1), Position(x, y - 1)
                };

                for (const Position& adj : adjacent) {
                    if (adj.getX() < zoneStart || adj.getX() > zoneEnd) continue;
                    const auto& objects = board.getObjectsAt(adj);
                    if (!objects.empty()) continue;

                    int dist = abs(myPos.getX() - adj.getX()) + abs(myPos.getY() - adj.getY());
                    if (dist < minDistance) {
                        minDistance = dist;
                        bestCover = adj;
                    }
                }
            }
        }
    }

    return bestCover;
}

ActionRequest ZoneControlAlgo::decideNextAction(
        const Tank& self,
        const Board& board,
        const std::vector<std::unique_ptr<Tank>>& enemyTanks,
        const std::vector<std::unique_ptr<Shell>>& shells,
        const std::vector<std::unique_ptr<Mine>>& mines)
{
    Position myPos = self.getPosition();
    Direction myDir = self.getDirection();

    // 1. Zone enforcement
    int zoneCenterX = (zoneStart_ + zoneEnd_) / 2;
    int dx = zoneCenterX - myPos.getX();

    if (std::abs(dx) > 0) {
        if (dx > 0 && myDir != Direction::Right) return ActionRequest::RotateRight90;
        if (dx < 0 && myDir != Direction::Left) return ActionRequest::RotateLeft90;
        return ActionRequest::MoveForward;
    }

    // 2. Shell avoidance
    auto isHorizontal = [](Direction dir) {
        return dir == Direction::Left || dir == Direction::Right;
    };
    auto isVertical = [](Direction dir) {
        return dir == Direction::Up || dir == Direction::Down;
    };
    auto isFacingEscape = [&](Direction tankDir, Direction shellDir) {
        if (isHorizontal(shellDir)) return isVertical(tankDir);
        if (isVertical(shellDir)) return isHorizontal(tankDir);
        return true;
    };

    for (const auto& shell : shells) {
        Position sPos = shell->getPosition();
        Direction sDir = shell->getDirection();

        if (sPos.getY() == myPos.getY()) {
            if (sDir == Direction::Right && sPos.getX() < myPos.getX()) {
                if ((myPos.getX() - sPos.getX()) / 2 <= 3) {
                    if (!isFacingEscape(myDir, sDir)) return ActionRequest::RotateRight90;
                    return ActionRequest::MoveForward;
                }
            }
            if (sDir == Direction::Left && sPos.getX() > myPos.getX()) {
                if ((sPos.getX() - myPos.getX()) / 2 <= 3) {
                    if (!isFacingEscape(myDir, sDir)) return ActionRequest::RotateRight90;
                    return ActionRequest::MoveForward;
                }
            }
        }

        if (sPos.getX() == myPos.getX()) {
            if (sDir == Direction::Down && sPos.getY() < myPos.getY()) {
                if ((myPos.getY() - sPos.getY()) / 2 <= 3) {
                    if (!isFacingEscape(myDir, sDir)) return ActionRequest::RotateRight90;
                    return ActionRequest::MoveForward;
                }
            }
            if (sDir == Direction::Up && sPos.getY() > myPos.getY()) {
                if ((sPos.getY() - myPos.getY()) / 2 <= 3) {
                    if (!isFacingEscape(myDir, sDir)) return ActionRequest::RotateRight90;
                    return ActionRequest::MoveForward;
                }
            }
        }
    }

    // 3. Enemy check + line of sight (with optional wall clearing)
    for (const auto& enemy : enemyTanks) {
        Position ePos = enemy->getPosition();

        if (ePos.getX() >= zoneStart_ && ePos.getX() <= zoneEnd_) {
            if (ePos.getY() == myPos.getY()) {
                int minX = std::min(myPos.getX(), ePos.getX());
                int maxX = std::max(myPos.getX(), ePos.getX());
                bool wallInPath = false, clearLine = true;
                for (int x = minX + 1; x < maxX; ++x) {
                    for (auto obj : board.getObjectsAt({x, myPos.getY()})) {
                        if (dynamic_cast<Wall*>(obj)) { wallInPath = true; clearLine = false; break; }
                    }
                    if (!clearLine) break;
                }
                if (clearLine && ((ePos.getX() > myPos.getX() && myDir == Direction::Right) ||
                                  (ePos.getX() < myPos.getX() && myDir == Direction::Left)))
                    return ActionRequest::Shoot;
                if (wallInPath && self.getShellsLeft() > 0 && !self.getIsWaitingAfterShoot()) {
                    if ((ePos.getX() > myPos.getX() && myDir == Direction::Right) ||
                        (ePos.getX() < myPos.getX() && myDir == Direction::Left))
                        return ActionRequest::Shoot;
                }
            }

            if (ePos.getX() == myPos.getX()) {
                int minY = std::min(myPos.getY(), ePos.getY());
                int maxY = std::max(myPos.getY(), ePos.getY());
                bool wallInPath = false, clearLine = true;
                for (int y = minY + 1; y < maxY; ++y) {
                    for (auto obj : board.getObjectsAt({myPos.getX(), y})) {
                        if (dynamic_cast<Wall*>(obj)) { wallInPath = true; clearLine = false; break; }
                    }
                    if (!clearLine) break;
                }
                if (clearLine && ((ePos.getY() > myPos.getY() && myDir == Direction::Down) ||
                                  (ePos.getY() < myPos.getY() && myDir == Direction::Up)))
                    return ActionRequest::Shoot;
                if (wallInPath && self.getShellsLeft() > 0 && !self.getIsWaitingAfterShoot()) {
                    if ((ePos.getY() > myPos.getY() && myDir == Direction::Down) ||
                        (ePos.getY() < myPos.getY() && myDir == Direction::Up))
                        return ActionRequest::Shoot;
                }
            }
        }
    }

    // 4. Move toward cover if idle
    Position cover = findWallCoverInZone(self, board, zoneStart_, zoneEnd_);
    if (cover.getX() != -1) {
        Direction toCover = getDirectionTo(myPos, cover);
        if (myDir != toCover) return ActionRequest::RotateRight90;
        return ActionRequest::MoveForward;
    }

    return ActionRequest::DoNothing;
}
