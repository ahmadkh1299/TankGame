# 🧠 TankGame

This project simulates a turn-based tank battle between two AI-controlled players on a grid-based board. Each player controls multiple tanks with distinct strategies and must navigate, dodge hazards, and defeat opponents while conforming to the Assignment 2 C++ interface.

---

## 🌟 Objective

* Build a simulation engine for a tank battle game
* Implement **two distinct AI algorithms** for Player 1 and Player 2
* Adhere to provided interfaces: `TankAlgorithm`, `BattleInfo`, `Player`, and `SatelliteView`
* Input is parsed from an A2-style text file; output is written in CSV format

---

## 🧠 Strategy Design

### 🔵 Player 1: ZoneControlAlgo (Dynamic Zone Defense)

Each tank is assigned a horizontal zone of the board. As tanks die, the remaining tanks redistribute the zones evenly.

**Behavior:**

* Stay within assigned zone
* Shoot only when enemy enters zone
* Prefer standing behind or near walls (cover)
* Dodge shells using lateral movement or backward retreat
* Reassign zones dynamically based on alive tanks

**Zone Logic:**

* If width = 15 and 3 tanks: zones are 0–4, 5–9, 10–14
* Each tank owns a zone and patrols within it
* Zone center is used to guide movement when idle

---

### 🔴 Player 2: HunterAlgo (Aggressive Search & Destroy)

Each tank aggressively searches for and engages the closest visible enemy using BFS.

**Behavior:**

* Use BFS to path toward closest enemy tank
* Shoot if enemy is in front and in line of fire
* If blocked, rotate or shoot wall to clear path
* Avoid mines if possible
* Continue pursuit of last known location if enemy escapes

---

## 🛡️ Architecture Overview

### 📁 Directory Structure

```
TankGame/
├── CMakeLists.txt
├── input/                      # Game input files (A2 format)
├── output/                     # Output log files
├── include/
│   ├── Board.h
│   ├── Direction.h
│   ├── GameObject.h
│   ├── InputParser.h
│   ├── Mine.h
│   ├── MovingGameObject.h
│   ├── Position.h
│   ├── Shell.h
│   ├── Tank.h
│   ├── Wall.h
│   ├── ZoneControlAlgo.h       # AI for Player 1
│   ├── HunterAlgo.h            # AI for Player 2 (upcoming)
│   ├── AlgorithmType.h         # Enum to select AI per player
│   └── common/                 # Assignment-provided headers
│       ├── ActionRequest.h
│       ├── BattleInfo.h
│       ├── Player.h
│       ├── PlayerFactory.h
│       ├── SatelliteView.h
│       ├── TankAlgorithm.h
│       └── TankAlgorithmFactory.h
├── src/
│   ├── Board.cpp
│   ├── InputParser.cpp
│   ├── Shell.cpp
│   ├── Tank.cpp
│   ├── ZoneControlAlgo.cpp     # AI logic for Player 1
│   ├── HunterAlgo.cpp          # AI logic for Player 2 (upcoming)
│   └── main.cpp
```

---

### 🧠 AI Logic Interface

Each algorithm implements:

```cpp
ActionRequest decideNextAction(
    const Tank& self,
    const Board& board,
    const std::vector<std::unique_ptr<Tank>>& enemyTanks,
    const std::vector<std::unique_ptr<Shell>>& shells,
    const std::vector<std::unique_ptr<Mine>>& mines);
```

---

### 📘 Input Format

Each input file looks like:

```
MaxSteps = 5000
NumShells = 16
Rows = 10
Cols = 15
###############
#1     @     2#
#   ##     ## #
###############
```

* `1` = Player 1 tank
* `2` = Player 2 tank
* `@` = Mine
* `#` = Wall

---

### 📄 Output Format (CSV-style log)

One line per round, comma-separated actions for each tank:

```
MoveForward, RotateRight90, Shoot
Shoot (killed), DoNothing, MoveForward
Player 2 won with 1 tanks still alive
```

---

## ✅ Current Progress

| Component               | Status    | Notes                            |
| ----------------------- | --------- | -------------------------------- |
| Tank, Shell, Wall, Mine | ✅ Done    | Core game objects                |
| Board                   | ✅ Done    | Grid storage & access            |
| InputParser             | ✅ Done    | Reads board + metadata           |
| ZoneControlAlgo         | ⚫ Partial | Player 1 AI logic started        |
| HunterAlgo              | ⬜ Not Yet | BFS logic to implement           |
| MyTankAlgorithm         | ⬜ Not Yet | Wraps either AI for each tank    |
| MyTankAlgorithmFactory  | ⬜ Not Yet | Chooses AI based on player index |
| GameManager             | ⬜ Not Yet | Simulation loop                  |
| OutputWriter            | ⬜ Not Yet | Output log generation            |

---

## 🛋️ Next Steps

1. 🔧 Finish `ZoneControlAlgo.cpp` (add cover-seeking + dodging)
2. 🌟 Implement `HunterAlgo.cpp`
3. 🛠️ Create `MyTankAlgorithm` to wrap both AI types
4. 📏 Build `MyTankAlgorithmFactory`
5. 🌄 Implement `GameManager` to simulate turns
6. 📊 Create `OutputWriter` to log rounds
7. 🔢 Test everything using provided input files

---

## 📌 Notes

* Zones for Player 1 are recalculated dynamically based on active tanks
* Mines are static hazards — AI must avoid but not interact
* Shells move in a straight line each round; tanks should anticipate them
* Tanks have cooldown rules for movement and shooting per assignment rules

---

## 👤 Author

**Ahmad Khalaily**
B.Sc. Computer Science
Tel Aviv University, 2025B
Assignment 2 – Advanced Topics in Programming
