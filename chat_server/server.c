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
#include <sys/select.h>
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
int connect_ServerPipe();
fd_set set_FileSelect_Server(int filedesc);
fd_set set_FileSelect_STDIN();

int main(int argc, char **argv){
 
   // Initialize loop iterators
   int i;
   int j;
 
   // Initalize Status Indicators
   int status; 
   int server_status;
   int server_read;
   int stdin_read;

   struct stat st;

   // Select Parameters
   fd_set s_read_set;
   fd_set stdin_set;
   fd_set write_set;

   struct timeval read_timeout;
   
   // Intialize Characters
	char* command_line = (char*)calloc(MESSAGE_SIZE, sizeof(char));
   char* fifo_pipe = (char*)calloc(MESSAGE_SIZE, sizeof(char));

   // Initialize Files
   int server_fifo;

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
      status = mkfifo(SERVER_PIPE, 0666);
      if(status != 0){
         printf("Error: Server Pipe could not be created.\n");
         return -1;
      }
      else{
         printf("Pipe Opened");
      }
   } 

   // Connect to reading end of the server pipe
   server_fifo = connect_ServerPipe();
   printf("FIFO File Descripter: %d\n",server_fifo); 
   // Setup Select Parameters
   read_timeout.tv_sec = 0;
   read_timeout.tv_usec = 10000;

   /*FD_ZERO(&s_read_set);
   FD_SET(server_fifo, &s_read_set);*/

//   s_read_set = set_FileSelect_Server(server_fifo);

   // Set server status to run and print commands
   server_status = 1;
   print_commands();   
 
   // Main While Loop
   while(server_status){
      
      s_read_set = set_FileSelect_Server(server_fifo);
      server_read = select(server_fifo + 1, &s_read_set, NULL, NULL, &read_timeout);

      if(FD_ISSET(server_fifo, &s_read_set)){
         read(server_fifo, command_line, MESSAGE_SIZE*sizeof(command_line));
         printf("Read: %s\n", command_line);
         memset(command_line, 0, MESSAGE_SIZE);
      }
     

      // STDIN Select for Input 
      stdin_set = set_FileSelect_STDIN();
      stdin_read = select(STDIN_FILENO + 1, &stdin_set, NULL, NULL, &read_timeout);
      // Check for commands from STDIN
      if(FD_ISSET(STDIN_FILENO, &stdin_set)){
         
         fgets(command_line, MESSAGE_SIZE, stdin);
         // Exit Command
         if(strncmp(S_COMMAND_EXIT,command_line, 5) == 0){
            printf("Closing Server.\n");
            server_status = 0;
            // Destroy Pipe
            close(server_fifo);
            unlink(SERVER_PIPE);
            // Free MALLOCs
            free(command_line);
            free(fifo_pipe);
         }
         
         // Help Command
         if(strncmp(S_COMMAND_HELP,command_line, strlen(S_COMMAND_HELP)) == 0){
            print_commands();
         }

         // Read Command
         if(strncmp(S_COMMAND_READ, command_line, 5) == 0){

            printf("File Opened\n");
            memset(command_line, 0 , MESSAGE_SIZE);
            read(server_fifo, command_line, MESSAGE_SIZE*sizeof(command_line));
            printf("Command: %s\n", command_line);
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
   printf("/read -- Test Command :: Read Line from FIFO\n");
   return;

}

int connect_ServerPipe(){
   int val;
   val = open(SERVER_PIPE, O_NONBLOCK | O_RDONLY);
   if(val < 1){
      printf("Error opening pipe.");
   }
   return val;
}

// Creates a file descriptor set to check server fifo for reading
fd_set set_FileSelect_Server(int filedesc){

   fd_set fds;

   FD_ZERO(&fds);
   FD_SET(filedesc, &fds);

   return fds;
}

// Creates a file descriptor set to check STDIN for input
fd_set set_FileSelect_STDIN(){

   fd_set fds;

   FD_ZERO(&fds);
   FD_SET(STDIN_FILENO, &fds);

   return fds;
}
