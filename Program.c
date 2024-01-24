#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//Prototypes
void GenerateMap();
void PrintMap();
char* FindTerrain();

//Global variables
#define ROWS 21
#define COLUMNS 80
char* map[ROWS][COLUMNS];

//Colors
#define BLACK   "\x1b[30m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define PURPLE  "\033[0;34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"
#define RESET   "\x1b[0m"
#define GREY    "\x1b[90m"

/*
Percent signs (%) represent
boulders and mountains. Carrots (ˆ) represent tree and forests. Hashes (#) are roads. Cs and Ms are Pokemon ´
Centers and Pokemarts (buildings), respectively. Colons (:) are long grass and periods (.) are clearings. 
*/
//Elements
char* MTN = GREY "%" RESET;
char* TREE = GREY "^" RESET;
char* ROAD = YELLOW "#" RESET;
char* LNGR = GREEN ":" RESET;
char* CLRNG = GREEN "." RESET;
char* WATER = CYAN "~" RESET;
char* CNTR = MAGENTA "C" RESET;
char* PKMART = MAGENTA "M" RESET;

int main(int argc, char *argv[]) {
    srand(time(NULL));
    GenerateMap();
    PrintMap();
}

void GenerateMap() {
    //Fill with blank space TEMP
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            map[i][j] = " ";
        }
    }
    //Make border 
    for (int i = 0; i < COLUMNS; i++) {
        map[0][i] = MTN;
    }
    for (int i = 1; i < ROWS - 1; i++) {
        map[i][0] = MTN;
        map[i][COLUMNS - 1] = MTN;
    }
    for (int i = 0; i < COLUMNS; i++) {
        map[ROWS - 1][i] = MTN;
    }

    //Fill in middle
    int xLength = 5;
    int yLength = 5;
    int blankSpace = (COLUMNS - 2) * (ROWS - 2);

    //Keep generating terrain until there is no blank space. 
    while (blankSpace != 0) {
        //Scale the length of new terrain
        int newXLength = xLength * (((double)rand() / RAND_MAX) + 1.0);
        int newYLength = yLength * (((double)rand() / RAND_MAX) + 1.0);
        
        //Find point to place. 
        int x = rand() % (ROWS - 2) + 1;
        int y = rand() % (COLUMNS - 2) + 1;

        //Place terrain
        char* terrain = FindTerrain();
        for (int i = x; i < x + newXLength; i++) {
            for (int j = y; j < y + newYLength; j++) {
                //Check if on border
                if ((i > 0) && (i < ROWS - 1) && (j > 0) && (j < COLUMNS - 1)) {
                    if (strcmp(map[i][j], " ") == 0) blankSpace--;
                    map[i][j] = terrain;
                }
            }
        }
    }
}

void PrintMap() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            printf("%s", map[i][j]);
        }
        printf("\n");
    }
}

char* FindTerrain() {
    int ter = rand() % 5 + 1;

    if (ter == 1) {
        return MTN;
    } else if (ter == 2) {
        return TREE;
    } else if (ter == 3) {
        return LNGR;
    } else if (ter == 4) {
        return CLRNG;
    } else if (ter == 5) {
        return WATER;
    }

    return NULL;
}