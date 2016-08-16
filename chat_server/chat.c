/* 
 * chat.c contains definitions for chat.h functions
 *
 *
 **/

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include "chat.h"

// Connects to the server fifo and returns its file descripter
int connect_ServerPipe(){

   int val;

   val = open(SERVER_PIPE, O_NONBLOCK | O_RDONLY);

   if(val < 1){
      printf("Error opening pipe.\n");
   }

   return val;
}

// Clears an fd_set then  a file dectricptor to the fd_set.
fd_set set_FileSelect_Clear(int file_desc){

   fd_set fds;
   
   FD_ZERO(&fds);
   FD_SET(file_desc, &fds);

   return fds;

}

// Sets a fd_set with a file descriptor but doesn't clear it first
fd_set set_FileSelect_NoClear(fd_set fds, int file_desc){

   FD_SET(file_desc, &fds);

   return fds;


}
