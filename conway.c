#include <assert.h>
#include <curses.h>
#include <malloc.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct cellGrid {
    bool *grid;
    int   maxY;
    int   maxX;

} CellGrid;

typedef struct coord {
    int x;
    int y;
} Coord;

void DrawGrid(CellGrid *outGrid)
{
    int rows     = outGrid->maxY;
    int cols     = outGrid->maxX;
    int maxIndex = rows * cols;

    clear();
    for (int index = 0; index < maxIndex; index++) {
        bool *cell = &(outGrid->grid[index]);
        if (index != 0 && index % (cols - 1) == 0) {
            waddch(stdscr, '\n');
        }
        if (*cell == true) {
            wattron(stdscr, A_REVERSE);
        }
        waddch(stdscr, ' ');
        wattroff(stdscr, A_REVERSE);
    }

    refresh();
}

bool *GetCellByCoord(CellGrid *targetGrid, int x, int y)
{
    int offset = (y * targetGrid->maxX) + x;
    return &(targetGrid->grid[offset]);
}

Coord GetCellCoord(CellGrid *targetGrid, int offset)
{
    int rows = targetGrid->maxY;
    int cols = targetGrid->maxX;
    int y;
    for (y = 0; y < rows; y++) {
        if (y * cols <= offset)
            ;
        else
            break;
    }

    return (Coord){(offset % cols), y - 1};
}

int GetDistance(Coord coord1, Coord coord2)
{
    int x1 = coord1.x;
    int x2 = coord2.x;
    int y1 = coord1.y;
    int y2 = coord2.y;
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

int CountNeighbors(CellGrid *targetGrid, Coord targetCell)
{
    Coord start;
    Coord end;

    // calculate search grid
    start.x = targetCell.x - 1;
    start.y = targetCell.y - 1;
    end.x   = targetCell.x + 1;
    end.y   = targetCell.y + 1;

    // handle edge exceptions
    if (start.x < 0) {
        start.x = 0;
    }
    if (start.y < 0) {
        start.y = 0;
    }
    if (end.x > targetGrid->maxX) {
        end.x = targetGrid->maxX;
    }
    if (end.y > targetGrid->maxY) {
        end.y = targetGrid->maxY;
    }

    // subtract self from neighbor count if necessary
    bool *orig      = GetCellByCoord(targetGrid, targetCell.x, targetCell.y);
    int   neighbors = 0;
    if (*orig) {
        neighbors--;
    }

    for (int y = start.y; y <= end.y; y++) {
        for (int x = start.x; x <= end.x; x++) {
            bool *cell = GetCellByCoord(targetGrid, x, y);
            if (*cell == true) {
                neighbors++;
            }
        }
    }

    return neighbors;
}

void NextGeneration(CellGrid *outGrid)
{
    int   rows     = outGrid->maxY;
    int   cols     = outGrid->maxX;
    int   maxIndex = rows * cols;

    // allocate new grid
    bool *newGrid  = malloc(sizeof(bool) * maxIndex);

    for (int index = 0; index < maxIndex; index++) {
        bool *cell      = &(outGrid->grid[index]);
        bool *newCell   = &(newGrid[index]);

        // copy old cells to new grid
        *newCell        = *cell;

        Coord cellCoord = GetCellCoord(outGrid, index);
        int   neighbors = CountNeighbors(outGrid, cellCoord);
        if (neighbors < 2) {
            // cell dies from underpopulation
            *newCell = false;
            continue;
        }
        if (neighbors == 2) {
            // let it live
            continue;
        }
        if (neighbors == 3) {
            *newCell = true;
            continue;
        }
        if (neighbors > 3) {
            // death from overpopulation
            *newCell = false;
            continue;
        }
    }

    // swap new grid
    free(outGrid->grid);
    outGrid->grid = newGrid;
}

void SignalHandler(int sig)
{
    // end curses mode and exit
    endwin();
    exit(0);
}

void GenerateGrid(CellGrid *targetGrid)
{
    int rows     = targetGrid->maxY;
    int cols     = targetGrid->maxX;
    int maxIndex = rows * cols;
    for (int index = 0; index < maxIndex; index++) {
        int random              = (rand() % 2);
        targetGrid->grid[index] = random;
    }
}

int main(int argc, char **argv)
{
    // handle seed argument
    if (argc < 2) {
        printf("Invalid number of arguments!\nUsage:    %s  [seed]\n", argv[0]);
        return 1;
    }

    // start curses
    initscr();
    noecho();
    curs_set(0);
    timeout(0);

    // handle signals
    signal(SIGINT, SignalHandler);

    // get terminal dimensions
    int termMaxY, termMaxX;
    getmaxyx(stdscr, termMaxY, termMaxX);

    // create grid
    CellGrid conwayGrid;
    conwayGrid.grid = malloc(sizeof(char) * termMaxY * termMaxX);
    conwayGrid.maxY = termMaxY;
    conwayGrid.maxX = termMaxX;

    // populate grid
    time_t timeMan;
    srand((unsigned int)atoi(argv[1]));
    GenerateGrid(&conwayGrid);

    for (;;) {
        DrawGrid(&conwayGrid);
        usleep(50000);
        NextGeneration(&conwayGrid);
        refresh();

        int keyBuf = getch();
        switch (keyBuf) {
        case 'r':
            srand((unsigned int)time(&timeMan));
            for (int index = 0; index < (termMaxY * termMaxX); index++) {
                int random             = (rand() % 2);
                conwayGrid.grid[index] = random;
            }
            break;

        case KEY_RESIZE:
            free(conwayGrid.grid);

            getmaxyx(stdscr, termMaxY, termMaxX);
            conwayGrid.grid = malloc(sizeof(bool) * termMaxY * termMaxX);
            conwayGrid.maxX = termMaxX;
            conwayGrid.maxY = termMaxY;

            srand((unsigned int)time(&timeMan));
            GenerateGrid(&conwayGrid);
            break;

        case 'q':
            free(conwayGrid.grid);
            SignalHandler(0);
        }
    }

    return 0;
}