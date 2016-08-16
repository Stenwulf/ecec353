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
#include <sys/select.h>
#include <unistd.h>

#include "chat.h"

// Command Building Prototypes
void build_JoinGroup(char* command_line, char* clientID, char* groupID);
void build_WaitingForConnection(char* command_line);

int main(int argc, char *argv[]){
 
   // Initialize loop iterators
   int i;
   int j;
   int client_status;
   int server_fifo_status;
   int stdin_status;

   // Command Line Args
   char* groupID;
   char* clientID;

<<<<<<< HEAD
   // Command Messages
   char* command_line = (char*)calloc(MESSAGE_SIZE, sizeof(char));
   char* command_message = (char*)calloc(MESSAGE_SIZE, sizeof(char));
=======
   char message[MESSAGE_SIZE] = "/g coolgroup /u userName\n";
>>>>>>> 05f54a13be1ced26e862bfed788476f34a7cd976

   int server_fifo;

   fd_set stdin_set;
   fd_set server_fifo_write;

   struct timeval join_rest;
   join_rest.tv_sec = 0;
   join_rest.tv_usec = 100;

   struct timeval read_timeout;
   read_timeout.tv_sec = 0;
   read_timeout.tv_usec = 10000;

   // Get group and client ID from command line
   if (argc != 3) {
      fprintf(stderr,"The program must have 2 arguments (groupID and clientID).\n");      
      exit(1);
   }
   else {
      groupID = argv[1];
      clientID = argv[2];
   }
  
   // Init Loop Status
   client_status = 1;

   // Open connection to main server pipe
   server_fifo = open(SERVER_PIPE, O_NONBLOCK | O_WRONLY);
   if(server_fifo < 0){
      printf("Error opening pipe.\n");
   }
   else{
      build_JoinGroup(command_line,clientID,groupID);
      write(server_fifo, command_line, MESSAGE_SIZE*sizeof(char));

      //select(0, NULL, NULL, NULL, &join_rest);
     
      //build_WaitingForConnection(command_line);
      //write(server_fifo, command_line, MESSAGE_SIZE*sizeof(char));
      //memset(command_line, 0, MESSAGE_SIZE);
   }

   printf("Client: %s. Connected to Group: %s.\n", clientID, groupID);
   while(client_status){
      

      stdin_set = set_FileSelect_Clear(STDIN_FILENO);
      stdin_status = select(STDIN_FILENO+1, &stdin_set, NULL, NULL, &read_timeout);
      
      if(FD_ISSET(STDIN_FILENO, &stdin_set)){
         // Get input
         fgets(command_line, MESSAGE_SIZE, stdin);
         // Exit Command
         if(strncmp(C_COMMAND_EXIT,command_line, 2) == 0){
            printf("Closing Server.\n");
            client_status = 0;
            close(server_fifo);

            free(command_line);
            free(command_message);
         }
         
         // Join Group Command
         if(strncmp(C_COMMAND_GROUP,command_line, 2) == 0){
            printf("%s\n",command_line);
            write(server_fifo, command_line, MESSAGE_SIZE*sizeof(char));
            printf("Wrote to file.\n");
            memset(command_line, 0, MESSAGE_SIZE);
         }

      } 

   }
 
   return 0;
   
}

void build_JoinGroup(char* command_line, char* clientID, char* groupID){
   // Intitalize Emtpy String to hold command
   memset(command_line, 0, MESSAGE_SIZE);

   // Add command to head
   strcpy(command_line,COMMAND_JOIN);

   // Add client and group to string
   strcat(command_line,clientID);
   strcat(command_line,COMMAND_DELIM);

   strcat(command_line,groupID);
}

void build_WaitingForConnection(char* command_line){
   memset(command_line, 0, MESSAGE_SIZE);
   strcpy(command_line,J_CLIENT_CONN);
}
