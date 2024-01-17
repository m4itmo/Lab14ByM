#ifndef LIFEGENERATOR_H
#define LIFEGENERATOR_H

#include <stdio.h>

typedef struct {
    unsigned char header[54];
    int width;
    int height;
    int size;
} BitmapHeader;

typedef struct {
    int maxIterations;
    int saveFrequency;
    char* outputDir;
} GameConfig;

void simulateGame(int** map, int height, int width);
void readBitmapHeader(FILE* file, BitmapHeader* bmp);
int** createAndInitImage(BitmapHeader bmp, FILE* file);
void updateGameState(int iteration, int** image, BitmapHeader bmp, GameConfig* config);
void releaseImageMemory(int** image, int height);
void parseCommandLineArgs(int argc, char* argv[], GameConfig* config, FILE** inputFile);

#endif
