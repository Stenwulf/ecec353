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

#include <poll.h>

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
   char NewMessage[MESSAGE_SIZE];

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

   //int *group_usage = (char*)calloc(CLIENT_MAX, sizeof(int));
   char **group_usage = (char**)calloc(CLIENT_MAX, sizeof(char*));
   for(i = 0; i < CLIENT_MAX; i++){
      //group_usage[i] = 0;
        group_usage[i] = (char*)calloc(MESSAGE_SIZE, sizeof(char*));

   }    

   // Initalize Group Memember list
   //    This structure is a list of lists that contain the members
   //    of each group on the server
   //
   //    group_list[0][0] = "Member 0 of group 0"


//OLD CODE DONT USE
   //char ***group_members = calloc(GROUP_MAX, sizeof(***group_members));
   //for(i = 0; i < CLIENT_MAX; i++){
   //   group_members[i] = calloc(CLIENT_MAX, sizeof(char*));
   //}
   //for(i = 0; i < CLIENT_MAX; i++){
   //   for(j = 0; j < CLIENT_MAX; j++){
   //      group_members[i][j] = calloc( MESSAGE_SIZE, sizeof(char*));
   //   }
   //}

   char **group_members = calloc(GROUP_MAX, MESSAGE_SIZE*sizeof(char));
   for(i = 0; i < CLIENT_MAX; i++){
      group_members[i] = calloc(CLIENT_MAX, MESSAGE_SIZE* sizeof(char*));
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
//      struct pollfd mypoll = {STDIN_FILENO, POLLIN|POLLPRI};
//      if(poll(&mypoll, 2, 1000))

      if(fgets(command_line, MESSAGE_SIZE, stdin)!= NULL)
      {
         // Check for commands
            // Exit Command
            if(strncmp(S_COMMAND_EXIT,command_line, 5) == 0){
               printf("Closing Server.\n");
               server_status = 0;
               unlink(SERVER_PIPE);
               break;
            }
         
         // Help Command
            if(strncmp(S_COMMAND_HELP,command_line, 5) == 0){
               print_commands();
            }

            fflush(stdin);
     // }
     // else
     // {

            fread(command_line, sizeof(command_line), 1, server_fifo);
            printf("Command Received: %s\n",command_line);


	    char cmd_line_split[MESSAGE_SIZE];
            char New_Message[MESSAGE_SIZE];
            char *token_ptr;
            int clientListID, groupUsageID;
            int compare_test;
            

            strcpy(cmd_line_split, command_line);
            
            token_ptr = strtok(cmd_line_split," ");

            struct group_context Group1;

            while( token_ptr!= NULL){
            compare_test = 0;
               if(strcmp("/u", token_ptr) == 0){
                  token_ptr = strtok(NULL, " ,");
                  //strcpy(Group1.client_id, p);
                  for(i = 0; i < CLIENT_MAX; i++){
                     if(strcmp(client_list[i], token_ptr) == 0){ 
                        goto bad_username;
                     }
                  }
                  for(i = 0; i < CLIENT_MAX; i++){
                     if(*client_list[i] == 0){
                        strcpy(client_list[i],token_ptr);
                        clientListID = i;
                        break;
                     }
                     //NEED CONDITION FOR MORE THAN 10 people to send back
                  
                  }
    
               }
               else if(strcmp("/g", token_ptr) == 0){
                  token_ptr = strtok(NULL, " ,");
                  //strcpy(Group1.group_id, p);
                  for(i = 0; i < GROUP_MAX; i++){
                     if(strcmp(group_usage[i], token_ptr) == 0){ 
                        group_members[i][clientListID] = 1;
                        compare_test = 1;
                        break;
                     }
                  }
                  if (compare_test != 1){
                     for(i = 0; i < GROUP_MAX; i++){
                        if(*group_usage[i] == 0){
                           strcpy(group_usage[i],token_ptr);
                           groupUsageID = i;
                           group_members[groupUsageID][clientListID] = 1;
                           break;
                        }
                     }
                  }
                 
               }
               else if(strcmp("/t", token_ptr) == 0){
                  token_ptr = strtok(NULL, " ,");
                  if(strcmp("/w", token_ptr) == 0){
                    //WRITE FUNCTION TO HANDLE WHISPER
                    //Need to delete last element and handle "blank" msgs
                     token_ptr = strtok(NULL, " ,");
                     char* ToUser;
                     strcpy(ToUser, token_ptr);
                     token_ptr = strtok(NULL, " ,");
                     while( token_ptr!= NULL){
                        strcpy(New_Message, token_ptr);
                        strcpy(" ", token_ptr);
                        token_ptr = strtok(NULL, " ,");
                     }
                  }
                  else
                  {
                        memcpy(NewMessage, &command_line[3], sizeof(command_line));
                        NewMessage[sizeof(command_line)] = '\0';
                  }
 
                  }
                 
               token_ptr = strtok(NULL, " ,");
            }
      }
   }
   // ---- Testing -----
   struct group_context test_context;
  // test_context.group_id = "teststring\n";
  // if(group_usage[0] == 0){
  //   group_members[0][0] = test_context.group_id;   
  //   group_usage[0] = 1;
 //  }

   printf("%d\n",group_members[0][0]);
   printf("%d\n",group_members[0][1]);
   printf("%d\n",group_members[1][0]);
   printf("%d\n",group_members[1][1]);
   printf("%d\n",group_members[2][0]);
   printf("%s\n",group_usage[0]);
   printf("%s\n",group_usage[1]);
   printf("%s\n",group_usage[2]);
   free(client_list);
   free(group_usage);
   //free(group_members);

//GOTO STATEMENT
   bad_username:
   //send message to client to exit gracefuly and pick a different username


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


