#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

typedef struct {
    unsigned char header[54];
    int width;
    int height;
    int size;
} BMPHeader;

typedef struct {
    int maxIter;
    int dumpFreq;
    char *output;
} GameConfig;

void runIteration(int **map, int height, int width) {
    int liveCells, noChangeCells = 0;
    int tempMap[height][width];
    int x, y;

    for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
            tempMap[y][x] = map[y][x];
        }
    }

    for (x = 1; x < width - 1; ++x) {
        for (y = 1; y < height - 1; ++y) {
            liveCells = map[y - 1][x - 1] + map[y - 1][x] + map[y - 1][x + 1] + map[y][x - 1] + map[y][x + 1] +
                        map[y + 1][x - 1] + map[y + 1][x] + map[y + 1][x + 1];

            if (map[y][x] == 1 && (liveCells < 2 || liveCells > 3))
                tempMap[y][x] = 0;
            else if (map[y][x] != 1 && liveCells == 3)
                tempMap[y][x] = 1;
        }
    }

    for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
            if (map[y][x] == tempMap[y][x])
                ++noChangeCells;
            map[y][x] = tempMap[y][x];
        }
    }

    if (noChangeCells == height * width)
        exit(-4);
}

void getBMPHeaderData(FILE *file, BMPHeader *bmp) {
    fread(bmp->header, 1, 54, file);
    bmp->width = bmp->header[21] * 16777216 + bmp->header[20] * 65536 + bmp->header[19] * 256 + bmp->header[18];
    bmp->height = bmp->header[25] * 16777216 + bmp->header[24] * 65536 + bmp->header[23] * 256 + bmp->header[22];
    bmp->size = bmp->header[5] * 16777216 + bmp->header[4] * 65536 + bmp->header[3] * 256 + bmp->header[2];
}

int **initImage(BMPHeader bmp, FILE *file) {
    int **image = (int **) malloc(bmp.height * sizeof(int *));
    for (int i = 0; i < bmp.height; ++i)
        image[i] = (int *) malloc(bmp.width * sizeof(int));

    unsigned char pixelData[bmp.size - 54];
    fread(pixelData, 1, bmp.size, file);

    int k = 0;
    for (int i = bmp.height - 1; i >= 0; --i) {
        k += (bmp.width % 4);
        for (int j = 0; j < bmp.width; ++j) {
            image[i][j] = (pixelData[k] == 255) ? 0 : 1;
            k += 3;
        }
    }

    return image;
}

void saveNewSate(int iteration, int **image, BMPHeader bmp, GameConfig *config) {
    if (iteration % config->dumpFreq == 0) {
        char filename[20];
        char fullPath[100];
        sprintf(filename, "%d.bmp", iteration);
        strcpy(fullPath, config->output);
        strcat(fullPath, "\\");
        strcat(fullPath, filename);

        FILE *outputFile = fopen(fullPath, "wb");
        fwrite(bmp.header, 1, 54, outputFile);

        unsigned char pixelData[bmp.size - 54];
        for (int i = bmp.height - 1, m = 0; i >= 0; --i) {
            for (int j = 0; j < bmp.width; ++j)
                for (int k = 0; k < 3; ++k)
                    pixelData[m++] = (image[i][j] == 1) ? 0 : 255;
            while (m % 4 != 0)
                pixelData[m++] = 0;
        }

        fwrite(pixelData, 1, bmp.size, outputFile);
        fclose(outputFile);
    }
}

void freeImageMemory(int **image, int height) {
    for (int i = 0; i < height; ++i)
        free(image[i]);
    free(image);
}

void parseCommandLineArgs(int argc, char *argv[], GameConfig *config, FILE **inputFile) {
    for (int i = 1; i < argc; ++i) {
        if (!strcmp("--input", argv[i]) && i + 1 < argc) {
            *inputFile = fopen(argv[i + 1], "rb");
            if (!*inputFile)
                perror("Error opening input file");
        } else if (!strcmp("--output", argv[i]) && i + 1 < argc) {
            config->output = argv[i + 1];
            mkdir(config->output);
        } else if (!strcmp("--max_iter", argv[i]) && i + 1 < argc) {
            config->maxIter = strtol(argv[i + 1], NULL, 10);
        } else if (!strcmp("--dump_freq", argv[i]) && i + 1 < argc) {
            config->dumpFreq = strtol(argv[i + 1], NULL, 10);
        }
    }
}

int main(int argc, char *argv[]) {
    BMPHeader bmpHeader;
    GameConfig config = {1, 1, NULL};
    FILE *inputFile = NULL;

    parseCommandLineArgs(argc, argv, &config, &inputFile);

    getBMPHeaderData(inputFile, &bmpHeader);
    int **gameMap = initImage(bmpHeader, inputFile);

    for (int iteration = 1; iteration <= config.maxIter; ++iteration) {
        saveNewSate(iteration, gameMap, bmpHeader, &config);
        runIteration(gameMap, bmpHeader.height, bmpHeader.width);
    }

    freeImageMemory(gameMap, bmpHeader.height);
    fclose(inputFile);

    return 0;
}
