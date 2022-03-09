/*
A simple C application program named chardevclient.c, to access the 
“character device” via the new read-only device driver, chardev.c in the kernel. 
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    int fd, c, rtn;
    fd = open("/dev/chardev", O_RDWR);  // open the chardev driver
    if (fd == -1)   // if fail to open the driver (the drive is not in kernel)
    {
        perror("open /dev/chardev");
        exit(EXIT_FAILURE);
    }

    // Successfully open the chardev driver
    printf("Reading from /dev/chardev: \n");
    while ((rtn = read(fd, &c, 1)) > 0) // read info from the driver
    {
        printf("%c", c);
    }
    if (rtn == -1)  // fail to ready info
    {
        perror("reading /dev/chardev");
    } 
    else 
    {
        printf("\n");
    }

    printf("Writing to /dev/chardev: \n");  // write info into the driver
    c = 'h';
    while ((rtn = write(fd, &c, 1)) > 0) 
    {
        printf("wrote %c\n", c);
    }
    if (rtn == -1) // fail to write info
    {
        perror("writing /dev/chardev");
    }

    exit(EXIT_SUCCESS);
}