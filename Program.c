#include <stdio.h>

//Prototypes
void GenerateMap();
void PrintMap();

//Global variables
#define ROWS 21
#define COLUMNS 80
char* map[ROWS][COLUMNS];

//Colors
#define BLACK   "\x1b[30m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define DARKGRN "\x1b[32;2;0;128;0m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"
#define RESET   "\x1b[0m"
#define GREY    "\x1b[90m"
#define BROWN   "\x1b[38;2;139;69;19m"

/*
Percent signs (%) represent
boulders and mountains. Carrots (ˆ) represent tree and forests. Hashes (#) are roads. Cs and Ms are Pokemon ´
Centers and Pokemarts (buildings), respectively. Colons (:) are long grass and periods (.) are clearings. 
*/
//Elements
char* MTN = GREY "%" RESET;
char* TREE = GREY "^" RESET;
char* ROAD = YELLOW "#" RESET;
char* CNTR = MAGENTA "C" RESET;
char* PKMART = MAGENTA "M" RESET;
char* LNGR = DARKGRN ":" RESET;
char* CLRNG = GREEN "." RESET;
char* WATER = BLUE "~" RESET;


int main(int argc, char *argv[]) {
    srand(time());
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
    int xlength = 5;
    int ylength = 5;
    int blankSpace = (COLUMNS - 2) * (ROWS - 2);

}

void PrintMap() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            printf("%s", map[i][j]);
        }
        printf("\n");
    }
}