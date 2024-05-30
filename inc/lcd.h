#ifndef  _LCD_H_
#define  _LCD_H_

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define LCD_CMD_CLR             "clr"
#define LCD_CMD_SET_CURSOR      "curs"

int lcd_open_dev_file(char *file);

void lcd_gotoXY(int lcd_fd, int x, int y);

void lcd_clearXY(int lcd_fd, int x, int y);

void lcd_clearScreen(int lcd_fd);

void lcd_write_string(int lcd_fd, char *str);

#endif /*_LCD_H_*/
