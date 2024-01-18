#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

typedef struct {
    unsigned char header[54];
    int width;
    int height;
    int size;
} BitmapHeader;

typedef struct {
    int maxIterations;
    int saveFrequency;
    char *outputDir;
} GameConfig;

void simulateGame(int **map, int height, int width) {
    int liveCells, noChangeCells = 0;
    int tempMap[height][width];
    int x, y;

    for (x = 0; x < width; x++) {
        for (y = 0; y < height; y++) {
            tempMap[y][x] = map[y][x];
        }
    }

    for (x = 1; x < width - 1; x++) {
        for (y = 1; y < height - 1; y++) {
            liveCells = map[y - 1][x - 1] + map[y - 1][x] + map[y - 1][x + 1] + map[y][x - 1] + map[y][x + 1] +
                        map[y + 1][x - 1] + map[y + 1][x] + map[y + 1][x + 1];

            if (map[y][x] == 1) {
                if (liveCells < 2 || liveCells > 3) {
                    tempMap[y][x] = 0;
                }
            } else {
                if (liveCells == 3) {
                    tempMap[y][x] = 1;
                }
            }
        }
    }

    for (x = 0; x < width; x++) {
        for (y = 0; y < height; y++) {
            if (map[y][x] == tempMap[y][x]) {
                noChangeCells++;
            }
            map[y][x] = tempMap[y][x];
        }
    }

    if (noChangeCells == height * width) {
        exit(-4);
    }
}

void readBitmapHeader(FILE *file, BitmapHeader *bmp) {
    fread(bmp->header, 1, 54, file);
    bmp->width =
            bmp->header[21] * 256 * 256 * 256 + bmp->header[20] * 256 * 256 + bmp->header[19] * 256 + bmp->header[18];
    bmp->height =
            bmp->header[25] * 256 * 256 * 256 + bmp->header[24] * 256 * 256 + bmp->header[23] * 256 + bmp->header[22];
    bmp->size = bmp->header[5] * 256 * 256 * 256 + bmp->header[4] * 256 * 256 + bmp->header[3] * 256 + bmp->header[2];
}

int **createAndInitImage(BitmapHeader bmp, FILE *file) {
    int **image = (int **) malloc(bmp.height * sizeof(int *));
    for (int i = 0; i < bmp.height; i++) {
        image[i] = (int *) malloc(bmp.width * sizeof(int));
    }

    unsigned char pixelData[bmp.size - 54];
    fread(pixelData, 1, bmp.size, file);

    int k = 0;
    for (int i = bmp.height - 1; i >= 0; i--) {
        k += (bmp.width % 4);
        for (int j = 0; j < bmp.width; j++) {
            image[i][j] = (pixelData[k] == 255) ? 0 : 1;
            k += 3;
        }
    }

    return image;
}

void updateGameState(int iteration, int **image, BitmapHeader bmp, GameConfig *config) {
    if (iteration % config->saveFrequency == 0) {
        char filename[20];
        sprintf(filename, "%d.bmp", iteration);
        char fullPath[100];
        strcpy(fullPath, config->outputDir);
        strcat(fullPath, "\\");
        strcat(fullPath, filename);

        FILE *outputFile = fopen(fullPath, "wb");
        fwrite(bmp.header, 1, 54, outputFile);

        unsigned char pixelData[bmp.size - 54];
        int m = 0;
        for (int i = bmp.height - 1; i >= 0; i--) {
            for (int j = 0; j < bmp.width; j++) {
                for (int k = 0; k < 3; k++) {
                    pixelData[m++] = (image[i][j] == 1) ? 0 : 255;
                }
            }
            while (m % 4 != 0) {
                pixelData[m++] = 0;
            }
        }

        fwrite(pixelData, 1, bmp.size, outputFile);
        fclose(outputFile);
    }
}

void releaseImageMemory(int **image, int height) {
    for (int i = 0; i < height; i++) {
        free(image[i]);
    }
    free(image);
}

void parseCommandLineArgs(int argc, char *argv[], GameConfig *config, FILE **inputFile) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp("--input", argv[i]) && i + 1 < argc) {
            *inputFile = fopen(argv[i + 1], "rb");
            if (!*inputFile) {
                perror("Error opening input file");
            }
        } else if (!strcmp("--output", argv[i]) && i + 1 < argc) {
            config->outputDir = argv[i + 1];
            mkdir(config->outputDir);
        } else if (!strcmp("--max_iter", argv[i]) && i + 1 < argc) {
            config->maxIterations = strtol(argv[i + 1], NULL, 10);
        } else if (!strcmp("--dump_freq", argv[i]) && i + 1 < argc) {
            config->saveFrequency = strtol(argv[i + 1], NULL, 10);
        }
    }
}

int main(int argc, char *argv[]) {
    BitmapHeader bmpHeader;
    GameConfig config = {1, 1, NULL};
    FILE *inputFile = NULL;

    parseCommandLineArgs(argc, argv, &config, &inputFile);

    readBitmapHeader(inputFile, &bmpHeader);
    int **gameMap = createAndInitImage(bmpHeader, inputFile);

    for (int iteration = 1; iteration <= config.maxIterations; iteration++) {
        updateGameState(iteration, gameMap, bmpHeader, &config);
        simulateGame(gameMap, bmpHeader.height, bmpHeader.width);
    }

    releaseImageMemory(gameMap, bmpHeader.height);
    fclose(inputFile);

    return 0;
}
