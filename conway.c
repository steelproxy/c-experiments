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
    int   generation;
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
    wmove(stdscr, 0, 0);
    for (int index = 0; index < maxIndex; index++) {
        bool *cell = &(outGrid->grid[index]);
        if (*cell == true) {
            wattron(stdscr, A_REVERSE);
        }
        waddch(stdscr, ' ');
        wattroff(stdscr, A_REVERSE);
    }
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

    return (Coord){(offset % (cols)), y - 1};
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
    int        rows       = outGrid->maxY;
    int        cols       = outGrid->maxX;
    int        maxIndex   = rows * cols;
    static int generation = 0;

    // allocate new grid
    bool *     newGrid    = malloc(sizeof(bool) * maxIndex);

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

    outGrid->generation++;
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
    targetGrid->generation = 0;
}

void ResizeGrid(CellGrid *targetGrid, int newMaxX, int newMaxY)
{
    free(targetGrid->grid);
    targetGrid->grid = malloc(sizeof(bool) * newMaxY * newMaxX);
    targetGrid->maxX = newMaxX;
    targetGrid->maxY = newMaxY;

    GenerateGrid(targetGrid);
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

    // create grid dimensions
    int      rows = termMaxY - 1;
    int      cols = termMaxX;

    // create grid
    CellGrid conwayGrid;
    conwayGrid.grid       = malloc(sizeof(char) * rows * cols);
    conwayGrid.maxY       = rows;
    conwayGrid.maxX       = cols;
    conwayGrid.generation = 0;

    // populate grid
    time_t timeMan;
    srand((unsigned int)atoi(argv[1]));
    GenerateGrid(&conwayGrid);

    for (;;) {
        DrawGrid(&conwayGrid);
        wmove(stdscr, termMaxY - 1, 0);
        wprintw(stdscr, "generation: %d", conwayGrid.generation);
        refresh();

        usleep(250000);
        NextGeneration(&conwayGrid);

        int keyBuf = getch();
        switch (keyBuf) {
        case 'r':
            srand((unsigned int)time(&timeMan));
            for (int index = 0; index < (rows * cols); index++) {
                int random             = (rand() % 2);
                conwayGrid.grid[index] = random;
            }
            conwayGrid.generation = 0;
            break;

        case KEY_RESIZE:
            getmaxyx(stdscr, termMaxY, termMaxX);
            rows = termMaxY - 1;
            cols = termMaxX;
            ResizeGrid(&conwayGrid, cols, rows);
            break;

        case 'q':
            free(conwayGrid.grid);
            SignalHandler(0);
            break;

        case 't':
            goto test;
        back:
            timeout(0);
            break;
        }
    }
test:;

    int cursor = 0;
    int key;
    timeout(-1);
    keypad(stdscr, true);
    while (1) {
        DrawGrid(&conwayGrid);
        Coord test = GetCellCoord(&conwayGrid, cursor);
        wmove(stdscr, termMaxY - 1, 0);
        wprintw(stdscr,
                "index: %d, coord: (%d,%d), alive: %d, neigbors: %d, "
                "generation: %d",
                cursor, test.x, test.y, conwayGrid.grid[cursor],
                CountNeighbors(&conwayGrid, test), conwayGrid.generation);
        wmove(stdscr, test.y, test.x);
        waddch(stdscr, 'X');
        refresh();

        key = getch();
        if (key == KEY_LEFT && cursor > 0) {
            cursor--;
        } else if (key == KEY_RIGHT &&
                   cursor < conwayGrid.maxX * conwayGrid.maxY) {
            cursor++;
        } else if (key == 'q') {
            goto back;
        } else if (key == KEY_RESIZE) {
            getmaxyx(stdscr, termMaxY, termMaxX);
            rows = termMaxY - 1;
            cols = termMaxX;
            ResizeGrid(&conwayGrid, cols, rows);
        }
    }

    return 0;
}