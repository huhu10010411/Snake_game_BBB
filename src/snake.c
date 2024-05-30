#include "snake.h"

bool snake_collisionObject(coordinate_t *object, struct snake* snake);

coordinate_t snake_generateFood(struct board *board, struct snake *snake);

// static void gotoXY(int x, int y);

// static void clearXY(int x, int y);

// static void clearXY(int x, int y)
// {
//     gotoXY(x, y);
//     printf("%c", BLANK);
// }

/**
 * @brief
 *
 * @param snake
 */
coordinate_t snake_move(struct snake *snake)
{
    coordinate_t ret = {
        .x = snake->body[snake->length - 1].x,
        .y = snake->body[snake->length - 1].y
    };
    int i;
    // clearXY(snake->body[snake->length - 1].x, snake->body[snake->length - 1].y);
    for (i = snake->length - 1; i > 0; i--)
    {
        snake->body[i].x = snake->body[i-1].x;
        snake->body[i].y = snake->body[i-1].y;
    }

    switch(snake->direction) {
    case DIRECTION_RIGHT:
        snake->body[0].x++;
        break;
    case DIRECTION_LEFT:
        snake->body[0].x--;
        break;
    case DIRECTION_UP:
        snake->body[0].y--;
        break;
    case DIRECTION_DOWN:
        snake->body[0].y++;
        break;
    default:
        break;
    }
    return ret;
}

bool snake_eat(struct snake *snake, coordinate_t *food)
{
    bool ret = false;
    if (snake->body[0].x == food->x && snake->body[0].y == food->y)
    {
        snake->length++;    /* Increase length  */
        ret = true;
    }
    return ret;
}

coordinate_t snake_generateFood(struct board *board, struct snake *snake)
{
    coordinate_t food;
    do {
        srand(time(NULL));
        food.x = rand() % (board->width - 2) + board->min.x + 1;    /* Avoid 0 coordinate, avoid board */
        srand(time(NULL));
        food.y = rand() % (board->length - 2) + board->min.y + 1;   /* Avoid 0 coordinate, avoid board */
    }while (snake_collisionObject(&food, snake));                   /* Avoid food is collision with snake body */

    return food;
}

bool snake_collisionObject(coordinate_t *object, struct snake* snake)
{
    bool ret = false;
    int i;

    for (i = 0; i < snake->length; i++)
    {
        if((object->x == snake->body[i].x)
            && (object->y == snake->body[i].y))
        {
            ret = true;                           /* object coordinate equal to one of snake body coordinate */
            break;
        }
    }
    return ret;
}

bool snake_collisionBody(struct snake* snake)
{
    bool ret = false;
    int i;
    for (i = 1; i < snake->length; i++)
    {
        if ((snake->body[0].x == snake->body[i].x) && (snake->body[0].y == snake->body[i].y))
        {
            ret = true;
            break;
        }
    }
    return ret;
}

bool snake_collisionBoard(struct board *board, struct snake* snake)
{
    bool ret = false;
    if ((snake->body[0].x <= board->min.x)
        ||(snake->body[0].x >= board->min.x + board->width - 1)
        ||(snake->body[0].y <= board->min.y)
        ||(snake->body[0].y >= board->min.y + board->length - 1))
    {
        ret = true;
    }
    return ret;
}

// void gotoXY(int x, int y)
// {
//     #ifdef CONSOLE
//     HANDLE a;
//     COORD b;
//     fflush(stdout);
//     b.X = x;
//     b.Y = y;
//     a = GetStdHandle(STD_OUTPUT_HANDLE);
//     SetConsoleCursorPosition(a,b);
//     #else

//     #endif
// }

// void printBoard(struct board *board)
// {
//     int i;
//     system("cls");
//     for (i = board->min.x; i < board->min.x + board->width ; i++)
//     {
//         gotoXY(i, board->min.y);
//         printf("!");
//         gotoXY(i, board->min.y + board->length);
//         printf("!");
//     }

//     for (i = board->min.y; i < board->min.y + board->length; i++)
//     {
//         gotoXY(board->min.x, i);
//         printf("!");
//         gotoXY(board->min.x + board->width, i);
//         printf("!");
//     }

// }

// void printFood(coordinate_t *food)
// {
//     gotoXY(food->x, food->y);
//     printf("%c", FOOD);
// }

