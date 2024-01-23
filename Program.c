#include <stdio.h>

//Prototypes
void GenerateMap();
void PrintMap();

//Global variables
#define ROWS 21
#define COLUMNS 80
char map[ROWS][COLUMNS];

int main(int argc, char *argv[]) {
    GenerateMap();
    PrintMap();
}

void GenerateMap() {
    //Fill with blank space
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            map[i][j] = ' ';
        }
    }

    //Make border 
    for (int i = 0; i < COLUMNS; i++) {
        map[0][i] = '%';
    }
    for (int i = 1; i < ROWS - 1; i++) {
        map[i][0] = '%';
        map[i][COLUMNS - 1] = '%';
    }
    for (int i = 0; i < COLUMNS; i++) {
        map[ROWS - 1][i] = '%';
    }
}

void PrintMap() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            printf("%c", map[i][j]);
        }
        printf("\n");
    }
}