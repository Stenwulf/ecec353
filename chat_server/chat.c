/* 
 * chat.c contains definitions for chat.h functions
 *
 *
 **/

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include "chat.h"

// Connects to the server fifo and returns its file descripter
int connect_ServerPipe_Read(){

   int val;

   val = open(SERVER_PIPE, O_NONBLOCK | O_RDONLY);

   if(val < 1){
      printf("Error opening pipe.\n");
   }

   return val;
}

int connect_ClientPipe_Server(char* path){

   int val;
   
   int count = 0;
   while(val < 1){
     val = open(path, O_NONBLOCK | O_RDWR);
     if(val < 1){
        count++;
        if(count > 1000){break;}
     }
   }
   if(val < 1){
      printf("Error opening pipe.\n");
   }
   else{
         printf("Pipe Opened Sucessfully: %s\n", path);
      }
   return val;
}

int connect_ClientPipe_Write(char* path){

   int val;
   
   int count = 0;
   while(val < 1){
     val = open(path, O_NONBLOCK | O_WRONLY);
     if(val < 1){
        count++;
        if(count > 1000){break;}
     }
   }
   if(val < 1){
      printf("Error opening pipe.\n");
   }
   else{
         printf("Pipe Opened Sucessfully: %s\n", path);
      }
   return val;
}

int connect_ClientPipe_Read(char* path){

   int val;
   int count = 0;
   while(val < 1){
      val = open(path, O_NONBLOCK | O_RDONLY);
      if(val < 1){
         count++;
         if(count > 1000){break;}
      }
   }
      if(val < 1){
         printf("Error opening pipe.\n");
      }
      else{
         printf("Pipe Opened Sucessfully: %s with value: %d\n", path, val);
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

void build_ClientWrite_ID(char* pipe_name, char* clientID){
            
   strcpy(pipe_name, clientID);
   strcat(pipe_name, "_w");

   return;
}

void build_ClientRead_ID(char* pipe_name, char* clientID){
            
   strcpy(pipe_name, clientID);
   strcat(pipe_name, "_r");

   return;
}
