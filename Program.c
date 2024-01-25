/*
PROGRAM INFO
Author: Jacob Garcia
Version: 1.01
*/
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

    return 0;
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
        //Randomly scale the length of new terrain
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

    //Set up roads
    //West to East
    int westEnt = rand() % (ROWS - 2) + 1;
    int eastEnt = rand() % (ROWS - 2) + 1;
    map[westEnt][0] = ROAD;
    map[eastEnt][COLUMNS - 1] = ROAD;

    //Pick "intersect" column then make first two paths
    int interCol = rand() % (COLUMNS - 2) + 1;
    for (int i = 1; i <= interCol; i++) {
        map[westEnt][i] = ROAD;
    }
    for (int i = COLUMNS - 2; i >= interCol; i--) {
        map[eastEnt][i] = ROAD;
    }

    //Complete path
    int westEntCpy = westEnt;
    while (westEntCpy - eastEnt != 0) {
        westEntCpy += -((westEntCpy - eastEnt) / abs(westEntCpy - eastEnt));
        map[westEntCpy][interCol] = ROAD;
    }

    //North to South
    int northEnt = rand() % (COLUMNS - 2) + 1;
    int southEnt = rand() % (COLUMNS - 2) + 1;
    map[0][northEnt] = ROAD;
    map[ROWS - 1][southEnt] = ROAD;

    //Pick "intersect" row then make first two paths
    int interRow = rand() % (ROWS - 2) + 1;
    for (int i = 1; i <= interRow; i++) {
        map[i][northEnt] = ROAD;
    }
    for (int i = ROWS - 2; i >= interRow; i--) {
        map[i][southEnt] = ROAD;
    }

    //Complete path
    int northEntCpy = northEnt;
    while (northEntCpy - southEnt != 0) {
        northEntCpy += -((northEntCpy - southEnt) / abs(northEntCpy - southEnt));
        map[interRow][northEntCpy] = ROAD;
    }

    //Place PokeMart and PokeCenter 
    int buidlingsPlaced = 0;
    char* building = CNTR;
    while (buidlingsPlaced != 2) {
        /*TODO Okay so here is my idea, for like the terrain, randomly choose spots on the map looking for a path
        This sound incredibly inefficient but just choose randomly until a good spot is found that satisfies all conditions:
        1. Doesn't go out of bounds
        2. There is no road in placement. 
        3. Placed next to road. 
        4. Not placed on other building 
        */
        int spotFound = 0;
        while (spotFound == 0) {
            int spotCol = rand() % (COLUMNS - 3) + 1;
            int spotRow = rand() % (ROWS - 3) + 3;

            //Check conditions
            if (strcmp(map[spotRow][spotCol], ROAD) != 0) continue;
            if (strcmp(map[spotRow - 1][spotCol], CNTR) == 0 ||
                strcmp(map[spotRow - 2][spotCol], CNTR) == 0 || 
                strcmp(map[spotRow - 1][spotCol + 1], CNTR) == 0 ||
                strcmp(map[spotRow - 2][spotCol + 1], CNTR) == 0) continue;
            if (strcmp(map[spotRow - 1][spotCol], ROAD) != 0 &&
                strcmp(map[spotRow - 2][spotCol], ROAD) != 0 && 
                strcmp(map[spotRow - 1][spotCol + 1], ROAD) != 0 &&
                strcmp(map[spotRow - 2][spotCol + 1], ROAD) != 0) {
                    map[spotRow - 1][spotCol] = building;
                    map[spotRow - 2][spotCol] = building;
                    map[spotRow - 1][spotCol + 1] = building;
                    map[spotRow - 2][spotCol + 1] = building;
                    spotFound = 1;
                }
        }

        building = PKMART;
        buidlingsPlaced++;
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
    int ter = rand() % 4 + 1;

    if (ter == 1) {
        return TREE;
    } else if (ter == 2) {
        return LNGR;
    } else if (ter == 3) {
        return CLRNG;
    } else if (ter == 4) {
        return WATER;
    }

    return NULL;
}