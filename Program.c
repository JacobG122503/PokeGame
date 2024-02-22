/*
PROGRAM INFO
Author: Jacob Garcia
Version: 1.04
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
 #include <unistd.h>

#include "heap.h"

//Constants
#define ROWS 21
#define COLUMNS 80
#define WORLDROWS 401
#define WORLDCOLUMNS 401
#define MAXNPC 500

//Structs
typedef enum {
    hikerNPC,
    rivalNPC,
    pacerNPC,
    wandererNPC,
    sentryNPC,
    explorerNPC,
    NPCTYPES
} npc;

typedef struct NPCs {
    int x;
    int y;
    int dirX; 
    int dirY;
    npc type;
} NPCs;

struct map {
    char* map[ROWS][COLUMNS];
    int weights[ROWS][COLUMNS];
    NPCs npcs[MAXNPC];
    int hikerMap[ROWS][COLUMNS];
    int rivalMap[ROWS][COLUMNS];
    int nmbOfNPCs;
    int x;
    int y;
    int northEnt;
    int southEnt;
    int westEnt;
    int eastEnt;
};

typedef struct path {
  heap_node_t *hn;
  int x;
  int y;
  int cost;
} path;

typedef struct PlayerChar {
    int x;
    int y;
    int worldX;
    int worldY;
} PlayerChar;

//Prototypes
void PacerMove(int worldX, int worldY, NPCs *currNPC);
void CostMapMove(int worldX, int worldY, NPCs *currNPC);
void SpawnNPCs(int number, int worldX, int worldY);
void PlacePC(int worldX, int worldY);
struct map GenerateMap(int x, int y);
void PrintMap(int worldX, int worldY);
char* FindTerrain();
void DeleteWorld();
static int32_t path_cmp(const void *key, const void *with);
static void Dijkstra(struct map *map, npc npcType, int playerX, int playerY);

//Colors
#define BLACK   "\x1b[30m"
#define RED     "\x1b[31m"
#define BLDRED  "\e[1;31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define PURPLE  "\033[0;34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"
#define RESET   "\x1b[0m"
#define GREY    "\x1b[90m"

//Elements and Characters
char* MTN = GREY "%" RESET;
char* TREE = GREY "^" RESET;
char* ROAD = YELLOW "#" RESET;
char* LNGR = GREEN ":" RESET;
char* CLRNG = GREEN "." RESET;
char* WATER = CYAN "~" RESET;
char* CNTR = MAGENTA "C" RESET;
char* PKMART = MAGENTA "M" RESET;

char* PC = RESET "@" RESET;
char* HIKER = BLDRED "h" RESET;
char* RIVAL = BLDRED "r" RESET;
char* PACER = BLDRED "p" RESET;
char* WANDERER = BLDRED "w" RESET;
char* SENTRY = BLDRED "s" RESET;
char* EXPLORER = BLDRED "e" RESET;

// • Hikers: These will be represented by the letter ’h’. Hikers path to the PC by following a maximum
// gradient on the hiker map.
// • Rivals: These will be represented by the letter ’r’. Rivals path to the PC by following a maximum
// gradient on the rival map.
// • Pacers: These will be represented by the letter ’p’. Pacers start with a direction and walk until they
// hit some terrain they cannot traverse, then they turn around and repeat, pacing back and forth.
// • Wanderers: These will be represented by the letter ’w’. Wanderers never leave the terrain region they
// were spawned in. They have a direction and walk strait ahead to the edge of the terrain, whereupon
// they turn in a random direction and repeat.
// • Sentries: These will be represented by the letter ’s’. Sentries don’t move; they just wait for the action
// to come to them.
// • Explorers: These will be represented by the letter ’e’. Explorers move like wanderers, but they cross
// terrain type boundaries, only changing to a new, random direction when they reach an impassable
// terrain element

struct map *worldMap[WORLDROWS][WORLDCOLUMNS]; 
PlayerChar *Player;

int main(int argc, char *argv[]) {
    int numTrainers = 1;//10; 

    // --numtrainers switch
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--numtrainers") == 0) {
            if (i + 1 < argc) {
                numTrainers = atoi(argv[i + 1]); 
                if (numTrainers > MAXNPC) {
                    system("clear");
                    printf("The max number of npcs is %d. (Why would you want that many anyway!?)", MAXNPC);
                    printf("\nProgram closed.\n");
                    return 1;
                }
            } else {
                printf("Error: --numtrainers switch requires an argument.\n");
                return 1;
            }
        }
    }

    //Make seed and save into seeds.txt
    //Odd seeds
    //1708631599 - Endless loop, cant place buildings
    //1708632100 - Buildings overlap
    time_t seed = time(NULL);
    srand(seed); 
    FILE *seedFile;
    seedFile = fopen("seeds.txt", "a");
    fprintf(seedFile, "%ld\n", seed);
    fclose(seedFile);

    Player = malloc(sizeof(PlayerChar));

    for (int i = 0; i < WORLDROWS; i++) {
        for (int j = 0; j < WORLDCOLUMNS; j++) {
            worldMap[i][j] = NULL;
        }
    }

    int x = 200;
    int y = 200;
    GenerateMap(x, y);
    PlacePC(x, y);
    SpawnNPCs(numTrainers, x, y);

    //Start movement 
    char command = 'c';
    while (command != 'q') {
        system("clear");
        PrintMap(x, y);

        Dijkstra(worldMap[x][y], hikerNPC, Player->x, Player->y);
        Dijkstra(worldMap[x][y], rivalNPC, Player->x, Player->y);

        // for (int i = 0; i < ROWS; i++) {
        //     for (int j = 0; j < COLUMNS; j++) {
        //         if (worldMap[x][y]->hikerMap[i][j] == SHRT_MAX) {
        //             printf("   ");
        //             continue;
        //         }
        //         if (i == Player->x && j == Player->y) {
        //             printf("%s%2d%s ", GREEN, worldMap[x][y]->hikerMap[i][j] % 100, RESET);
        //             continue;
        //         }
        //         // if (i == worldMap[x][y]->npcs[0].x && j == worldMap[x][y]->npcs[0].y) {
        //         //     printf("%s%2d%s ", BLDRED, worldMap[x][y]->hikerMap[i][j] % 100, RESET);
        //         //     continue;
        //         // }
        //         printf("%2d ", worldMap[x][y]->hikerMap[i][j] % 100);
        //     }
        //     printf("\n");
        // }

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
                "f x y: x and y are integers; Fly to map (x, y) and display it.\n"
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

            GenerateMap(x, y);
            continue;
        }
        if (command == 'f') {
            int oldX = x;
            int oldY = y;
            scanf(" %d %d", &x, &y);
            x += 200;
            y += 200;
            if (x > 400 || y > 400 || x < 0 || y < 0) {
                x = oldX;
                y = oldY;
                continue;
            }
            GenerateMap(x, y);
        }
        if (command == 'm') {
            while (1) {
                PacerMove(x, y, &worldMap[x][y]->npcs[0]);
                // CostMapMove(x, y, &worldMap[x][y]->npcs[0]);
                // CostMapMove(x, y, &worldMap[x][y]->npcs[1]);
                PrintMap(x, y);
                usleep(250000);
                system("clear");
            }
            continue;
        }
    }

    printf("Closing game...\n");
    DeleteWorld();

    return 0;
}

void PacerMove(int worldX, int worldY, NPCs *currNPC) {
    //If never moved. Find movement direciton and destination x and y. 
    while (currNPC->dirX == 0 && currNPC->dirY == 0) {
        //Rand int -1, 0, or 1
        currNPC->dirX = (int)(((double)rand() / RAND_MAX) * 3) - 1;
        currNPC->dirY = (int)(((double)rand() / RAND_MAX) * 3) - 1;
    }

    int nextX = currNPC->x + currNPC->dirX;
    int nextY = currNPC->y + currNPC->dirY;
    char *nextTer = worldMap[worldX][worldY]->map[nextX][nextY];
    //If can continue walking, continue
    //Else, go back
    if (strcmp(nextTer, TREE) && strcmp(nextTer, WATER) && strcmp(nextTer, MTN)) {
        currNPC->x = nextX;
        currNPC->y = nextY;
    } else {
        currNPC->dirX = -currNPC->dirX;
        currNPC->dirY = -currNPC->dirY;
    }
}

void CostMapMove(int worldX, int worldY, NPCs *currNPC) {
    int nextX = currNPC->x;
    int nextY = currNPC->y;
    struct map currMap = *worldMap[worldX][worldY];

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            //Ignore center
            if (i == 0 && j == 0) continue;
            //Check area if there is a smaller weight and move to it. 
            if (currNPC->type == hikerNPC && currMap.hikerMap[currNPC->x + i][currNPC->y + j] < currMap.hikerMap[nextX][nextY]) {
                nextX = currNPC->x + i;
                nextY = currNPC->y + j;
            } else if (currNPC->type == rivalNPC && currMap.rivalMap[currNPC->x + i][currNPC->y + j] < currMap.rivalMap[nextX][nextY]) {
                nextX = currNPC->x + i;
                nextY = currNPC->y + j;
            }
        }
    }

    currNPC->x = nextX;
    currNPC->y = nextY;
}

void SpawnNPCs(int number, int worldX, int worldY) {
    struct map *currentMap = worldMap[worldX][worldY];
    currentMap->nmbOfNPCs = number;
    npc npcList[number];

    for (int i = 0; i < number; i++) {
        //Guarantees first two are hiker and rival
        if (i == 0) {
            npcList[i] = pacerNPC;//hikerNPC;
            continue;
        } else if (i == 1) {
            npcList[i] = rivalNPC;
            continue;
        }
        
        int nextNpc = rand() % NPCTYPES;
        npcList[i] = nextNpc;
    }

    //Initialize all the npcs
    for (int i = 0; i < number; i++) {
        NPCs currNPC;
        currNPC.type = npcList[i];
        currNPC.x = rand() % (ROWS - 2) + 1;
        currNPC.y = rand() % (COLUMNS - 2) + 1;
        currNPC.dirX = 0;
        currNPC.dirY= 0;

        if (!strcmp(currentMap->map[currNPC.x][currNPC.y], WATER) || !strcmp(currentMap->map[currNPC.x][currNPC.y], TREE)) {
            i--;
            continue;
        }

        currentMap->npcs[i] = currNPC;
    }
}

void PlacePC(int worldX, int worldY) {
    struct map *map = worldMap[worldX][worldY];
    int placed = 0;

    while (placed == 0) {
        int row = rand() % (ROWS - 2) + 1;
        int col = rand() % (COLUMNS - 2) + 1;

        if (!strcmp(map->map[row][col], ROAD)) {
            Player->x = row;
            Player->y = col;
            Player->worldX = worldX;
            Player->worldY = worldY;
            placed = 1;
        }
    }
}

struct map GenerateMap(int x, int y) {
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

    //Copy map over to World Map
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
    newMap.nmbOfNPCs = 0;

    worldMap[x][y] = malloc(sizeof(struct map));
    if (worldMap[x][y] != NULL) {
        memcpy(worldMap[x][y], &newMap, sizeof(struct map));
    }

    return newMap;
}

void PrintMap(int worldX, int worldY) {
    struct map currMap = *worldMap[worldX][worldY];

    //Place NPCs / PC
    if (worldX == Player->worldX && worldY == Player->worldY) {
        currMap.map[Player->x][Player->y] = PC;
    }
    for (int i = 0; i < currMap.nmbOfNPCs; i++) {
        char *nextNPC;
        NPCs currNPC = currMap.npcs[i];

        switch (currNPC.type) {
        case hikerNPC:
            nextNPC = HIKER;
            break;
        case rivalNPC:
            nextNPC = RIVAL;
            break;
        case pacerNPC:
            nextNPC = PACER;
            break;
        case wandererNPC:
            nextNPC = WANDERER;
            break;
        case sentryNPC:
            nextNPC = SENTRY;
            break;
        case explorerNPC:
            nextNPC = EXPLORER;
            break;
        default:
            system("clear");
            printf("Error generating and printing npcs");
            exit(0);
        }

        currMap.map[currNPC.x][currNPC.y] = nextNPC;
    }

    //Print map
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

void DeleteWorld() {
    free(Player);
    for (int i = 0; i < WORLDROWS; i++) {
        for (int j = 0; j < WORLDCOLUMNS; j++) {
            free(worldMap[i][j]);
            worldMap[i][j] = NULL;
        }
    }
}

static int32_t path_cmp(const void *key, const void *with){
  return ((path *) key)->cost - ((path *) with)->cost;
}

/*
Credit Geeks for Geeks for inspiration/structure
*/
static void Dijkstra(struct map *map, npc npcType, int playerX, int playerY){
    path npcPath[ROWS][COLUMNS], *npcP;
    heap_t h;

    //Initialize terrain weights
    if (npcType == hikerNPC) {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                char *terrain = map->map[i][j];

                if (!strcmp(terrain, CLRNG) || !strcmp(terrain, ROAD)) {
                    map->weights[i][j] = 10;
                } else if (!strcmp(terrain, LNGR) || !strcmp(terrain, TREE)) {
                    map->weights[i][j] = 15;
                } else if (!strcmp(terrain, PKMART) || !strcmp(terrain, CNTR)) {
                    map->weights[i][j] = 50;
                } else {
                    map->weights[i][j] = SHRT_MAX;
                }
            }
        }
    } 
    else if (npcType == rivalNPC) {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                char *terrain = map->map[i][j];

                if (!strcmp(terrain, CLRNG) || !strcmp(terrain, ROAD)) {
                    map->weights[i][j] = 10;
                } else if (!strcmp(terrain, LNGR)) {
                    map->weights[i][j] = 20;
                } else if (!strcmp(terrain, PKMART) || !strcmp(terrain, CNTR)) {
                    map->weights[i][j] = 50;
                } else {
                    map->weights[i][j] = SHRT_MAX;
                }
            }
        }
    }
  
    // Set cost of all cells to SHRT_MAX 
    for (int i = 0; i < ROWS; i++){
        for (int j = 0; j < COLUMNS; j++){
            npcPath[i][j].x = i;
            npcPath[i][j].y = j;
            npcPath[i][j].hn = NULL;
            npcPath[i][j].cost = SHRT_MAX;
        }
    }

    npcPath[playerX][playerY].cost = 0;

    heap_init(&h, path_cmp, NULL);

    //Go through and insert into heap if not infinity
    for(int i = 1; i < ROWS - 1; i++){
        for(int j = 1; j < COLUMNS - 1; j++){
            if(map->weights[i][j] != SHRT_MAX){
                npcPath[i][j].hn = heap_insert(&h, &npcPath[i][j]);
            }
        }
    }

    while ((npcP = heap_remove_min(&h))){
        npcP->hn = NULL;

        // Check all surrounding of the minimum on heap
        for(int i = -1; i <= 1; i++){
            for(int j = -1; j <= 1; j++){
                //Check if going out of bounds
                if (npcP->x + i > ROWS - 1 || npcP->y + j > COLUMNS - 1) continue;
                if (npcP->x > ROWS - 1 || npcP->y > COLUMNS - 1) continue;
                path current = npcPath[npcP->x + i][npcP->y + j]; 
                int centerWeight = map->weights[npcP->x][npcP->y];
                if (!(i == 0 && j == 0) && 
                    (current.hn) && 
                    (current.cost > ((npcP->cost + centerWeight)))) {
                        npcPath[npcP->x + i][npcP->y + j].cost = ((npcP->cost + centerWeight));
                        heap_decrease_key_no_replace(&h, current.hn);
                }
            }
        }
    }

    // Add cost maps to map struct
    if (npcType == hikerNPC) {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                map->hikerMap[i][j] = npcPath[i][j].cost;
            }
        }
    } else if (npcType == rivalNPC) {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                map->rivalMap[i][j] = npcPath[i][j].cost;
            }
        }
    }

    //Print costs. NOTE: REMOVE AFTER ASSIGNMENT 1.03
    // for (int i = 0; i < ROWS; i++){
    //     for (int j = 0; j < COLUMNS; j++){
    //         if(npcPath[i][j].cost == SHRT_MAX){
    //             printf("   ");
    //             continue;
    //         }
    //         if (i == playerX && j == playerY) {
    //             printf("%s%2d%s ", GREEN, npcPath[i][j].cost % 100, RESET);
    //             continue;
    //         }
    //         printf("%2d ", npcPath[i][j].cost % 100);
    //     }
    //     printf("\n");
    // }
    
}