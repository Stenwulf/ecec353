/******************************************
 * q
 * Chat Server
 *
 * Erick Weigel
 * Gerardo Faia
 *
 * Programming Assignment 3
 * Local Chat Server
 *****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "chat.h"

#define CLIENT_MAX 10
#define GROUP_MAX 10
#define PEER_MAX 10

/* 
 * Server Command Definitons can be found in chat.h
 * 
 * Server Status List
 *    0 - Server Stopped
 *    1 - Server Running
 */

void print_commands();

int main(int argc, char **argv){
 
   // Initialize loop iterators
   int i;
   int j;
 
   // Initalize Status Indicators
   int status; 
   int server_status;

   struct stat st;
   dev_t dev;

   // Intialize Characters
   char command_line[MESSAGE_SIZE];

   // Initialize Files
   FILE* server_fifo;

   // Intialize Client List Array
   //    Can be accessed using standard array notatiom
   //       client_list[i] = "somestring"
   //
   //    Overwritting an array location is allowed
 

   char **client_list = (char**)calloc(CLIENT_MAX, sizeof(char*));
   for(i = 0; i < CLIENT_MAX; i++){
      client_list[i] = (char*)calloc(MESSAGE_SIZE, sizeof(char*));
   }

   // Intialize Group Usage Array
   //    Maintains a list of integers indicating which group slots are
   //    available.

   int *group_usage = (int*)calloc(CLIENT_MAX, sizeof(int));
   for(i = 0; i < CLIENT_MAX; i++){
      group_usage[i] = 0;
   }    

   // Initalize Group Memember list
   //    This structure is a list of lists that contain the members
   //    of each group on the server
   //
   //    group_list[0][0] = "Member 0 of group 0"

   char ***group_members = calloc(GROUP_MAX, sizeof(***group_members));
   for(i = 0; i < CLIENT_MAX; i++){
      group_members[i] = calloc(CLIENT_MAX, sizeof(char*));
   }
   for(i = 0; i < CLIENT_MAX; i++){
      for(j = 0; j < CLIENT_MAX; j++){
         group_members[i][j] = calloc(MESSAGE_SIZE, sizeof(char*));
      }
   }

   // Setup FIFO File as a pipe if it doesn't exist
   //    Defines a new special file in a temp directory
   //    SERVER_PIPE is defined in "chat.h"

   if(stat(SERVER_PIPE, &st) != 0){
      //unlink(SERVER_PIPE);
      status = mkfifo(SERVER_PIPE, 0666);
      if(status != 0){
         printf("Error: Server Pipe could not be created.\n");
         return -1;
      }
   } 

   

   // Set server status to run and print commands
   server_status = 1;
   print_commands();   

   //server_fifo = (FILE*)open(SERVER_PIPE, O_RDONLY);
   if(server_fifo == NULL){
      printf("Failed to open file.\n");
   }
   
   server_fifo = fopen(SERVER_PIPE, "r+");
  
   // Main While Loop
   while(server_status){

      // Check for commands
      if(fgets(command_line, MESSAGE_SIZE, stdin) != NULL){

         // Exit Command
         if(strncmp(S_COMMAND_EXIT,command_line, 5) == 0){
            printf("Closing Server.\n");
            server_status = 0;
            unlink(SERVER_PIPE);
         }
         
         // Help Command
         if(strncmp(S_COMMAND_HELP,command_line, 5) == 0){
            print_commands();
         }

         if(strncmp(S_COMMAND_READ, command_line, 5) == 0){
            fread(command_line, sizeof(command_line), 1, server_fifo);
            printf("Command Received: %s\n",command_line);
         }

         
         fflush(stdin);
      }
   }

   

   // ---- Testing -----
   struct group_context test_context;
   test_context.group_id = "teststring\n";
   if(group_usage[0] == 0){
     group_members[0][0] = test_context.group_id;   
     group_usage[0] = 1;
   }

   printf(group_members[0][0]);
   printf("%d\n",group_usage[0]);
   free(client_list);
   free(group_usage);
   //free(group_members);



   return 0;

}

// Print Commands
//    Prints a list of the server commands
void print_commands(){

   printf("\n\nCommand List:\n");
   printf("/help -- Prints this list of commands\n");
   printf("/exit -- Exits the server\n");

   return;
}
