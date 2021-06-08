/**
 * @file main.c
 * @author Yong In Kim (you@domain.com)
 * @brief '2048' for the console game.
 * @version 0.1
 * @date 2021-05-29
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'

// REMOVE THIS
void create_test_case(int **, int);

int **init_board(int);
void start(int **, int);
void print_board(int **, int);
int move_blocks(int **, int, int *, char);
void mergeBlocks(int **, int, int *, int *, int *, char);
int checkGameOver(int **, int);
void addNewNumber(int **, int, int);
int getBlankCount(int **, int);
char get_keyboard_input();
void gotoxy(int, int);

// TODO : Add user interface

int main()
{
    int **board;
    const int boardSize = 5; // 5 x 5 board

    board = init_board(boardSize);
    start(board, boardSize);

    printf("fin.\n");
    return 0;
}

int **init_board(int size)
{
    const int generate_number_max = 2;
    int **board = malloc(size * sizeof(int *));
    int count;

    for (int i = 0; i < size; i++)
    {
        board[i] = malloc(size * sizeof(int));
        for (int j = 0; j < size; j++)
        {
            board[i][j] = 0;
        }
    }
    // Places numbers in random locations.
    for (int i = 0; i < generate_number_max; i++)
    {
        int pos = (rand() % size * size);
        count = 0;
        for (int j = 0; j < size * size; j++)
        {
            if (pos == count)
            {
                int start_number = (rand() & 1) ? 2 : 4;
                board[j / size][j % size] = start_number;
            }
            count++;
        }
    }

    // REMOVE THIS!!!!
    // create_test_case(board, size);

    return board;
}

// REMOVE THIS
void create_test_case(int **board, int size)
{
    const int test_case[16] = {0, 0, 0, 0, 2, 0, 0, 4, 0, 0, 0, 2, 0, 0, 0, 8};
    int count = 0;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            board[i][j] = test_case[count];
            count++;
        }
    }
}

void start(int **board, int size)
{
    printf("\e[1;1H\e[2J"); // Clear Console
    int score = 0;
    int comboCount, combo = 0;
    int game_over = FALSE;
    int moveResult, moveCount, blankAmount;
    char key;

    do
    {
        comboCount = 0;

        gotoxy(0, 0);
        print_board(board, size);
        // Print score
        gotoxy(30, 3);
        printf("Your score: %d", score);

        game_over = checkGameOver(board, size);
        if (game_over)
            break;
        gotoxy(0, 7);
        printf("key: ");
        key = get_keyboard_input();

        if (key == 'q')
            game_over = TRUE;

        // Move blocks
        moveResult = 0;
        moveCount = 0;

        while (!moveResult)
        {
            moveResult = move_blocks(board, size, &moveCount, key);
        }
        mergeBlocks(board, size, &moveCount, &score, &comboCount, key);
        moveResult = move_blocks(board, size, &moveCount, key);

        // Move completed but no merge
        if (comboCount == 0)
        {
            gotoxy(30, 4);
            printf("                        "); // Remove combo count on console.
            combo = 0;
        }
        // Move completed and success to merge
        // Add new number in board.
        if (moveCount != 0)
        {
            // printf("Add new Number!\n");
            blankAmount = getBlankCount(board, size);
            addNewNumber(board, size, blankAmount);
            gotoxy(30, 4);
            if (comboCount != 0)
            {
                combo += comboCount;
                printf("%d Combo !", combo);
            }
        }

    } while (!game_over);

    printf("Game Finished!!");
}

/**
 * Print the game board to the console.
 * @param board Game board.
 * @param size Board size.
 */
void print_board(int **board, int size)
{
    for (int i = 0; i < size; i++)
    {
        // printf("|----|----|----|----|\n");
        for (int j = 0; j < size; j++)
        {
            char buffer[10];
            sprintf(buffer, "%d", board[i][j]);
            int str_length = (int)strlen(buffer);
            int space = 4 - str_length;
            if (board[i][j] == 0)
            {
                // printf("|   x");
                printf("    .");
            }
            else
            {
                // printf("|%*d", space + str_length, board[i][j]);
                printf(" %*d", space + str_length, board[i][j]);
            }
        }
        // printf("|\n");
        printf(" \n");
    }
    // printf("|----|----|----|----|\n");
}

/**
 * @brief s
 * @param board Game board
 * @param size Board size
 * @param moveCount
 * @param direction
 * @return int
 */
int move_blocks(int **board, int size, int *moveCount, char direction)
{
    int done = TRUE;
    switch (direction)
    {
    case UP:
        for (int i = 0; i < size - 1; i++)
        {
            for (int j = 0; j < size; j++)
            {
                if (board[i][j] == 0 && board[i + 1][j] != 0)
                {
                    done = FALSE;
                    board[i][j] = board[i + 1][j];
                    board[i + 1][j] = 0;
                    *moveCount += 1;
                }
            }
        }
        break;
    case DOWN:
        for (int i = size - 1; i > 0; i--)
        {
            for (int j = size - 1; j >= 0; j--)
            {
                if (board[i][j] == 0 && board[i - 1][j] != 0)
                {
                    done = FALSE;
                    board[i][j] = board[i - 1][j];
                    board[i - 1][j] = 0;
                    *moveCount += 1;
                }
            }
        }
        break;
    case LEFT:
        for (int i = 0; i < size - 1; i++)
        {
            for (int j = 0; j < size; j++)
            {
                if (board[j][i] == 0 && board[j][i + 1] != 0)
                {
                    done = FALSE;
                    board[j][i] = board[j][i + 1];
                    board[j][i + 1] = 0;
                    *moveCount += 1;
                }
            }
        }
        break;
    case RIGHT:
        for (int i = size - 1; i > 0; i--)
        {
            for (int j = size - 1; j >= 0; j--)
            {
                if (board[j][i] == 0 && board[j][i - 1] != 0)
                {
                    done = FALSE;
                    board[j][i] = board[j][i - 1];
                    board[j][i - 1] = 0;
                    *moveCount += 1;
                }
            }
        }
        break;
    default:
        break;
    }

    return done;
}

/**
 * @brief Merge if there is the same number in the direction you want to move.
 *
 * @param board Game board
 * @param size Size of board
 * @param moveCount Number of times the block has moved
 * @param direction The direction to merge.
 */
void mergeBlocks(int **board, int size, int *moveCount, int *score, int *comboCount, char direction)
{
    switch (direction)
    {
    case UP:
        for (int i = 0; i < size - 1; i++)
        {
            for (int j = 0; j < size; j++)
            {
                if (board[i][j] != 0 && board[i][j] == board[i + 1][j])
                {
                    board[i][j] = board[i][j] + board[i + 1][j];
                    board[i + 1][j] = 0;
                    *score += board[i][j];
                    *moveCount += 1;
                    *comboCount += 1;
                }
            }
        }
        break;
    case DOWN:
        for (int i = size - 1; i > 0; i--)
        {
            for (int j = size - 1; j >= 0; j--)
            {
                if (board[i][j] != 0 && board[i][j] == board[i - 1][j])
                {
                    board[i][j] = board[i][j] + board[i - 1][j];
                    board[i - 1][j] = 0;
                    *score += board[i][j];
                    *moveCount += 1;
                    *comboCount += 1;
                }
            }
        }
        break;
    case LEFT:
        for (int i = 0; i < size - 1; i++)
        {
            for (int j = 0; j < size; j++)
            {
                if (board[j][i] != 0 && board[j][i] == board[j][i + 1])
                {
                    board[j][i] = board[j][i] + board[j][i + 1];
                    board[j][i + 1] = 0;
                    *score += board[j][i];
                    *moveCount += 1;
                    *comboCount += 1;
                }
            }
        }
    case RIGHT:
        for (int i = size - 1; i > 0; i--)
        {
            for (int j = size - 1; j >= 0; j--)
            {
                if (board[j][i] != 0 && board[j][i] == board[j][i - 1])
                {
                    board[j][i] = board[j][i] + board[j][i - 1];
                    board[j][i - 1] = 0;
                    *score += board[j][i];
                    *moveCount += 1;
                    *comboCount += 1;
                }
            }
        }
    default:
        break;
    }
}

int checkGameOver(int **board, int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (board[i][j] == 32)
                return TRUE;
        }
    }
    return FALSE;
}

void addNewNumber(int **board, int size, int blankAmount)
{
    int randomPos = (rand() % blankAmount);
    int count = -1;

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (board[i][j] == 0)
                count++;
            if (board[i][j] == 0 && count == randomPos)
            {
                int newNumber = (rand() & 1) ? 2 : 4;
                board[i][j] = newNumber;
            }
        }
    }
}

int getBlankCount(int **board, int size)
{
    int count = 0;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (board[i][j] == 0)
                count++;
        }
    }

    return count;
}

char get_keyboard_input()
{
    char key;
    scanf(" %c", &key);

    if (key < 'a')
    {
        key -= 32;
    }

    return key;
}

/**
 * @brief
 *
 * @param x
 * @param y
 */
void gotoxy(int x, int y)
{
    printf("\033[%d;%df", y, x);
    fflush(stdout);
}