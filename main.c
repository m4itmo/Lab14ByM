#include <stdio.h>
#include "LifeGeneration.h"

int main(int argc, char* argv[]) {
    BitmapHeader bmpHeader;
    GameConfig config = {1, 1, NULL};
    FILE* inputFile = NULL;

    parseCommandLineArgs(argc, argv, &config, &inputFile);

    readBitmapHeader(inputFile, &bmpHeader);
    int** gameMap = createAndInitImage(bmpHeader, inputFile);

    for (int iteration = 1; iteration <= config.maxIterations; iteration++) {
        updateGameState(iteration, gameMap, bmpHeader, &config);
        simulateGame(gameMap, bmpHeader.height, bmpHeader.width);
    }

    releaseImageMemory(gameMap, bmpHeader.height);
    fclose(inputFile);

    return 0;
}
