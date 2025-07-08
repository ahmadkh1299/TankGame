#include "GameManager.h"
#include "MyPlayerFactory.h"
#include "MyTankAlgorithmFactory.h"
#include <iostream>


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: TankGame <input_file>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];

    GameManager game(
            std::make_unique<MyPlayerFactory>(),
            std::make_unique<MyTankAlgorithmFactory>()
    );

    game.readBoard(inputFile);
    game.run();
    return 0;
}
