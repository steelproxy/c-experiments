#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>

typedef struct cellGrid
{
    bool* grid;
    int maxY;
    int maxX;

} CellGrid;

typedef struct coord
{
    int x;
    int y;
} Coord;

void DrawGrid(CellGrid *outGrid)
{
    clear();
    for(int index = 0; index < (outGrid->maxY * outGrid->maxX); index++)
    {
            if(index != 0 && index % (outGrid->maxX - 1) == 0)
            {
                waddch(stdscr, '\n');
            }
            if(outGrid->grid[index] == true)
            {
                wattron(stdscr, A_REVERSE);
            }
            waddch(stdscr, ' ');
            wattroff(stdscr, A_REVERSE);
    }

    refresh();
}

bool GetCellByCoord(CellGrid *targetGrid, int y, int x)
{
    int offset = (y * targetGrid->maxX) + x;
    return targetGrid->grid[offset];
}

Coord GetCellCoord(CellGrid *targetGrid, int offset)
{
    int rows = targetGrid->maxY;
    int cols = targetGrid->maxX;
    int y;
    int or_offset = offset;
    for(y = 0; y < rows; y++){
        if(y * cols <= offset );
        else
         break;
    }

    return (Coord){(offset % cols), y - 1};
}

void NextGeneration(CellGrid *outGrid)
{
    /*for(int index = 0; index < maxIndex; index++)
    {

    }*/
}

int main(int argc, char** argv)
{
    // start curses
    initscr();
    noecho();
    curs_set(0);

    // get terminal dimensions
    int termMaxY, termMaxX;
    getmaxyx(stdscr, termMaxY, termMaxX);

    // create grid
    CellGrid conwayGrid;
    conwayGrid.grid = malloc(sizeof(char) * termMaxY * termMaxX);
    conwayGrid.maxY = termMaxY;
    conwayGrid.maxX = termMaxX;

    // populate
    for(int index = 0; index < (termMaxY * termMaxX); index++)
    {
        if(index % 2 == 0)
        {
            conwayGrid.grid[index] = true;
        }
        else
        {
            conwayGrid.grid[index] = false;
        }
    }

    time_t timeMan;
    srand((unsigned int) time(&timeMan));
    for(int index = 0;; index++)
    {
        /*DrawGrid(&conwayGrid);
        for(int index = 0; index < (termMaxY * termMaxX); index++)
        {
            conwayGrid.grid[index] = (rand() % 2);
        }*/

        getch();
        clear();
        Coord result = GetCellCoord(&conwayGrid, index);
        wprintw(stdscr, "index: %d, coord: %d, %d: %s", index, result.x, result.y, conwayGrid.grid[index] ? "true" : "false");
        refresh();
    }

    return 0;
}