
/******************************************
 * Chat Client
 *
 * Erick Weigel
 * Gerardo Faia
 *
 * Programming Assignment 3
 * Local Chat Server
 *****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "chat.h"


int main(int argc, char *argv[]){
 
   // Initialize loop iterators
   int i;
   int j;

   int client_status;

   char* groupID;
   char* clientID;
   char command_line[MESSAGE_SIZE];

   char message[MESSAGE_SIZE] = "TestMessage\n";

   int server_fifo;



   if (argc != 3) {
      fprintf(stderr,"The program must have 2 arguments (groupID and clientID).\n");      
      exit(1);
   }
   else {
      groupID = argv[1];
      clientID = argv[2];
   }
  
   client_status = 1;

   while(client_status){
   
      if(fgets(command_line, MESSAGE_SIZE, stdin) != NULL){

         // Exit Command
         if(strncmp(C_COMMAND_EXIT,command_line, 5) == 0){
            printf("Closing Server.\n");
            client_status = 0;
         }
         
         if(strncmp(C_COMMAND_GROUP,command_line, 5) == 0){

            server_fifo = open(SERVER_PIPE, O_NONBLOCK | O_WRONLY);
            if(server_fifo < 0){
               printf("Error opening pipe.\n");
            }

            write(server_fifo, &message, sizeof(message));
            printf("Wrote to file.\n");
            close(server_fifo);
         }

      } 

   }
 
   return 0;
   
}
