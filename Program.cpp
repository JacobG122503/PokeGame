/*
PROGRAM INFO
Author: Jacob Garcia
Version: 1.06
*/
#include <iostream>
#include <limits.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
    int alive;
    int x;
    int y;
    int dirX; 
    int dirY;
    int turnTime;
    npc type;
} NPCs;

struct map {
    std::string map[ROWS][COLUMNS];
    int hikerWeights[ROWS][COLUMNS];
    int othersWeights[ROWS][COLUMNS];
    NPCs npcs[MAXNPC];
    heap_t turns;
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
    int turnTime;
} PlayerChar;

//Prototypes
void SetupColors();
void MoveNPC(int worldX, int worldY, NPCs *currNPC);
int MoveNPC_CheckValid(int worldX, int worldY, int nextX, int nextY);
void SpawnNPCs(int number, int worldX, int worldY);
void PlacePC(int worldX, int worldY);
struct map GenerateMap(int x, int y);
void PrintMap(int worldX, int worldY);
// void PrintExploredWorld();
std::string FindTerrain();
void DeleteWorld();
static int32_t path_cmp(const void *key, const void *with);
static int32_t npc_turn_cmp(const void *key, const void *with);
static void Dijkstra(struct map *map, npc npcType, int playerX, int playerY);

//Elements and Characters
std::string MTN = "%";
std::string TREE = "^";
std::string ROAD = "#";
std::string LNGR = ":";
std::string CLRNG = ".";
std::string WATER = "~";
std::string CNTR = "C";
std::string PKMART = "M";

std::string HIKER = "h";
std::string RIVAL = "r";
std::string PACER = "p";
std::string WANDERER = "w";
std::string SENTRY = "s";
std::string EXPLORER = "e";

struct map *worldMap[WORLDROWS][WORLDCOLUMNS]; 
PlayerChar *Player;

int main(int argc, char *argv[]) {
    int numTrainers = 10; 

    // --numtrainers switch
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--numtrainers") == 0) {
            if (i + 1 < argc) {
                numTrainers = atoi(argv[i + 1]); 
                if (numTrainers > MAXNPC) {
                    
                    printw("The max number of npcs is %d. (Why would you want that many anyway!?)", MAXNPC);
                    printw("\nProgram closed.\n");
                    return 1;
                }
            } else {
                printw("Error: --numtrainers switch requires an argument.\n");
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

    Player = new PlayerChar;

    for (int i = 0; i < WORLDROWS; i++) {
        for (int j = 0; j < WORLDCOLUMNS; j++) {
            worldMap[i][j] = NULL;
        }
    }

    int x = 200;
    int y = 200;
    std::string statusMessage = "";
    GenerateMap(x, y);
    PlacePC(x, y);
    SpawnNPCs(numTrainers, x, y);

    initscr();
    raw(); 
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    SetupColors();

    //Start movement 
    char command = 'c';
    while (command != 'Q') {
        Dijkstra(worldMap[x][y], hikerNPC, Player->x, Player->y);
        Dijkstra(worldMap[x][y], rivalNPC, Player->x, Player->y);

        // Check if it is PCs turn
        NPCs *nextNPC = (NPCs *) heap_remove_min(&worldMap[x][y]->turns);
        NPCs *battleNPC = NULL;
        int battleTime = 0;

        if (Player->turnTime > nextNPC->turnTime) {
            MoveNPC(x, y, nextNPC);

            //Check if battle time 
            battleTime = nextNPC->x == Player->x && nextNPC->y == Player->y && nextNPC->alive;
            if (battleTime) {
                statusMessage = "You have entered a battle!! Hit esc or q to defeat them.";
                battleNPC = nextNPC;
            } else {
                heap_insert(&worldMap[x][y]->turns, nextNPC);
                continue;
            }
        }
        heap_insert(&worldMap[x][y]->turns, nextNPC);

        clear();
        PrintMap(x, y); //TODO make a "world explored" print function. Just print a single char for each map. would be cool
        attron(COLOR_PAIR(COLOR_MAGENTA));
        printw(statusMessage.c_str());
        attroff(COLOR_PAIR(COLOR_MAGENTA));
        refresh();
        statusMessage = "";

        if (battleTime) {
            while (battleTime) {
                command = getch();
                if (command == 'q' || command == 27) {
                    battleNPC->alive = 0;
                    battleTime = false;

                    // NPC death animation
                    attron(COLOR_PAIR(COLOR_YELLOW) | A_BOLD);
                    for (int i = 0; i < 2; i++) {
                        for (int j = -i; j <= i; j++) {
                            mvprintw(battleNPC->x + i, battleNPC->y + j, "O");
                            mvprintw(battleNPC->x - i, battleNPC->y + j, "O");
                        }
                        for (int j = -(i - 1); j <= (i - 1); j++) {
                            mvprintw(battleNPC->x + j, battleNPC->y + i, "O");
                            mvprintw(battleNPC->x + j, battleNPC->y - i, "O");
                        }
                        refresh();
                        usleep(110000);
                    }
                    attroff(COLOR_PAIR(COLOR_YELLOW) | A_BOLD);
                    attron(COLOR_PAIR(COLOR_RED) | A_BOLD);
                    for (int i = 0; i < 2; i++) {
                        for (int j = -i; j <= i; j++) {
                            mvprintw(battleNPC->x + i, battleNPC->y + j, "X");
                            mvprintw(battleNPC->x - i, battleNPC->y + j, "X");
                        }
                        for (int j = -(i - 1); j <= (i - 1); j++) {
                            mvprintw(battleNPC->x + j, battleNPC->y + i, "X");
                            mvprintw(battleNPC->x + j, battleNPC->y - i, "X");
                        }
                        refresh();
                        usleep(110000);
                    }
                    attroff(COLOR_PAIR(COLOR_RED) | A_BOLD);
                    usleep(150000);
                }
            }
            continue;
        }

        command = getch();

        // Movement commands
        int moveX = 0;
        int moveY = 0;
        // Upper left
        if (command == '7' || command == 'y') {
            moveX--;
            moveY--;
        }
        // Up
        else if (command == '8' || command == 'k') {
            moveX--;
        }
        // Upper right
        else if (command == '9' || command == 'u') {
            moveY++;
            moveX--;
        }
        // Right
        else if (command == '6' || command == 'l') {
            moveY++;
        }
        // Lower right
        else if (command == '3' || command == 'n') {
            moveY++;
            moveX++;
        }
        // Down
        else if (command == '2' || command == 'j') {
            moveX++;
        }
        // Lower left
        else if (command == '1' || command == 'b') {
            moveY--;
            moveX++;
        }
        // Left
        else if (command == '4' || command == 'h') {
            moveY--;
        }

        // Run checks
        if (!(moveX == 0 && moveY == 0)) {
            if (Player->x + moveX > 0 && Player->y + moveY > 0 &&
                Player->x + moveX < ROWS - 1 && Player->y + moveY < COLUMNS - 1 &&
                strcmp(worldMap[x][y]->map[Player->x + moveX][Player->y + moveY].c_str(), WATER.c_str()) &&
                strcmp(worldMap[x][y]->map[Player->x + moveX][Player->y + moveY].c_str(), TREE.c_str())) {

                // Add time to Player from last terrain weight and move
                Player->turnTime += worldMap[x][y]->othersWeights[Player->x][Player->y];
                Player->x += moveX;
                Player->y += moveY;
            } else if ((Player->x + moveX == 0 || Player->y + moveY == 0 ||
                       Player->x + moveX == ROWS - 1 || Player->y + moveY == COLUMNS - 1) &&
                       !strcmp(worldMap[x][y]->map[Player->x + moveX][Player->y + moveY].c_str(), ROAD.c_str())) {

                // Player is going to next map
                int newWorldX = 0;
                int newWorldY = 0;
                int newPCX, newPCY;
                if (Player->y + moveY == worldMap[x][y]->northEnt && Player->x + moveX == 0) {
                    //North
                    newWorldY++;
                    newPCX = ROWS - 2;
                    newPCY = worldMap[x][y]->northEnt;
                } else if (Player->y + moveY == worldMap[x][y]->southEnt && Player->x + moveX == ROWS - 1) {
                    //South
                    newWorldY--;
                    newPCX = 1;
                    newPCY = worldMap[x][y]->southEnt;
                } else if (Player->y + moveY == 0 && Player->x + moveX == worldMap[x][y]->westEnt) {
                    //West
                    newWorldX--;
                    newPCX = worldMap[x][y]->westEnt;
                    newPCY = COLUMNS - 2;
                } else if (Player->y + moveY == COLUMNS - 1 && Player->x + moveX == worldMap[x][y]->eastEnt) {
                    //East
                    newWorldX++;
                    newPCX = worldMap[x][y]->eastEnt;
                    newPCY = 1;
                }

                if (!(newWorldX == 0 && newWorldY == 0) && !(x + newWorldX > 400 || y + newWorldY > 400 || x + newWorldX < 0 || y + newWorldY < 0)) {
                    x += newWorldX;
                    y += newWorldY;

                    Player->turnTime = 0;
                    Player->x = newPCX;
                    Player->y = newPCY;
                    Player->worldX = x;
                    Player->worldY = y;

                    GenerateMap(x, y);
                    if (!worldMap[x][y]->turns.initialized) {
                        SpawnNPCs(numTrainers, x, y);
                    } else {
                        // Set all npcs turn time back to zero
                        for (int i = 0; i < numTrainers; i++) {
                            worldMap[x][y]->npcs[i].turnTime = 0;
                        }
                    }
                }
            }
        }

        // Rest for a turn
        if (command == '5' || command == ' ' || command == '.') {
            Player->turnTime += worldMap[x][y]->othersWeights[Player->x][Player->y];
        }

        // Enter building
        if (command == '>') {
            if (!strcmp(worldMap[x][y]->map[Player->x][Player->y].c_str(), CNTR.c_str()) ||
                !strcmp(worldMap[x][y]->map[Player->x][Player->y].c_str(), PKMART.c_str())) {

                char building = ' ';
                attron(COLOR_PAIR(COLOR_MAGENTA));
                printw("You have entered the building! Type < to leave");
                attroff(COLOR_PAIR(COLOR_MAGENTA));
                refresh();
                while (building != '<')
                    building = getch();
            }
        }

        // Fly
        if (command == 'f') {
            clear();
            attron(COLOR_PAIR(COLOR_MAGENTA));
            printw("You are about to fly! Please type your two coordinates you would like to fly to or type q to cancel: \n");
            refresh();
            attroff(COLOR_PAIR(COLOR_MAGENTA));

            echo();
            char input[4];
            getstr(input);
            if (!strcmp(input, "q"))
                continue;
            char input2[4];
            getstr(input2);
            noecho();

            int newX = atoi(input);
            int newY = atoi(input2);
            newX += 200;
            newY += 200;

            if (!(newX > 400 || newY > 400 || newX < 0 || newY < 0)) {
                x = newX;
                y = newY;
                GenerateMap(x, y);

                //Place PC on new map
                int placed = 0;
                while (placed == 0) {
                    int row = rand() % (ROWS - 2) + 1;
                    int col = rand() % (COLUMNS - 2) + 1;
                    if (!strcmp(worldMap[x][y]->map[row][col].c_str(), ROAD.c_str())) {
                        Player->x = row;
                        Player->y = col;
                        Player->turnTime = 0;
                        Player->worldX = x;
                        Player->worldY = y;

                        placed = 1;
                    }
                }

                if (!worldMap[x][y]->turns.initialized) {
                    SpawnNPCs(numTrainers, x, y);
                } else {
                    // Set all npcs turn time back to zero
                    for (int i = 0; i < numTrainers; i++) {
                        worldMap[x][y]->npcs[i].turnTime = 0;
                    }
                }
            } else {
                statusMessage = "Error flying to coordinates. Please make sure they are in the world bounds.";
            }
        }

        //Show World Exploration Map
        // if (command == 'E') {
        //     PrintExploredWorld();
        // }

        // View trainer list
        if (command == 't') {
            //Set up box for info
            for (int i = 0; i < (ROWS - 4) / 2; i++) {
                //Top row
                for (int j = 0; j < COLUMNS - 8; j++) {
                    mvprintw(3 + i, 4 + j, " ");
                }
                //Bottom row
                for (int j = 0; j < COLUMNS - 8; j++) {
                    mvprintw((ROWS - 4) - i, 4 + j, " ");
                }
                usleep(35000);
                refresh();
            }
            //Print info
            usleep(10000);
            attron(COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
            mvprintw(4, (COLUMNS/2) - 6, "TRAINER INFO");

            //Set up trainer info string array
            std::string trainerInfo[numTrainers];
            for (int i = 0; i < numTrainers; i++) {
                NPCs currNPC = worldMap[x][y]->npcs[i];

                // Get name of npc
                std::string name;
                switch (currNPC.type) {
                case hikerNPC:
                    name = "Hiker";
                    break;
                case rivalNPC:
                    name = "Rival";
                    break;
                case pacerNPC:
                    name = "Pacer";
                    break;
                case wandererNPC:
                    name = "Wanderer";
                    break;
                case sentryNPC:
                    name = "Sentry";
                    break;
                case explorerNPC:
                    name = "Explorer";
                    break;
                default:
                    name = "Unknown NPC";
                    break;
                }

                if (!currNPC.alive) {
                    trainerInfo[i] = name + " - (dead)";
                    continue;
                }

                std::string xStr, yStr;
                int xDist = Player->x - currNPC.x;
                int yDist = Player->y - currNPC.y;
                if (xDist < 0) {
                    xStr = "South " + std::to_string(abs(xDist)) + " squares";
                } else {
                    xStr = "North " + std::to_string(xDist) + " squares";
                }
                if (yDist < 0) {
                    yStr = "East " + std::to_string(abs(yDist)) + " squares";
                } else {
                    yStr = "West " + std::to_string(yDist) + " squares";
                }

                trainerInfo[i] = name + " - Location: " + xStr + ", " + yStr;
            }

            // Print all trainers
            int i, s;
            int scroll = 0;
            int intCommand = 0;
            // 27 is esc
            while (intCommand != 27 && intCommand != 'q') {
                // Clear area
                for (i = 0; i < (ROWS - 9); i++) {
                    for (int j = 0; j < COLUMNS - 8; j++) {
                        mvprintw(6 + i , 4 + j, " ");
                    }
                }

                // Print trainers
                for (i = 0 + scroll, s = 0; i < numTrainers && (6 + s) < ROWS - 4; i++, s += 2) {
                    mvprintw(6 + s, 5, "%s", trainerInfo[i].c_str());
                }
                refresh();
                intCommand = getch();

                // Scroll logic
                if (intCommand == KEY_UP) {
                    if ((scroll - 1) >= 0)
                        scroll--;
                } else if (intCommand == KEY_DOWN) {
                    if ((scroll + 1) < numTrainers)
                        scroll++;
                }
            }

            attroff(COLOR_PAIR(COLOR_MAGENTA) | A_BOLD);
        }

        //Old movement logic
        //Instructions
        // if (command == 'i') {
        //     printw("%sCOMMAND LIST%s\n","green", "reset");
        //     printw("n: Move to the map immediately north of the current map and display it.\n"
        //         "s: Move to the map immediately south of the current map and display it.\n"
        //         "e: Move to the map immediately east of the current map and display it.\n"
        //         "w: Move to the map immediately west of the current map and display it.\n"
        //         "f x y: x and y are integers; Fly to map (x, y) and display it.\n"
        //         "q: Quit the game.\n");
        //     printw("\nType c to continue: ");
        //     refresh();
        //     while (command != 'c') command = getchar();
        //     continue;
        // }

        // if (command == 'n' || command == 'e' || command == 's' || command == 'w') {
        //     if (command == 'n' && !(x > 400 || y + 1 > 400 || x < 0 || y < 0)) y++;
        //     if (command == 'e' && !(x + 1 > 400 || y > 400 || x < 0 || y < 0)) x++;
        //     if (command == 's' && !(x > 400 || y > 400 || x < 0 || y - 1 < 0)) y--;
        //     if (command == 'w' && !(x > 400 || y > 400 || x - 1 < 0 || y < 0)) x--;

        //     GenerateMap(x, y);
        //     continue;
        // }
        // if (command == 'f') {
        //     int oldX = x;
        //     int oldY = y;
        //     scanf(" %d %d", &x, &y);
        //     x += 200;
        //     y += 200;
        //     if (x > 400 || y > 400 || x < 0 || y < 0) {
        //         x = oldX;
        //         y = oldY;
        //         continue;
        //     }
        //     GenerateMap(x, y);
        // }
        // if (command == 'm') {
        //     while (1) {
        //         NextTurn(x, y);
        //         PrintMap(x, y);
        //         refresh();
        //         usleep(40000);
        //         clear();
        //     }
        //     continue;
        // }
    }

    printw("Closing game...\n");
    refresh();
    DeleteWorld();
    endwin();

    return 0;
}

void SetupColors() {
    start_color();
    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}

void MoveNPC(int worldX, int worldY, NPCs *currNPC) {
    if (currNPC->type == hikerNPC || currNPC->type == rivalNPC) {
        int nextX = currNPC->x;
        int nextY = currNPC->y;
        struct map currMap = *worldMap[worldX][worldY];

        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                // Ignore center
                if (i == 0 && j == 0)
                    continue;
                // Check area if there is a smaller weight and move to it.
                if (currNPC->type == hikerNPC && currMap.hikerMap[currNPC->x + i][currNPC->y + j] < currMap.hikerMap[nextX][nextY]
                    && MoveNPC_CheckValid(worldX, worldY, nextX + i, nextY + j)) {
                    nextX = currNPC->x + i;
                    nextY = currNPC->y + j;
                } else if (currNPC->type == rivalNPC && currMap.rivalMap[currNPC->x + i][currNPC->y + j] < currMap.rivalMap[nextX][nextY]
                    && MoveNPC_CheckValid(worldX, worldY, nextX + i, nextY + j)) {
                    nextX = currNPC->x + i;
                    nextY = currNPC->y + j;
                }
            }
        }

        currNPC->x = nextX;
        currNPC->y = nextY;

        //Add time to npc
        if (currNPC->type == hikerNPC) {
            currNPC->turnTime += currMap.hikerWeights[nextX][nextY];
        } else if (currNPC->type == rivalNPC) {
            currNPC->turnTime += currMap.othersWeights[nextX][nextY];
        }
    } 
    else if (currNPC->type == pacerNPC) {
        // If never moved. Find movement direciton and destination x and y.
        while (currNPC->dirX == 0 && currNPC->dirY == 0) {
            // Rand int -1, 0, or 1
            currNPC->dirX = (int)(((double)rand() / RAND_MAX) * 3) - 1;
            currNPC->dirY = (int)(((double)rand() / RAND_MAX) * 3) - 1;
        }

        int nextX = currNPC->x + currNPC->dirX;
        int nextY = currNPC->y + currNPC->dirY;
        std::string nextTer = worldMap[worldX][worldY]->map[nextX][nextY];
        // If can continue walking, continue
        // Else, go back
        if (strcmp(nextTer.c_str(), TREE.c_str()) && strcmp(nextTer.c_str(), WATER.c_str()) && strcmp(nextTer.c_str(), MTN.c_str())
            && MoveNPC_CheckValid(worldX, worldY, nextX, nextY)) {
            currNPC->x = nextX;
            currNPC->y = nextY;
            currNPC->turnTime += worldMap[worldX][worldY]->othersWeights[nextX][nextY];
        } else {
            currNPC->dirX = -currNPC->dirX;
            currNPC->dirY = -currNPC->dirY;
            currNPC->turnTime += worldMap[worldX][worldY]->othersWeights[currNPC->x][currNPC->y];
        }
    } 
    else if (currNPC->type == wandererNPC) {
        //Wanderes never leave their terrain
        std::string currTer = worldMap[worldX][worldY]->map[currNPC->x][currNPC->y];
        //Find direction
        while (currNPC->dirX == 0 && currNPC->dirY == 0) {
            currNPC->dirX = (int)(((double)rand() / RAND_MAX) * 3) - 1;
            currNPC->dirY = (int)(((double)rand() / RAND_MAX) * 3) - 1;
        }

        int nextX = currNPC->x + currNPC->dirX;
        int nextY = currNPC->y + currNPC->dirY;
        std::string nextTer = worldMap[worldX][worldY]->map[nextX][nextY];
        if (!strcmp(currTer.c_str(), nextTer.c_str())
            && MoveNPC_CheckValid(worldX, worldY, nextX, nextY)) {
            currNPC->x = nextX;
            currNPC->y = nextY;
            currNPC->turnTime += worldMap[worldX][worldY]->othersWeights[nextX][nextY];
        } else {
            currNPC->dirX = 0;
            currNPC->dirY = 0;
            currNPC->turnTime += worldMap[worldX][worldY]->othersWeights[currNPC->x][currNPC->y];
        }
    } 
    else if (currNPC->type == explorerNPC) {
        //Find direction
        while (currNPC->dirX == 0 && currNPC->dirY == 0) {
            currNPC->dirX = (int)(((double)rand() / RAND_MAX) * 3) - 1;
            currNPC->dirY = (int)(((double)rand() / RAND_MAX) * 3) - 1;
        }

        int nextX = currNPC->x + currNPC->dirX;
        int nextY = currNPC->y + currNPC->dirY;
        std::string nextTer = worldMap[worldX][worldY]->map[nextX][nextY];
        if (strcmp(nextTer.c_str(), WATER.c_str()) && strcmp(nextTer.c_str(), MTN.c_str()) && strcmp(nextTer.c_str(), TREE.c_str())
            && MoveNPC_CheckValid(worldX, worldY, nextX, nextY)) {
            currNPC->x = nextX;
            currNPC->y = nextY;
            currNPC->turnTime += worldMap[worldX][worldY]->othersWeights[nextX][nextY];
        } else {
            currNPC->dirX = 0;
            currNPC->dirY = 0;
            currNPC->turnTime += worldMap[worldX][worldY]->othersWeights[currNPC->x][currNPC->y];
        }
    }
    else if (currNPC->type == sentryNPC) {
        currNPC->turnTime += worldMap[worldX][worldY]->othersWeights[currNPC->x][currNPC->y];
    }
}

int MoveNPC_CheckValid(int worldX, int worldY, int nextX, int nextY) {
    struct map currMap = *worldMap[worldX][worldY];

    // Check if next spot is gate
    if ((nextX == 0 && nextY == currMap.northEnt) ||
        (nextX == ROWS - 1 && nextY == currMap.southEnt) ||
        (nextX == currMap.westEnt && nextY == 0) ||
        (nextX == currMap.eastEnt && nextY == COLUMNS - 1)) {
        return 0;
    }

    //Check if next spot has an NPC there already
    for (int i = 0; i < currMap.nmbOfNPCs; i++) {
        if (!currMap.npcs[i].alive) continue;
        if (currMap.npcs[i].x == nextX &&
            currMap.npcs[i].y == nextY) {
                return 0;
            }
    }

    return 1;
}

void SpawnNPCs(int number, int worldX, int worldY) {
    struct map *currentMap = worldMap[worldX][worldY];

    heap_init(&currentMap->turns, npc_turn_cmp, NULL);
    currentMap->turns.initialized = 1;

    currentMap->nmbOfNPCs = number;
    npc npcList[number];

    for (int i = 0; i < number; i++) {
        //Guarantees first two are hiker and rival
        if (i == 0) {
            npcList[i] = hikerNPC;
            continue;
        } else if (i == 1) {
            npcList[i] = rivalNPC;
            continue;
        }
        
        int nextNpc = rand() % NPCTYPES;
        npcList[i] = (npc) nextNpc;
    }

    //Initialize all the npcs
    for (int i = 0; i < number; i++) {
        NPCs currNPC;
        currNPC.type = npcList[i];
        currNPC.x = rand() % (ROWS - 2) + 1;
        currNPC.y = rand() % (COLUMNS - 2) + 1;
        currNPC.dirX = 0;
        currNPC.dirY= 0;
        currNPC.turnTime = 0;
        currNPC.alive = 1;

        if (!strcmp(currentMap->map[currNPC.x][currNPC.y].c_str(), WATER.c_str()) || !strcmp(currentMap->map[currNPC.x][currNPC.y].c_str(), TREE.c_str())) {
            i--;
            continue;
        }

        currentMap->npcs[i] = currNPC;
        heap_insert(&currentMap->turns, &currentMap->npcs[i]);
    }


}

void PlacePC(int worldX, int worldY) {
    struct map *map = worldMap[worldX][worldY];
    int placed = 0;

    while (placed == 0) {
        int row = rand() % (ROWS - 2) + 1;
        int col = rand() % (COLUMNS - 2) + 1;

        if (!strcmp(map->map[row][col].c_str(), ROAD.c_str())) {
            Player->x = row;
            Player->y = col;
            Player->worldX = worldX;
            Player->worldY = worldY;
            Player->turnTime = 0;
            placed = 1;
        }
    }
}

struct map GenerateMap(int x, int y) {
    //Check if map already exists
    if (worldMap[x][y] != NULL) return *worldMap[x][y];

    std::string map[ROWS][COLUMNS];

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
        std::string terrain = FindTerrain();
        for (int i = x; i < x + newXLength; i++) {
            for (int j = y; j < y + newYLength; j++) {
                //Check if on border
                if ((i > 0) && (i < ROWS - 1) && (j > 0) && (j < COLUMNS - 1)) {
                    if (strcmp(map[i][j].c_str(), " ") == 0) blankSpace--;
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
    std::string building;
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
                if (strcmp(map[spotRow][spotCol].c_str(), ROAD.c_str()) != 0) continue;
                if (strcmp(map[spotRow - 1][spotCol].c_str(), CNTR.c_str()) == 0 ||
                    strcmp(map[spotRow - 2][spotCol].c_str(), CNTR.c_str()) == 0 || 
                    strcmp(map[spotRow - 1][spotCol + 1].c_str(), CNTR.c_str()) == 0 ||
                    strcmp(map[spotRow - 2][spotCol + 1].c_str(), CNTR.c_str()) == 0) continue;
                if (strcmp(map[spotRow - 1][spotCol].c_str(), ROAD.c_str()) != 0 &&
                    strcmp(map[spotRow - 2][spotCol].c_str(), ROAD.c_str()) != 0 && 
                    strcmp(map[spotRow - 1][spotCol + 1].c_str(), ROAD.c_str()) != 0 &&
                    strcmp(map[spotRow - 2][spotCol + 1].c_str(), ROAD.c_str()) != 0) {
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
                if (strcmp(map[spotRow][spotCol].c_str(), ROAD.c_str()) != 0) continue;
                if (strcmp(map[spotRow + 1][spotCol].c_str(), CNTR.c_str()) == 0 ||
                    strcmp(map[spotRow + 2][spotCol].c_str(), CNTR.c_str()) == 0 || 
                    strcmp(map[spotRow + 1][spotCol + 1].c_str(), CNTR.c_str()) == 0 ||
                    strcmp(map[spotRow + 2][spotCol + 1].c_str(), CNTR.c_str()) == 0) continue;
                if (strcmp(map[spotRow + 1][spotCol].c_str(), ROAD.c_str()) != 0 &&
                    strcmp(map[spotRow + 2][spotCol].c_str(), ROAD.c_str()) != 0 && 
                    strcmp(map[spotRow + 1][spotCol + 1].c_str(), ROAD.c_str()) != 0 &&
                    strcmp(map[spotRow + 2][spotCol + 1].c_str(), ROAD.c_str()) != 0) {
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
    newMap.turns.initialized = 0;

    // Initialize terrain weights
    // Hiker weights
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            std::string terrain = newMap.map[i][j];

            if (!strcmp(terrain.c_str(), CLRNG.c_str()) || !strcmp(terrain.c_str(), ROAD.c_str())) {
                newMap.hikerWeights[i][j] = 10;
            } else if (!strcmp(terrain.c_str(), LNGR.c_str()) || !strcmp(terrain.c_str(), TREE.c_str())) {
                newMap.hikerWeights[i][j] = 15;
            } else if (!strcmp(terrain.c_str(), PKMART.c_str()) || !strcmp(terrain.c_str(), CNTR.c_str())) {
                newMap.hikerWeights[i][j] = 50;
            } else {
                newMap.hikerWeights[i][j] = SHRT_MAX;
            }
        }
    }
    //Rival and Other weights
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            std::string terrain = newMap.map[i][j];

            if (!strcmp(terrain.c_str(), CLRNG.c_str()) || !strcmp(terrain.c_str(), ROAD.c_str())) {
                newMap.othersWeights[i][j] = 10;
            } else if (!strcmp(terrain.c_str(), LNGR.c_str())) {
                newMap.othersWeights[i][j] = 20;
            } else if (!strcmp(terrain.c_str(), PKMART.c_str()) || !strcmp(terrain.c_str(), CNTR.c_str())) {
                newMap.othersWeights[i][j] = 50;
            } else {
                newMap.othersWeights[i][j] = SHRT_MAX;
            }
        }
    }

    worldMap[x][y] = new struct map;
    if (worldMap[x][y] != NULL) {
        memcpy(worldMap[x][y], &newMap, sizeof(struct map));
    }

    return newMap;
}

void PrintMap(int worldX, int worldY) {
    struct map currMap = *worldMap[worldX][worldY];

    //Place NPCs / PC
    if (worldX == Player->worldX && worldY == Player->worldY) {
        currMap.map[Player->x][Player->y] = "@";
    }
    for (int i = 0; i < currMap.nmbOfNPCs; i++) {
        std::string nextNPC;
        NPCs currNPC = currMap.npcs[i];
        if (!currNPC.alive) continue;

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
            
            printw("Error generating and printing npcs");
            exit(0);
        }

        currMap.map[currNPC.x][currNPC.y] = nextNPC;
    }

    //Print map
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            //Find color to switch to
            int color = -1;
            int bold = 0;
            if (!strcmp(currMap.map[i][j].c_str(), TREE.c_str()) || !strcmp(currMap.map[i][j].c_str(), MTN.c_str())) {
                color = COLOR_WHITE;
            } else if (!strcmp(currMap.map[i][j].c_str(), ROAD.c_str())) {
                color = COLOR_YELLOW;
            } else if (!strcmp(currMap.map[i][j].c_str(), LNGR.c_str()) || !strcmp(currMap.map[i][j].c_str(), CLRNG.c_str())) {
                color = COLOR_GREEN;
            } else if (!strcmp(currMap.map[i][j].c_str(), WATER.c_str())) {
                color = COLOR_CYAN;
            } else if (!strcmp(currMap.map[i][j].c_str(), CNTR.c_str()) || !strcmp(currMap.map[i][j].c_str(), PKMART.c_str())) {
                color = COLOR_MAGENTA;
            } else if (!strcmp(currMap.map[i][j].c_str(), "@")) {
                color = COLOR_WHITE;
                bold = 1;
            } else if (!strcmp(currMap.map[i][j].c_str(), HIKER.c_str()) || 
            !strcmp(currMap.map[i][j].c_str(), RIVAL.c_str()) || 
            !strcmp(currMap.map[i][j].c_str(), PACER.c_str()) || 
            !strcmp(currMap.map[i][j].c_str(), WANDERER.c_str()) || 
            !strcmp(currMap.map[i][j].c_str(), SENTRY.c_str()) || 
            !strcmp(currMap.map[i][j].c_str(), EXPLORER.c_str())) {
                color = COLOR_RED;
                bold = 1;
            }

            if (bold) attron(A_BOLD);
            attron(COLOR_PAIR(color));
            printw("%s", currMap.map[i][j].c_str());
            attroff(COLOR_PAIR(color));
            if (bold) attroff(A_BOLD);
        }
        //printw("\n");
    }
    printw("(%d, %d)\n", currMap.x - 200, currMap.y - 200);
}

// void PrintExploredWorld() {
//     clear();

//     int adjust_i = 0;
//     int adjust_j = 0;
//     for (int i = 0; i < ROWS + adjust_i; i++) {
//         for (int j = 0; j < COLUMNS + adjust_j; j++) {
//             if (worldMap[i][j]) {
//                 printw("X");
//             } else {
//                 printw(" ");
//             }
//         }
//     }
//     refresh();

//     int command = getch();
//     if (command == 'q') return;
// }


std::string FindTerrain() {
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
    delete Player;
    for (int i = 0; i < WORLDROWS; i++) {
        for (int j = 0; j < WORLDCOLUMNS; j++) {
            free(worldMap[i][j]);
            worldMap[i][j] = NULL;
        }
    }
}

static int32_t path_cmp(const void *key, const void *with) {
  return ((path *) key)->cost - ((path *) with)->cost;
}

static int32_t npc_turn_cmp(const void *key, const void *with) {
  return ((NPCs *) key)->turnTime - ((NPCs *) with)->turnTime;
}

static void Dijkstra(struct map *map, npc npcType, int playerX, int playerY){
    path npcPath[ROWS][COLUMNS], *npcP;
    heap_t h;

    int weights[ROWS][COLUMNS];
    if (npcType == hikerNPC) {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                weights[i][j] = map->hikerWeights[i][j];
            }
        }
    } else {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                weights[i][j] = map->othersWeights[i][j];
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
            if(weights[i][j] != SHRT_MAX){
                npcPath[i][j].hn = heap_insert(&h, &npcPath[i][j]);
            }
        }
    }

    while ((npcP = (path *) heap_remove_min(&h))){
        npcP->hn = NULL;

        // Check all surrounding of the minimum on heap
        for(int i = -1; i <= 1; i++){
            for(int j = -1; j <= 1; j++){
                //Check if going out of bounds
                if (npcP->x + i > ROWS - 1 || npcP->y + j > COLUMNS - 1) continue;
                if (npcP->x > ROWS - 1 || npcP->y > COLUMNS - 1) continue;
                path current = npcPath[npcP->x + i][npcP->y + j]; 
                int centerWeight = weights[npcP->x][npcP->y];
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
    //             printw("   ");
    //             continue;
    //         }
    //         if (i == playerX && j == playerY) {
    //             printw("%s%2d%s ",, npcPath[i][j].cost % 100,);
    //             continue;
    //         }
    //         printw("%2d ", npcPath[i][j].cost % 100);
    //     }
    //     printw("\n");
    // }
    
}