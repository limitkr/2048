/**
 * @file main.c
 * @author Yong In Kim (2018-17174)
 * @brief '2048' for the console game.
 * @version 0.1
 * @date 2021-05-29
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define CLEAR_CONSOLE() printf("\e[1;1H\e[2J")

#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'

#define MAX_BLOCK_NUMBER 2048
#define GAME_BOARD_SIZE 5
#define TIME 600 // Seconds

typedef enum _GameStatus
{
    GAME_CLEAR,
    GAME_OVER,
    GAME_IN_PROGRESS
} GameStatus;

typedef enum _boolean
{
    FALSE,
    TRUE
} boolean;

typedef struct timer
{
    int set_time;
    int current_time;
    boolean stop;
} Timer, *p_Timer;

typedef struct game
{
    int **board;
    int board_size;
    int score;
    int combo;
    int max_combo;
    int movement_count;
    GameStatus game_status;
} Game, *p_Game;

struct score
{
    char *user_name;
    char *game_status;
    int score;
    int number_of_moves;
    int elapsed_time;
    int max_combo;
};

const char *FILE_PATH = "ranking.txt";

void print_title();
void print_main_menu_component();
void print_how_to();
int compare1(const void *, const void *);
int compare2(const void *, const void *);
int get_data_from_file();
void print_ranking();
void press_key_to_continue();
int file_exists();
int kbhit();
p_Game create_new_game(p_Game);
p_Timer stopWatch(p_Timer);
void start_game(p_Game);
void add_score_to_rank(p_Game, p_Timer, const char *);
void draw_interface(p_Game);
void processing(p_Game, char);
int move_blocks(p_Game, int *, char);
void mergeBlocks(p_Game, int *, int *, char);
boolean check_game_clear(p_Game);
boolean check_game_over(p_Game);
void add_new_number(p_Game, int);
int get_blank_count(p_Game);
// char get_keyboard_input();
void gotoxy(int, int);

int main(void)
{
    char select = '0';
    p_Game game = malloc(sizeof(Game));

    do
    {
        print_main_menu_component();

        if (kbhit() != 0)
        {
            select = getchar();
            switch (select)
            {
            case '1':
                game = create_new_game(game);
                start_game(game);
                break;
            case '2':
                print_how_to();
                break;
            case '3':
                print_ranking();
                break;
            default:
                break;
            }
        }
        usleep(100000);
    } while (select != '4');
    free(game);
    return 0;
}

void print_title()
{
    CLEAR_CONSOLE();
    gotoxy(0, 0);
    printf("|------------------------------------------|\n");
    printf("|   222   000     4   888                  |\n");
    printf("|  2   2 0   0   44  8   8   - Created by  |\n");
    printf("|     2  0   0  4 4   888    Yong In Kim   |\n");
    printf("|    2   0   0 44444 8   8                 |\n");
    printf("|  22222  000     4   888    in C          |\n");
    printf("|------------------------------------------|\n");
    printf("\n");
}

void print_main_menu_component()
{
    print_title();
    printf("==========MAIN MENU==========\n\n");
    printf("1. Game Start\n");
    printf("2. How to\n");
    printf("3. Ranking\n");
    printf("4. Exit\n\n");

    printf("Key: ");
}

void print_how_to()
{
    print_title();
    printf("==========HOW TO PLAY?==========\n\n");
    printf("- Use key 'w', 'a', 's', 'd' to move tiles up, left, down, right.\n");
    printf("- Press 'q' to quit.\n\n");
    printf("Press any key to continue...\n");
    press_key_to_continue();
}

int compare1(const void *target1, const void *target2)
{
    struct score *score1 = (struct score *)target1;
    struct score *score2 = (struct score *)target2;
    if (strcmp(score1->game_status, "Lose") > strcmp(score2->game_status, "Clear"))
        return 1;
    if (strcmp(score1->game_status, "Clear") < strcmp(score2->game_status, "Lose"))
        return -1;
    return 0;
}

int compare2(const void *target1, const void *target2)
{
    struct score *score1 = (struct score *)target1;
    struct score *score2 = (struct score *)target2;

    if (!strcmp(score1->game_status, "Clear") && !strcmp(score2->game_status, "Clear"))
    {
        if (score1->elapsed_time > score2->elapsed_time)
            return -1;
        if (score1->elapsed_time < score2->elapsed_time)
            return 1;
        return 0;
    }
    else
    {
        if (score1->score > score2->score)
            return -1;
        if (score1->score < score2->score)
            return 1;
        return 0;
    }
}

int get_data_from_file()
{
    struct score *data;
    int idx = 0;
    char user_name[20];
    char game_status[15];
    char line_buffer[256];
    int number_of_moves, elapsed_time, max_combo, score;

    FILE *fp = fopen(FILE_PATH, "r");
    if (fp == NULL)
    {
        printf("Ranking data not found.\n");
        return FALSE;
    }

    data = malloc(100 * sizeof(struct score));

    while (fgets(line_buffer, sizeof(line_buffer), fp))
    {
        if (fscanf(fp, "%20[^,] %s %d %d %d %d", user_name, game_status, &score, &number_of_moves, &elapsed_time,
                   &max_combo) == 6)
        {
            data[idx].user_name = strdup(user_name);
            data[idx].game_status = strdup(game_status);
            data[idx].score = score;
            data[idx].number_of_moves = number_of_moves;
            data[idx].elapsed_time = elapsed_time;
            data[idx].max_combo = max_combo;
            idx++;
        }
    }

    fclose(fp);

    qsort(data, idx, sizeof *data, compare1);
    // qsort(data, idx, sizeof *data, compare2);
    printf("%-20s %-7s %-7s %-7s %-5s  %-15s\n", "Name", "Status", "Score", "Moves", "Elapsed time", "Max combo");
    for (int i = 0; i < idx; i++)
    {
        int m = (TIME - data[i].elapsed_time) / 60;
        int s = (TIME - data[i].elapsed_time) % 60;
        printf("%-20s %-7s %-7d %-7d %02d:%02d         %-15d\n", data[i].user_name, data[i].game_status, data[i].score,
               data[i].number_of_moves, m, s, data[i].max_combo);
    }
    printf("\n");

    while (--idx >= 0)
    {
        free(data[idx].user_name);
        free(data[idx].game_status);
    }

    free(data);

    return 0;
}

void print_ranking()
{
    print_title();
    printf("==========RANK==========\n\n");
    get_data_from_file();
    printf("Press any key to continue...\n");
    press_key_to_continue();
}

void press_key_to_continue()
{
    while (1)
    {
        if (kbhit() != 0)
        {
            if (getchar())
                break;
        }
        usleep(100000);
    }
}

int file_exists(const char *file_name)
{
    FILE *file;
    if ((file = fopen(file_name, "r")))
    {
        fclose(file);
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief kbhit function for linux, macOS system.
 *
 * @return int
 */
int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

p_Game create_new_game(p_Game game)
{
    const int generate_number_max = 2;
    int **board = malloc(GAME_BOARD_SIZE * sizeof(int *));
    int count;

    for (int i = 0; i < GAME_BOARD_SIZE; i++)
    {
        board[i] = malloc(GAME_BOARD_SIZE * sizeof(int));
        for (int j = 0; j < GAME_BOARD_SIZE; j++)
        {
            board[i][j] = 0;
        }
    }
    // Places numbers in random locations.
    for (int i = 0; i < generate_number_max; i++)
    {
        int pos = (rand() % GAME_BOARD_SIZE * GAME_BOARD_SIZE);
        count = 0;
        for (int j = 0; j < GAME_BOARD_SIZE * GAME_BOARD_SIZE; j++)
        {
            if (pos == count)
            {
                int start_number = (rand() & 1) ? 2 : 4;
                board[j / GAME_BOARD_SIZE][j % GAME_BOARD_SIZE] = start_number;
            }
            count++;
        }
    }
    game->board = board;
    game->board_size = GAME_BOARD_SIZE;
    game->score = 0;
    game->combo = 0;
    game->max_combo = 0;
    game->movement_count = 0;
    game->game_status = GAME_IN_PROGRESS;
    return game;
}

p_Timer stopWatch(p_Timer timer)
{
    int end_time, total_seconds;
    end_time = time(NULL);
    total_seconds = timer->set_time - end_time;
    timer->current_time = total_seconds;

    gotoxy(30, 5);
    printf("Time Remaining: %02d:%02d\n", (total_seconds / 60), (total_seconds % 60));

    if (total_seconds == 0)
        timer->stop = TRUE;
    return timer;
}

void start_game(p_Game game)
{
    int comboCount, moveResult, moveCount, blankAmount;
    char key;

    p_Timer timer = malloc(sizeof(Timer));
    timer->set_time = time(NULL);
    timer->set_time += TIME;
    timer->current_time = timer->set_time;
    timer->stop = FALSE;

    do
    {
        CLEAR_CONSOLE();
        gotoxy(0, 0);

        if (check_game_clear(game))
        {
            draw_interface(game);
            game->game_status = GAME_CLEAR;
        }
        if (check_game_over(game))
        {
            draw_interface(game);
            game->game_status = GAME_OVER;
        }

        timer = stopWatch(timer);

        if (timer->stop == TRUE)
            game->game_status = GAME_OVER;

        draw_interface(game);

        if (kbhit())
        {
            key = getchar();
            processing(game, key);

            if (key == 'q')
                game->game_status = GAME_OVER;
        }
        usleep(100000);
    } while (game->game_status == GAME_IN_PROGRESS);

    gotoxy(30, 7);
    if (game->game_status == GAME_CLEAR)
        printf("CLEAR!!\n");
    if (game->game_status == GAME_OVER)
        printf("GAME OVER!!\n");

    char *user_name = malloc(sizeof(char) * 100);

    gotoxy(0, 13);
    printf("Enter your name: ");
    scanf("%[^\n]s", user_name);

    printf("\n");
    printf("Your score will be updated in the ranking.\n");
    add_score_to_rank(game, timer, user_name);
    printf("back to the main menu after a second...\n");
    sleep(1);

    for (int i = 0; i < GAME_BOARD_SIZE; i++)
    {
        free(game->board[i]);
    }
    free(user_name);
    free(timer);
}

void add_score_to_rank(p_Game game, p_Timer timer, const char *username)
{
    FILE *fp;
    char *game_status;

    if (!file_exists(FILE_PATH))
    {
        fp = fopen(FILE_PATH, "w");
        fprintf(fp, "%-20s %-15s %-15s %-15s %-15s %-15s\n", "Name", "Status", "Score", "Number of moves",
                "Elapsed time", "Max combo");
        fclose(fp);
    }

    game_status = game->game_status == GAME_CLEAR ? "Clear" : "Lose";
    fp = fopen(FILE_PATH, "a");
    fprintf(fp, "%-20s %-15s %-15d %-15d %-15d %-15d\n", username, game_status, game->score, game->movement_count,
            timer->current_time, game->max_combo);

    fclose(fp);
}

/**
 * Print the game board to the console.
 * @param board Game board.
 */
void draw_interface(p_Game game)
{
    gotoxy(0, 0);
    for (int i = 0; i < game->board_size; i++)
    {
        printf("|----|----|----|----|----|\n");
        for (int j = 0; j < game->board_size; j++)
        {
            char buffer[10];
            sprintf(buffer, "%d", game->board[i][j]);
            int str_length = (int)strlen(buffer);
            int space = 4 - str_length;
            if (game->board[i][j] == 0)
            {
                printf("|    ");
            }
            else
            {
                printf("|%*d", space + str_length, game->board[i][j]);
            }
        }
        printf("|\n");
    }
    printf("|----|----|----|----|----|\n");
    gotoxy(30, 2);
    printf("Your score: %d", game->score);
    gotoxy(30, 3);
    if (game->combo != 0)
    {
        printf("%d Combo !", game->combo);
    }
    else
    {
        // Remove combo count on console.
        printf("                        ");
    }
    gotoxy(0, 13);
    printf("Key: ");
}

void processing(p_Game game, char key)
{
    int comboCount, moveResult, moveCount, blankAmount;

    comboCount = 0;
    moveResult = 0;
    moveCount = 0;

    while (!moveResult)
    {
        moveResult = move_blocks(game, &moveCount, key);
    }
    mergeBlocks(game, &moveCount, &comboCount, key);
    moveResult = move_blocks(game, &moveCount, key);
    if (moveCount != 0)
        game->movement_count += 1;
    // Block movement succeeded. However there are no blocks to merge
    if (comboCount == 0)
    {
        game->combo = 0;
    }
    // Block movement succeeded and success to merge blocks
    // Add new number elements in board.
    if (moveCount != 0)
    {
        blankAmount = get_blank_count(game);
        add_new_number(game, blankAmount);
        if (comboCount != 0)
        {
            game->combo += comboCount;
            if (game->max_combo < game->combo)
                game->max_combo = game->combo;
        }
    }
}

int move_blocks(p_Game game, int *moveCount, char direction)
{
    boolean done = TRUE;
    switch (direction)
    {
    case UP:
        for (int i = 0; i < game->board_size - 1; i++)
        {
            for (int j = 0; j < game->board_size; j++)
            {
                if (game->board[i][j] == 0 && game->board[i + 1][j] != 0)
                {
                    done = FALSE;
                    game->board[i][j] = game->board[i + 1][j];
                    game->board[i + 1][j] = 0;
                    *moveCount += 1;
                }
            }
        }
        break;
    case DOWN:
        for (int i = game->board_size - 1; i > 0; i--)
        {
            for (int j = game->board_size - 1; j >= 0; j--)
            {
                if (game->board[i][j] == 0 && game->board[i - 1][j] != 0)
                {
                    done = FALSE;
                    game->board[i][j] = game->board[i - 1][j];
                    game->board[i - 1][j] = 0;
                    *moveCount += 1;
                }
            }
        }
        break;
    case LEFT:
        for (int i = 0; i < game->board_size - 1; i++)
        {
            for (int j = 0; j < game->board_size; j++)
            {
                if (game->board[j][i] == 0 && game->board[j][i + 1] != 0)
                {
                    done = FALSE;
                    game->board[j][i] = game->board[j][i + 1];
                    game->board[j][i + 1] = 0;
                    *moveCount += 1;
                }
            }
        }
        break;
    case RIGHT:
        for (int i = game->board_size - 1; i > 0; i--)
        {
            for (int j = game->board_size - 1; j >= 0; j--)
            {
                if (game->board[j][i] == 0 && game->board[j][i - 1] != 0)
                {
                    done = FALSE;
                    game->board[j][i] = game->board[j][i - 1];
                    game->board[j][i - 1] = 0;
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

void mergeBlocks(p_Game game, int *moveCount, int *comboCount, char direction)
{
    switch (direction)
    {
    case UP:
        for (int i = 0; i < game->board_size - 1; i++)
        {
            for (int j = 0; j < game->board_size; j++)
            {
                if (game->board[i][j] != 0 && game->board[i][j] == game->board[i + 1][j])
                {
                    game->board[i][j] = game->board[i][j] + game->board[i + 1][j];
                    game->board[i + 1][j] = 0;
                    game->score += game->board[i][j];
                    *moveCount += 1;
                    *comboCount += 1;
                }
            }
        }
        break;
    case DOWN:
        for (int i = game->board_size - 1; i > 0; i--)
        {
            for (int j = game->board_size - 1; j >= 0; j--)
            {
                if (game->board[i][j] != 0 && game->board[i][j] == game->board[i - 1][j])
                {
                    game->board[i][j] = game->board[i][j] + game->board[i - 1][j];
                    game->board[i - 1][j] = 0;
                    game->score += game->board[i][j];
                    *moveCount += 1;
                    *comboCount += 1;
                }
            }
        }
        break;
    case LEFT:
        for (int i = 0; i < game->board_size - 1; i++)
        {
            for (int j = 0; j < game->board_size; j++)
            {
                if (game->board[j][i] != 0 && game->board[j][i] == game->board[j][i + 1])
                {
                    game->board[j][i] = game->board[j][i] + game->board[j][i + 1];
                    game->board[j][i + 1] = 0;
                    game->score += game->board[j][i];
                    *moveCount += 1;
                    *comboCount += 1;
                }
            }
        }
    case RIGHT:
        for (int i = game->board_size - 1; i > 0; i--)
        {
            for (int j = game->board_size - 1; j >= 0; j--)
            {
                if (game->board[j][i] != 0 && game->board[j][i] == game->board[j][i - 1])
                {
                    game->board[j][i] = game->board[j][i] + game->board[j][i - 1];
                    game->board[j][i - 1] = 0;
                    game->score += game->board[j][i];
                    *moveCount += 1;
                    *comboCount += 1;
                }
            }
        }
    default:
        break;
    }
}

boolean check_game_clear(p_Game game)
{
    for (int i = 0; i < game->board_size; i++)
    {
        for (int j = 0; j < game->board_size; j++)
        {
            if (game->board[i][j] == MAX_BLOCK_NUMBER)
                return TRUE;
        }
    }
    return FALSE;
}

boolean check_game_over(p_Game game)
{
    // Check row
    for (int i = 0; i < game->board_size; i++)
    {
        for (int j = 0; j < game->board_size - 1; j++)
        {
            if (game->board[i][j] == game->board[i][j + 1])
                return FALSE;
            if (game->board[j][i] == game->board[j + 1][i])
                return FALSE;
        }
    }
    for (int i = 0; i < game->board_size; i++)
    {
        for (int j = 0; j < game->board_size; j++)
        {
            if (game->board[i][j] == 0)
                return FALSE;
        }
    }
    return TRUE;
}

void add_new_number(p_Game game, int blankAmount)
{
    int randomPos = (rand() % blankAmount);
    int count = -1;

    for (int i = 0; i < game->board_size; i++)
    {
        for (int j = 0; j < game->board_size; j++)
        {
            if (game->board[i][j] == 0)
                count++;
            if (game->board[i][j] == 0 && count == randomPos)
            {
                int newNumber = (rand() & 1) ? 2 : 4;
                game->board[i][j] = newNumber;
            }
        }
    }
}

int get_blank_count(p_Game game)
{
    int count = 0;
    for (int i = 0; i < game->board_size; i++)
    {
        for (int j = 0; j < game->board_size; j++)
        {
            if (game->board[i][j] == 0)
                count++;
        }
    }

    return count;
}

/**
 * @brief Moves the console cursor to the specified location.
 *
 * @param x
 * @param y
 */
void gotoxy(int x, int y)
{
    printf("\033[%d;%df", y, x);
    fflush(stdout);
}