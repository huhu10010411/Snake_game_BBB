#ifndef  _BUTTON_H_
#define  _BUTTON_H_

#define BTN_DEFAULT_CODE    0
#define BTN_LEFT_CODE       1
#define BTN_RIGHT_CODE      2
#define BTN_UP_CODE         3
#define BTN_DOWN_CODE       4
#define BTN_OK_CODE         5

#define BTN_CODE_ARR    {BTN_DEFAULT_CODE, BTN_LEFT_CODE, BTN_RIGHT_CODE, BTN_UP_CODE, BTN_DOWN_CODE, BTN_OK_CODE}

typedef enum {
    BTN_DEFAULT = 0,
    BTN_LEFT    = 1,
    BTN_RIGHT,
    BTN_UP,
    BTN_DOWN,
    BTN_OK
}button_t;

int button_open_dev_file(char* file);

int button_hit(int fd);

button_t button_read(int btn_fd);

#endif /*_BUTTON_H_*/