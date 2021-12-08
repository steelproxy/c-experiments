#include <curses.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOARD_X 10
#define BOARD_Y 20

typedef enum blockType {
    I_BLOCK = 0,
    J_BLOCK,
    L_BLOCK,
    O_BLOCK,
    S_BLOCK,
    T_BLOCK,
    Z_BLOCK
} BlockType;

typedef enum blockColor {
    NONE = 0,
    CYAN,
    BLUE,
    ORANGE,
    YELLOW,
    GREEN,
    PURPLE,
    RED
} BlockColor;

typedef enum blockRotation { NORMAL = 0, RIGHT, FLIP, LEFT } BlockRotation;

typedef struct blockPos {
    int x;
    int y;
} BlockPos;

typedef struct block {
    BlockType     type;
    BlockColor    color;
    BlockPos      pos;
    BlockRotation rotation;
} Block;

typedef struct tetrisBoard {
    Block      falling;
    Block      next;
    BlockColor board[BOARD_Y][BOARD_X];
} TetrisBoard;

const bool blockBitmaps[8][4][2] = {
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}},
    {{1, 0}, {1, 0}, {1, 0}, {1, 0}}, // I_BLOCK
    {{1, 1}, {1, 0}, {1, 0}, {0, 0}}, // J_BLOCK
    {{1, 0}, {1, 0}, {1, 1}, {0, 0}}, // L_BLOCK
    {{1, 1}, {1, 1}, {0, 0}, {0, 0}}, // O_BLOCK
    {{1, 0}, {1, 1}, {0, 1}, {0, 0}}, // S_BLOCK
    {{1, 0}, {1, 1}, {1, 0}, {0, 0}}, // T_BLOCK
    {{0, 1}, {1, 1}, {1, 0}, {0, 0}}  // Z_BLOCK
};

void DrawBlock(WINDOW *boardWin, Block *blockToDraw)
{
    wattron(boardWin, COLOR_PAIR(blockToDraw->color));

    const int y_or            = blockToDraw->pos.y + 1;
    const int x_or            = blockToDraw->pos.x + 1;

    bool      copy[4][4]      = {0};
    bool      transpose[4][4] = {0};

    wmove(stdscr, y_or, x_or);

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 2; x++) {
            copy[y][x] = blockBitmaps[blockToDraw->type][y][x];
        }
    }

    for (BlockRotation transCount = NORMAL; transCount < blockToDraw->rotation;
         transCount++) {

        // transpose
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                transpose[x][y] = copy[y][x];
            }
        }

        // flip
        bool temp[4][4] = {0};
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                temp[y][x] = transpose[y][3 - x];
            }
        }

        memcpy(transpose, temp, (sizeof(bool) * 4 * 4));
        memcpy(copy, transpose, (sizeof(bool) * 4 * 4));
    }

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            wmove(boardWin, y_or + y, x_or + x);
            if (copy[y][x]) {
                waddch(boardWin, ACS_CKBOARD);
            } else {
                wmove(boardWin, y_or + y, x_or + x + 1);
            }
            wrefresh(boardWin);
        }
    }

    wattroff(boardWin, COLOR_PAIR(blockToDraw->color));

    wrefresh(boardWin);
}

void DrawTetrisBoard(WINDOW *boardWin, TetrisBoard *boardToDraw)
{
    wclear(boardWin);
    wborder(boardWin, '|', '|', '-', '-', ACS_ULCORNER, ACS_URCORNER,
            ACS_LLCORNER, ACS_LRCORNER);

    const int startY = 1;
    const int startX = 1;
    for (int y = 0; y < BOARD_Y; y++) {
        wmove(boardWin, startY + y, startX);
        for (int x = 0; x < BOARD_X; x++) {
            BlockColor *cell = &boardToDraw->board[y][x];
            if (*cell == 0) {
                waddch(boardWin, ' ');
                wrefresh(boardWin);
                continue;
            }
            wattron(boardWin, COLOR_PAIR(*cell));
            waddch(boardWin, ACS_CKBOARD);
            wattroff(boardWin, COLOR_PAIR(*cell));
            wrefresh(boardWin);
        }
    }
    DrawBlock(boardWin, &boardToDraw->falling);

    wrefresh(boardWin);
}

void LineClear(TetrisBoard *boardToClearLines)
{
    for (int boardY = 0; boardY < BOARD_Y; boardY++) {
        bool clearLine = true;
        for (int boardX = 0; boardX < BOARD_X; boardX++) {
            BlockColor *cell = &boardToClearLines->board[boardY][boardX];
            if (*cell == NONE) {
                clearLine = false;
                break;
            }
        }
        if (clearLine) {
            for (int clearX = 0; clearX < BOARD_X; clearX++) {
                boardToClearLines->board[boardY][clearX] = NONE;
            }
        }
    }
}

void BlockCollide(TetrisBoard *boardToCheck)
{
    int startX = boardToCheck->falling.pos.x;
    int startY = boardToCheck->falling.pos.y;
    int endX   = startX + 4;
    int endY   = startY + 4;
    for (int cellToCheck = startX; cellToCheck < endX; cellToCheck++) {
        BlockColor *currentCell = &boardToCheck->board[endY][cellToCheck];
        if (*currentCell != NONE) {
            return;
        }
    }
    boardToCheck->falling.pos.y++;

    // cast falling block onto board, check for collisions
}

void PrintCenter(WINDOW *outwin, char *str)
{
    int winMaxY;
    int winMaxX;
    getmaxyx(outwin, winMaxY, winMaxX);

    wmove(outwin, winMaxY / 2, (winMaxX / 2) - (strlen(str) / 2));
    waddstr(outwin, str);
    wrefresh(outwin);
}

void SignalHandler(int sig)
{
    endwin();
    exit(sig);
}

int main(int argc, char **argv)
{
    // start curses
    initscr();

    // setup curses
    keypad(stdscr, true); // enable arrow keys
    curs_set(0);          // hide cursor
    start_color();        // enable colors
    noecho();

    // set curses color pairs
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_RED, COLOR_BLACK);

    // handle signals
    signal(SIGINT, SignalHandler);

    // get term dimensions
    int termRows;
    int termCols;
    getmaxyx(stdscr, termRows, termCols);

    // make next block window
    WINDOW *w_nextBlock;
    w_nextBlock = newwin(termRows / 2, termCols / 3, 0, 0);
    wborder(w_nextBlock, '|', '|', '-', '-', ACS_ULCORNER, ACS_URCORNER,
            ACS_LLCORNER, ACS_LRCORNER);

    // make score count window
    WINDOW *w_scoreCount;
    w_scoreCount = newwin((termRows / 2) + 1, termCols / 3, termRows / 2, 0);
    wborder(w_scoreCount, '|', '|', '-', '-', ACS_ULCORNER, ACS_URCORNER,
            ACS_LLCORNER, ACS_LRCORNER);

    // make tetris board window
    WINDOW *w_tetrisBoard;
    w_tetrisBoard = newwin(22, 12, 0, (termCols / 3) + 1);
    wborder(w_tetrisBoard, '|', '|', '-', '-', ACS_ULCORNER, ACS_URCORNER,
            ACS_LLCORNER, ACS_LRCORNER);
    keypad(w_tetrisBoard, true);

    // refresh windows
    wrefresh(w_nextBlock);
    wrefresh(w_scoreCount);
    wrefresh(w_tetrisBoard);

    // start menu
    // PrintCenter(w_nextBlock, "Press any key to start...");
    // wrefresh(w_nextBlock);
    // wgetch(w_nextBlock);

    TetrisBoard testBoard;
    memset(testBoard.board, 0, (sizeof(BlockColor) * BOARD_Y * BOARD_X));
    // testBoard.board[19][0] = CYAN;
    // testBoard.board[19][1] = RED;
    // testBoard.board[0][5]  = YELLOW;

    Block test;
    test.color    = YELLOW;
    test.type     = J_BLOCK;
    test.rotation = FLIP;
    test.pos      = (BlockPos){5, 0};

    for (int testX = 0; testX < BOARD_X; testX++) {
        testBoard.board[10][testX] = COLOR_CYAN;
    }

    testBoard.falling = test;

    for (;;) {
        // DrawTetrisBoard(w_tetrisBoard, &testBoard);
        // wgetch(w_tetrisBoard);
        int keycode;
        DrawTetrisBoard(w_tetrisBoard, &testBoard);

        switch (keycode = wgetch(w_tetrisBoard)) {
        case KEY_LEFT:
            if (testBoard.falling.pos.x > -3) {
                testBoard.falling.pos.x--;
            }
            break;

        case KEY_RIGHT:
            if (testBoard.falling.pos.x < BOARD_X - 4) {
                testBoard.falling.pos.x++;
            }
            break;

        case 'z':
            testBoard.falling.rotation++;
            break;

        default:
            break;
        }

        BlockCollide(&testBoard);
        LineClear(&testBoard);
    }

    return 0;
}