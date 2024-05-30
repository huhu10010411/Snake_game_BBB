#include "button.h"
#include "lcd.h"
#include "snake.h"

#include <stdlib.h>
#include <stdio.h>

#define BOARDER_COL         "|"
#define BOARDER_ROW         "_"

#define BOARD_HEIGHT    6
#define BOARD_WIDTH    14

#define LCD_DEV_FILE    "/dev/nokia5110f"
#define BUTTON_DEV_FILE "/dev/my_buttonf"

static int lcd_fd;
static int btn_fd;

void displayBoard(struct board *board);

void displayFood(coordinate_t *food);

void displaySnake(coordinate_t move, struct snake *snake);

static void menu(void);

void init()
{
    /* open lcd device file */
    lcd_fd = lcd_open_dev_file(LCD_DEV_FILE);

    /* open button device file */
    btn_fd = button_open_dev_file(BUTTON_DEV_FILE);
}


direction_t dir_arr[4] = {DIRECTION_RIGHT, DIRECTION_DOWN, DIRECTION_LEFT, DIRECTION_UP};



void Delay(long double k)
{
    long double i;
    for(i=0; i<=(10000000)*k; i++);
}

struct snake SNAKE_DEFAULT = {
        .direction = DIRECTION_RIGHT,
        .length = 3,
        .body = {{5,2}, {4,2}, {3,2}}
        };

static char lcd_buffer[50];

void main(void)
{
    coordinate_t move = {.x = 0, .y = 0};
    static int i = 0;
    int j;

    struct snake snake =  SNAKE_DEFAULT;
    struct board board = {
        .length = BOARD_HEIGHT,
        .width = BOARD_WIDTH,
        .min = {
            .x = 0,
            .y = 0,
        }
    };

    init();

    // lcd_clearScreen(lcd_fd);
    // lcd_gotoXY(lcd_fd, 0, 0);
    // for (int k = 1; k < 20 ; k++)
    // {
    //     lcd_write_string(lcd_fd, BOARDER_ROW);
    // }
menu:
    menu();
    while (!button_hit(btn_fd))
    {
        /* code */
    }
    button_read(btn_fd);

    displayBoard(&board);

    coordinate_t food = snake_generateFood(&board, &snake);
    displayFood(&food);
    move = snake_move(&snake);
    // printf("move = %d", move);
    displaySnake(move, &snake);

    // while (1)
    // {
    //     sleep(1);
    // }
label:

    do
    {

        if (snake_collisionBoard(&board, &snake) == true
        || snake_collisionBody(&snake) == true)
        {
            // system("cls");
            lcd_clearScreen(lcd_fd);
            sprintf(lcd_buffer, "Score: %d", snake.length);
            lcd_gotoXY(lcd_fd, 3, 0);
            lcd_write_string(lcd_fd, "END game");
            lcd_gotoXY(lcd_fd, 3, 1);
            lcd_write_string(lcd_fd, lcd_buffer);
            lcd_gotoXY(lcd_fd, 0, 2);
            lcd_write_string(lcd_fd, "Press any key to exit");

            while (!button_hit(btn_fd))
            {

            }
            button_read(btn_fd);

            /* Do reset all variable */
            snake = SNAKE_DEFAULT;
            goto menu;
        }
        if ( snake_eat(&snake, &food) == true )
        {
           food = snake_generateFood(&board, &snake);
           displayFood(&food);
        }
        move = snake_move(&snake);
        displaySnake(move, &snake);
        // Delay(3);
        sleep(1);
    }while (!button_hit(btn_fd));

    button_t key = button_read(btn_fd);
    switch (key)
    {
    case BTN_RIGHT:
        if (snake.direction != DIRECTION_LEFT)
        {
            snake.direction = DIRECTION_RIGHT;
        }
        break;
    case BTN_LEFT:
        if (snake.direction != DIRECTION_RIGHT)
        {
            snake.direction = DIRECTION_LEFT;
        }
        break;
    case BTN_UP:
        if (snake.direction != DIRECTION_DOWN)
        {
            snake.direction = DIRECTION_UP;
        }
        break;
    case BTN_DOWN:
        if (snake.direction != DIRECTION_UP)
        {
            snake.direction = DIRECTION_DOWN;
        }
        break;
    default:
        break;
    }
    goto label;

}

void displaySnake(coordinate_t move, struct snake *snake)
{
    int i;

    lcd_clearXY(lcd_fd, move.x, move.y);

    lcd_gotoXY(lcd_fd , snake->body[0].x, snake->body[0].y);
    switch (snake->direction)
    {
    case DIRECTION_LEFT:
        lcd_write_string(lcd_fd, SNAKE_HEAD_LEFT);
        break;
    case DIRECTION_RIGHT:
        lcd_write_string(lcd_fd, SNAKE_HEAD_RIGHT);
        break;
    case DIRECTION_UP:
        lcd_write_string(lcd_fd, SNAKE_HEAD_UP);
        break;
    case DIRECTION_DOWN:
        lcd_write_string(lcd_fd, SNAKE_HEAD_DOWN);
        break;
    default:
        break;
    }

    for(i = 1; i < snake->length; i++)
    {
        lcd_gotoXY(lcd_fd, snake->body[i].x, snake->body[i].y);
        lcd_write_string(lcd_fd, SNAKE_BODY);
    }
}

void displayFood(coordinate_t *food)
{
    lcd_gotoXY(lcd_fd, food->x, food->y);
    lcd_write_string(lcd_fd, FOOD);
}

void displayBoard(struct board *board)
{
    int i;

    lcd_clearScreen(lcd_fd);

    lcd_gotoXY(lcd_fd, board->min.x, board->min.y);
    for (i = board->min.x; i < board->min.x + board->width ; i++)
    {
        lcd_write_string(lcd_fd, BOARDER_ROW);
    }

    lcd_gotoXY(lcd_fd, board->min.x, board->min.y + board->length - 1);
    for (i = board->min.x; i < board->min.x + board->width ; i++)
    {
        lcd_write_string(lcd_fd, BOARDER_ROW);
    }

    for (i = board->min.y + 1; i < board->min.y + board->length ; i++)
    {
        lcd_gotoXY(lcd_fd, board->min.x, i);
        lcd_write_string(lcd_fd, BOARDER_COL);
    }

    for (i = board->min.y + 1; i < board->min.y + board->length; i++)
    {
        lcd_gotoXY(lcd_fd, board->min.x + board->width - 1, i);
        lcd_write_string(lcd_fd, BOARDER_COL);
    }

}

static void menu(void)
{
    lcd_clearScreen(lcd_fd);
    lcd_write_string(lcd_fd, "Press any key to start");
}
