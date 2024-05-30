#ifndef _SNAKE_H_
#define _SNAKE_H_

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define SNAKE_MAX_LENGTH        50

#define SNAKE_HEAD_LEFT     "<"
#define SNAKE_HEAD_RIGHT    ">"
#define SNAKE_HEAD_UP       "^"
#define SNAKE_HEAD_DOWN     "v"

#define SNAKE_BODY  "o"
// #define WALL        (char)219
#define FOOD        "*"
// #define BLANK       ' '

/**
 * @brief Coordinate typedef
 *
 */
typedef struct {
    int x;
    int y;
}coordinate_t;

typedef enum {
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_UP,
    DIRECTION_DOWN
}direction_t;

/**
 * @brief   Snake object
 *
 */
struct snake
{
    coordinate_t    body[SNAKE_MAX_LENGTH];
    direction_t     direction;
    int             length;
};

struct board
{
    int length;
    int width;
    coordinate_t min;
};

// void printBoard(struct board *board);

coordinate_t snake_move(struct snake *snake);

bool snake_eat(struct snake *snake, coordinate_t *food);

coordinate_t snake_generateFood(struct board *board, struct snake *snake);

bool snake_collisionObject(coordinate_t *object, struct snake* snake);

bool snake_collisionBoard(struct board *board, struct snake* snake);

bool snake_collisionBody(struct snake* snake);

// void printSnake(struct snake *snake);

// void printFood(coordinate_t *food);

#endif /*_SNAKE_H_*/