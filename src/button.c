#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "button.h"

int btnArr[] = BTN_CODE_ARR;

int button_open_dev_file(char* file)
{
    return open(file, O_RDWR | O_NONBLOCK);
}

int button_hit(int fd)
{
    fd_set read_fd;
    struct timeval timeout;
    int select_ret;

    /* Initialize the file descriptor set. */
    FD_ZERO( &read_fd );
    FD_SET( fd, &read_fd );

    /* Initialize the timeout */
    timeout.tv_sec  = 0;       //0 Seconds
    timeout.tv_usec = 0;

    select_ret = select(FD_SETSIZE, &read_fd, NULL, NULL, &timeout);

    if( select_ret < 0 )
    {
        perror("select");
        assert(0);
    }
    return  FD_ISSET( fd, &read_fd );
}

button_t button_read(int btn_fd)
{
    char kernel_val[5];
    if (read(btn_fd, kernel_val, sizeof(kernel_val)) != 1)
    {
        return BTN_DEFAULT;
    }
    return (button_t)btnArr[kernel_val[0]];
}

// void main(void)
// {
//     char kernel_val[10];
//     memset(kernel_val, 0, sizeof(kernel_val));
//     fd_set read_fd;
//     struct timeval timeout;
//     int    ret;
//     int    fd = open("/dev/my_buttonf", O_RDWR | O_NONBLOCK);

//     if( fd == -1 )
//     {
//         perror("open");
//         exit(EXIT_FAILURE);
//     }

//     while(1)
//     {
//         puts("Starting Select...");

//         /* Initialize the file descriptor set. */
//         FD_ZERO( &read_fd );
//         FD_SET( fd, &read_fd );

//         /* Initialize the timeout */
//         timeout.tv_sec  = 0;       //0 Seconds
//         timeout.tv_usec = 0;

//         ret = select(FD_SETSIZE, &read_fd, NULL, NULL, &timeout);

//         if( ret < 0 )
//         {
//             perror("select");
//             assert(0);
//         }

//         if( FD_ISSET( fd, &read_fd ) )
//         {
//             read(fd, kernel_val, sizeof(kernel_val));
//             printf("READ : Kernel_val = %s\n", kernel_val);
//         }

//     }

// }