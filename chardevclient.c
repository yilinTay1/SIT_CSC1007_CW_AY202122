#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 256

int main()
{
   int fd, callback;
   char stringToSend[BUFFER_LENGTH];
   static char receiveString[BUFFER_LENGTH];
   /* Print title*/
   printf("CSC1007 assignment project - User space application program\n");

   /* Open the chardev driver*/
   fd = open("/dev/chardev", O_RDWR);
   if (fd == -1)
   {
      /*Catch missing driver exception, fail to open the driver (the drive is not in kernel) */
      perror("open /dev/chardev");
      exit(EXIT_FAILURE);
   }

   while (1)
   {
      printf("\n=============================================================================\n");
      printf("\n Hint: You can CTRL + Z to exit program\n");

      printf("\nEnter sentence/message:");
      /* Scan all the characters entered as the input, including the spaces, until we hit the enter button */
      scanf("%[^\n]%*c", stringToSend);
      /* Write to driver */
      callback = write(fd, stringToSend, strlen(stringToSend));

      if (callback == -1)
      {
         /*Catch if fail to write message into device */
         perror("Failed to write the message to the device.");
         exit(EXIT_FAILURE);
      }

      printf("\nPress the ENTER key to read from driver (Waiting for the user keyboard input ...)");
      getchar();
      /* Read to driver , print call back*/
      callback = read(fd, receiveString, BUFFER_LENGTH);
      if (callback == -1)
      {
         /*Catch if fail to read message from device */
         perror("Failed to read the message from the device.");
         exit(EXIT_FAILURE);
      }
      printf("\nDriver input : %s", receiveString);
   }
   exit(EXIT_SUCCESS);
}
