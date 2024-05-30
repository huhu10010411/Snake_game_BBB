#include "lcd.h"

#define BUFFER_SIZE     30

static char write_buffer[BUFFER_SIZE];

int lcd_open_dev_file(char *file)
{
    return open(file, O_WRONLY);
}

void lcd_gotoXY(int lcd_fd, int x, int y)
{
    int len = sprintf(write_buffer, "%s %d %d", LCD_CMD_SET_CURSOR, x, y);
    write(lcd_fd, write_buffer, len);
}

void lcd_clearXY(int lcd_fd, int x, int y)
{
    lcd_gotoXY(lcd_fd, x, y);
    write(lcd_fd, " ", 1);
}

void lcd_clearScreen(int lcd_fd)
{
    int len = sprintf(write_buffer, "%s", LCD_CMD_CLR);
    write(lcd_fd, write_buffer, len);
}

void lcd_write_string(int lcd_fd, char *str)
{
    int len = sprintf(write_buffer, "%s", str);
    write(lcd_fd, write_buffer, len);
}
