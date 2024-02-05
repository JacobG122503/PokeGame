/*
PROGRAM INFO
Author: Jacob Garcia
Version: 1.02
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//Constants
#define ROWS 21
#define COLUMNS 80
#define WORLDROWS 401
#define WORLDCOLUMNS 401

//Structs
struct map {
    char* map[ROWS][COLUMNS];
    int x;
    int y;
    int northEnt;
    int southEnt;
    int westEnt;
    int eastEnt;
};

//Prototypes
struct map GenerateMap(struct map *worldMap[WORLDROWS][WORLDCOLUMNS], int x, int y);
void PrintMap(struct map);
char* FindTerrain();

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

    struct map* worldMap[WORLDROWS][WORLDCOLUMNS]; //NOTE Maybe for fun add something that prints every single map

    for (int i = 0; i < WORLDROWS; i++) {
        for (int j = 0; j < WORLDCOLUMNS; j++) {
            worldMap[i][j] = NULL;
        }
    }

    int x = 200;
    int y = 200;
    struct map currentMap = GenerateMap(worldMap, x, y);

    //Start movement 
    char command = 'c';
    while (command != 'q') {
        system("clear");
        PrintMap(currentMap);

        printf("What would you like to do next? Type i to see available options.\n");
        scanf(" %c", &command);
        //Instructions
        if (command == 'i') {
            system("clear");
            printf("%sCOMMAND LIST%s\n", GREEN, RESET);
            printf("n: Move to the map immediately north of the current map and display it.\n"
                "s: Move to the map immediately south of the current map and display it.\n"
                "e: Move to the map immediately east of the current map and display it.\n"
                "w: Move to the map immediately west of the current map and display it.\n"
                "f x y: x and y are integers; Fly2 to map (x, y) and display it.\n"
                "q: Quit the game.\n");
            printf("\nType c to continue: ");
            while (command != 'c') scanf("%c", &command);
            continue;
        }
        if (command == 'n' || command == 'e' || command == 's' || command == 'w') {
            if (command == 'n' && !(x > 400 || y + 1 > 400 || x < 0 || y < 0)) y++;
            if (command == 'e' && !(x + 1 > 400 || y > 400 || x < 0 || y < 0)) x++;
            if (command == 's' && !(x > 400 || y > 400 || x < 0 || y - 1 < 0)) y--;
            if (command == 'w' && !(x > 400 || y > 400 || x - 1 < 0 || y < 0)) x--;

            currentMap = GenerateMap(worldMap, x, y);
            continue;
        }
        if (command == 'f') {
            int oldX = x;
            int oldY = y;
            //printf("Fly to (x y): ");
            scanf(" %d %d", &x, &y);
            x += 200;
            y += 200;
            if (x > 400 || y > 400 || x < 0 || y < 0) {
                x = oldX;
                y = oldY;
                continue;
            }
            currentMap = GenerateMap(worldMap, x, y);
        }
    }

    printf("\n");
    return 0;
}

struct map GenerateMap(struct map *worldMap[WORLDROWS][WORLDCOLUMNS], int x, int y) {
    //Check if map already exists
    if (worldMap[x][y] != NULL) return *worldMap[x][y];

    char* map[ROWS][COLUMNS];

    //Fill with blank space
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
    //Check if any maps exist around this new map, change gate to match other map. If not generate new ones. 
    int westEnt = rand() % (ROWS - 2) + 1;
    int eastEnt = rand() % (ROWS - 2) + 1;
    int northEnt = rand() % (COLUMNS - 2) + 1;
    int southEnt = rand() % (COLUMNS - 2) + 1;

    //Check North
    if (y < 400 && worldMap[x][y + 1] != NULL) {
        northEnt = worldMap[x][y + 1]->southEnt;
    }
    //Check South
    if (y > 0 && worldMap[x][y - 1] != NULL) {
        southEnt = worldMap[x][y - 1]->northEnt;
    }
    //Check West
    if (x > 0 && worldMap[x - 1][y] != NULL) {
        westEnt = worldMap[x - 1][y]->eastEnt;
    }
    //Check East
    if (x < 400 && worldMap[x + 1][y] != NULL) {
        eastEnt = worldMap[x + 1][y]->westEnt;
    }

    //Connect West to East
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

    //Connect North to South
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

    //If gate on border, close it
    if (x == 400) map[eastEnt][COLUMNS - 1] = MTN;
    if (x == 0) map[westEnt][0] = MTN;
    if (y == 400) map[0][northEnt] = MTN;
    if (y == 0) map[ROWS - 1][southEnt] = MTN;

    //Place PokeMart and PokeCenter 
    int buidlingsPlaced = 0;
    int buildingsToBePlaced = 2;
    char* building;
    int randBuilding = rand() % 2 + 1;
    if (randBuilding == 1) building = CNTR;
    if (randBuilding == 2) building = PKMART;

    if (!(x == 200 && y == 200)) {
        double equation = abs(x - 200) + abs(y - 200);
        equation *= -45;
        equation /= 200.00;
        equation += 50;
        //equation /= 100.00;
        if (equation < 5) equation = 5;
        
        int probBuildings = 0;
        for (int i = 0; i < 2; i++) {
            double prob = ((double)rand() / RAND_MAX) * 100.00;
            if (prob <= equation) probBuildings++;
        }
        buildingsToBePlaced = probBuildings;
    }

    while (buidlingsPlaced != buildingsToBePlaced) {
        /* Randomly pick spots on map, check 4 conditions, if pass place, else, pick new point. 
        1. Doesn't go out of bounds
        2. There is no road in placement. 
        3. Placed next to road. 
        4. Not placed on other building 
        */
        int spotFound = 0;
        while (spotFound == 0) {
            int topOrBottom = rand() % 2 + 1;
            //Top
            if (topOrBottom == 1) {
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
            //Bottom
            if (topOrBottom == 2) {
                int spotCol = rand() % (COLUMNS - 3) + 1;
                int spotRow = rand() % (ROWS - 6) + 3;
                //Check conditions
                if (strcmp(map[spotRow][spotCol], ROAD) != 0) continue;
                if (strcmp(map[spotRow + 1][spotCol], CNTR) == 0 ||
                    strcmp(map[spotRow + 2][spotCol], CNTR) == 0 || 
                    strcmp(map[spotRow + 1][spotCol + 1], CNTR) == 0 ||
                    strcmp(map[spotRow + 2][spotCol + 1], CNTR) == 0) continue;
                if (strcmp(map[spotRow + 1][spotCol], ROAD) != 0 &&
                    strcmp(map[spotRow + 2][spotCol], ROAD) != 0 && 
                    strcmp(map[spotRow + 1][spotCol + 1], ROAD) != 0 &&
                    strcmp(map[spotRow + 2][spotCol + 1], ROAD) != 0) {
                        map[spotRow + 1][spotCol] = building;
                        map[spotRow + 2][spotCol] = building;
                        map[spotRow + 1][spotCol + 1] = building;
                        map[spotRow + 2][spotCol + 1] = building;
                        spotFound = 1;
                    }
            }
        }

        if (building == CNTR) {
            building = PKMART;
        } else {
            building = CNTR;
        } 
        buidlingsPlaced++;
    }

    struct map newMap;

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            newMap.map[i][j] = map[i][j];
        }
    }
    newMap.x = x;
    newMap.y = y;
    newMap.northEnt = northEnt;
    newMap.southEnt = southEnt;
    newMap.westEnt = westEnt;
    newMap.eastEnt = eastEnt;

    worldMap[x][y] = malloc(sizeof(struct map));
    if (worldMap[x][y] != NULL) {
        memcpy(worldMap[x][y], &newMap, sizeof(struct map));
    }

    return newMap;
}

void PrintMap(struct map currMap) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            printf("%s", currMap.map[i][j]);
        }
        printf("\n");
    }
    printf("(%d, %d)\n", currMap.x - 200, currMap.y - 200);
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