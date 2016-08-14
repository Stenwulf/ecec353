
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

#include "chat.h"


int main(int argc, char *argv[]){
 
   // Initialize loop iterators
   int i;
   int j;

   int client_status;

   char* groupID;
   char* clientID;

   char command_line[MESSAGE_SIZE];
   char message[] = "TestMessage\n";

   FILE* server_fifo;



   if (argc != 3) {
      fprintf(stderr,"The program must have 2 arguments (groupID and clientID).\n");      
      exit(1);
   }
   else {
      groupID = argv[1];
      clientID = argv[2];
   }
  
   client_status = 1;
   server_fifo = fopen(SERVER_PIPE, "w");
   
   while(client_status){
   
      if(fgets(command_line, MESSAGE_SIZE, stdin) != NULL){

         // Exit Command
         if(strncmp(C_COMMAND_EXIT,command_line, 5) == 0){
            printf("Closing Server.\n");
            client_status = 0;
         }
         
         if(strncmp(C_COMMAND_GROUP,command_line, 5) == 0){
            printf("Writing Line.\n");
   //         server_fifo = open(SERVER_PIPE, "w");
            fwrite(message, sizeof(message), 1, server_fifo);
    //        close(server_fifo);
         }

      } 

   }
 
   return 0;
   
}
