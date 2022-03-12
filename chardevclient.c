#include <stdio.h>

#include <stdlib.h>

#include <errno.h>

#include <fcntl.h>

#include <string.h>

#define BUFFER_LENGTH 256

int main()

{

   int fd, rtn;

   char stringToSend[BUFFER_LENGTH];

   static char receiveString[BUFFER_LENGTH];

   printf("CSC1007 assignment project - User space application program\n");

   fd = open("/dev/chardev", O_RDWR); /* open the chardev driver*/

   if (fd == -1) /*if fail to open the driver (the drive is not in kernel) */

   {

      perror("open /dev/chardev");

      exit(EXIT_FAILURE);
   }

   printf("Type in a sentence/message:\n");

   scanf("%[^\n]%*c", stringToSend); /* all the characters entered as the input, including the spaces, until we hit the enter button */

   rtn = write(fd, stringToSend, strlen(stringToSend));

   if (rtn < 0)
   {

      perror("Failed to write the message to the device.");

      exit(EXIT_FAILURE);
   }

   printf("\nPress the ENTER key (Wait for the user keyboard input)\n");

   getchar();

   printf("Sentences/messages being read back from the device driver\n");

   rtn = read(fd, receiveString, BUFFER_LENGTH);

   if (rtn < 0)
   {

      perror("Failed to read the message from the device.");

      exit(EXIT_FAILURE);
   }

   printf("%s\n", receiveString);

   exit(EXIT_SUCCESS);
}
