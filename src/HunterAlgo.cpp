#include "../include/HunterAlgo.h"
#include <climits>
#include <queue>
#include <algorithm>
#include <cmath>

using namespace std;

HunterAlgo::HunterAlgo(int tankId)
        : tankId_(tankId), currentDirection_(Direction::Up),
          turnsSinceLastUpdate_(0) {}

void HunterAlgo::updateBattleInfo(BattleInfo& info) {
    auto* myInfoPtr = dynamic_cast<MyBattleInfo*>(&info);
    if (!myInfoPtr) {
        currentInfo_.reset();
        return;
    }
    currentInfo_ = *myInfoPtr;
    turnsSinceLastUpdate_ = 0;
}

ActionRequest HunterAlgo::getAction() {
    turnsSinceLastUpdate_++;

    if (turnsSinceLastUpdate_ > UPDATE_INTERVAL || !currentInfo_.has_value()) {
        return ActionRequest::GetBattleInfo;
    }

    const MyBattleInfo& info = *currentInfo_;

    // Build a simple grid from the battle info
    vector<vector<char>> grid(info.getRows(), vector<char>(info.getCols(), ' '));
    Position myPos;
    vector<Position> enemies;
    char enemySymbol = (tankId_ == 1) ? '2' : '1';

    for (size_t y = 0; y < info.getRows(); ++y) {
        for (size_t x = 0; x < info.getCols(); ++x) {
            char obj = info.getObjectAt(x, y);
            grid[y][x] = obj;
            if (obj == '%') {
                myPos = Position((int)x, (int)y);
            } else if (obj == enemySymbol) {
                enemies.emplace_back((int)x, (int)y);
            }
        }
    }

    if (enemies.empty()) {
        return ActionRequest::DoNothing;
    }

    // Find the closest enemy by Manhattan distance
    Position target = enemies.front();
    int bestDist = INT_MAX;
    for (const auto& e : enemies) {
        int dist = abs(e.getX() - myPos.getX()) + abs(e.getY() - myPos.getY());
        if (dist < bestDist) {
            bestDist = dist;
            target = e;
        }
    }

    currentPath = runBFS(myPos, target, grid);

    if (currentPath.size() < 2) {
        return ActionRequest::DoNothing;
    }

    Position nextStep = currentPath[1];
    Direction needed = getDirectionTo(myPos, nextStep);

    if (needed != currentDirection_) {
        ActionRequest act = rotateToward(currentDirection_, needed);
        // Update our internal direction based on the rotation we request
        if (act == ActionRequest::RotateRight90) {
            currentDirection_ = static_cast<Direction>((static_cast<int>(currentDirection_) + 1) % 4);
        } else if (act == ActionRequest::RotateLeft90) {
            currentDirection_ = static_cast<Direction>((static_cast<int>(currentDirection_) + 3) % 4);
        }
        return act;
    }

    return ActionRequest::MoveForward;
}

vector<Position> HunterAlgo::runBFS(const Position& start, const Position& goal,
                                    const vector<vector<char>>& grid) {
    int rows = grid.size();
    int cols = grid[0].size();
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<vector<Position>> parent(rows, vector<Position>(cols, Position(-1, -1)));
    queue<Position> q;

    q.push(start);
    visited[start.getY()][start.getX()] = true;

    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    while (!q.empty()) {
        Position cur = q.front();
        q.pop();

        if (cur == goal) {
            break;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = cur.getX() + dx[i];
            int ny = cur.getY() + dy[i];

            if (nx < 0 || ny < 0 || nx >= cols || ny >= rows) continue;
            if (visited[ny][nx]) continue;

            char cell = grid[ny][nx];
            if (cell == '#') continue; // Wall

            visited[ny][nx] = true;
            parent[ny][nx] = cur;
            q.emplace(nx, ny);
        }
    }

    if (!visited[goal.getY()][goal.getX()]) {
        return {};
    }

    vector<Position> path;
    Position step = goal;
    while (!(step == start)) {
        path.push_back(step);
        step = parent[step.getY()][step.getX()];
    }
    path.push_back(start);
    reverse(path.begin(), path.end());
    return path;
}

Direction HunterAlgo::getDirectionTo(const Position& from, const Position& to) const {
    if (to.getX() > from.getX()) return Direction::Right;
    if (to.getX() < from.getX()) return Direction::Left;
    if (to.getY() > from.getY()) return Direction::Down;
    if (to.getY() < from.getY()) return Direction::Up;
    return Direction::Up;
}

ActionRequest HunterAlgo::rotateToward(Direction current, Direction target) const {
    int c = static_cast<int>(current);
    int t = static_cast<int>(target);
    int diff = (t - c + 4) % 4;
    if (diff == 0) return ActionRequest::MoveForward;
    if (diff == 1) return ActionRequest::RotateRight90;
    if (diff == 3) return ActionRequest::RotateLeft90;
    return ActionRequest::RotateRight90;
}

