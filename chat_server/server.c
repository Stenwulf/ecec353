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
int create_ServerFifo();
int create_ClientFifo(char* pipe_name);

void close_ClientFifo(char* clientID, int write_desc, int read_desc);
void close_ClientFifo_All(char** client_list, int* write_pipes, int* read_pipes);
//fd_set set_FileSelect_Server(int filedesc);
//fd_set set_FileSelect_STDIN();

int main(int argc, char **argv){
 
   // Initialize loop iterators
   int i;
   int j;
 
   // Initalize Status Indicators
   int status; 
   int server_status;
   int server_read;
   int stdin_read;

   // Intalize Client Index Ints
   int client_id_index;
   int group_id_index;

   // Select Parameters
   fd_set s_read_set;
   fd_set stdin_set;
   fd_set write_set;

   struct timeval read_timeout;
   
   // Intialize Charactersi
   char* command_token;
	char* command_line = (char*)calloc(MESSAGE_SIZE, sizeof(char));
   char* fifo_pipe = (char*)calloc(MESSAGE_SIZE, sizeof(char));

   // Initialize Files
   int server_fifo;
   int* w_client_pipe = (int*)calloc(CLIENT_MAX, sizeof(int));
   int* r_client_pipe = (int*)calloc(CLIENT_MAX, sizeof(int));

   // Intialize Client List Array
   //    Can be accessed using standard array notatiom
   //       client_list[i] = "somestring"
   //
   //    Overwritting an array location is allowed
   char** client_list = (char**)calloc(CLIENT_MAX,sizeof(char*));
   for(i = 0; i < CLIENT_MAX; i++){
      client_list[i] = calloc(MESSAGE_SIZE,sizeof(char));
      strcpy(client_list[i], EMPTY_CLIENT);
   }
   // Intialize Group Usage Array
   //    Maintains a list of integers indicating which group slots are
   //    available.

   char** group_list = (char**)calloc(GROUP_MAX, sizeof(char*));
   for(i = 0; i < GROUP_MAX; i++){
      group_list[i] = calloc(MESSAGE_SIZE, sizeof(char));
      strcpy(group_list[i], EMPTY_CLIENT);
   }

   int *group_usage = (int*)calloc(CLIENT_MAX, sizeof(int));
   for(i = 0; i < CLIENT_MAX; i++){
      //strcpy(group_usage[i], EMPTY_CLIENT);
   }    

   // Initalize Group Memember list
   //    This structure is a list of lists that contain the members
   //    of each group on the server
   //
   //    group_list[0][0] = "Member 0 of group 0"

   char ***group_members = calloc(GROUP_MAX, MESSAGE_SIZE);
   for(i = 0; i < CLIENT_MAX; i++){
      group_members[i] = calloc(CLIENT_MAX, sizeof(char*));
   }
   for(i = 0; i < CLIENT_MAX; i++){
      for(j = 0; j < CLIENT_MAX; j++){
         group_members[i][j] = calloc(MESSAGE_SIZE, sizeof(char*));
         strcpy(group_members[i][j], EMPTY_CLIENT);
      }
   }

   // Setup FIFO File as a pipe if it doesn't exist
   //    Defines a new special file in a temp directory
   //    SERVER_PIPE is defined in "chat.h"

   /*if(stat(SERVER_PIPE, &st) != 0){
      status = mkfifo(SERVER_PIPE, 0666);
      if(status != 0){
         printf("Error: Server Pipe could not be created.\n");
         return -1;
      }
      else{
         printf("Pipe Opened");
      }
   }*/ 

   // Connect to reading end of the server pipe
   status = create_ServerFifo();
   server_fifo = connect_ServerPipe();
   printf("FIFO File Descripter: %d\n",server_fifo); 

   // Setup Select Parameters
   read_timeout.tv_sec = 0;
   read_timeout.tv_usec = 50000;

   // Set server status to run and print commands
   server_status = 1;
   print_commands();   
 
   // Main While Loop
   while(server_status){
		

      /* 
       * ------------------- Server Master FIFO Block -------------------
 		 *
       *    This block checks the server FIFO for write activity
 		 *    
 		 *    When a write is detected it checks for a valid group join request.
 		 *    The server adds the clientID to the following structures:
 		 *       client_list
 		 *       group_members
 		 *
 		 *    The server adds the groupID to the following structures:
 		 *       group_list
 		 *
 		 *    The server creates FIFO file descriptors for read and write in
 		 *    the following structures:
 		 *       w_client_pipe
 		 *       r_client_pipe
 		 *
 		 */

      // Set Server Read FIFO fd_set every run since it gets modified by select      
      s_read_set = set_FileSelect_Clear(server_fifo);
      server_read = select(server_fifo + 1, &s_read_set, NULL, NULL, &read_timeout);
      // If Server Pipe has input
      if(FD_ISSET(server_fifo, &s_read_set)){
         printf("\n\n---\nSelect Value: %d\n", server_read);
         // Read and print the command
         read(server_fifo, command_line, MESSAGE_SIZE*sizeof(command_line));
         printf("Read: %s\n", command_line);
         // Tokenize 
         command_token = strtok(command_line,COMMAND_DELIM);
         printf("\nFirst Token: %s\n",command_token);
         // Check if its a join command
         if(strncmp(J_CLIENT_GROUP, command_token, strlen(J_CLIENT_GROUP))== 0){
            command_token = strtok(NULL, COMMAND_DELIM);
            printf("Second Token: %s\n", command_token);

            // Copy User ID to client list
            for(i = 0; i < CLIENT_MAX;  i++){
               if(strcmp(client_list[i], EMPTY_CLIENT) == 0){
                  printf("\nAdding Client |%s| to client list at position |%d|.\n",command_token, i);
                  strcpy(client_list[i], command_token);
                  client_id_index = i;
                  break;
               }
            }
            
            // Advance Token
            command_token = strtok(NULL, COMMAND_DELIM);

            // Create Group
            for(i = 0; i < GROUP_MAX; i++){
               group_id_index = i;
               if(strcmp(group_list[i], command_token) == 0){break;}
               if(strcmp(group_list[i], EMPTY_CLIENT) == 0){
                  printf("Adding group |%s| to group list at position |%d|.\n", command_token, i);
                  strcpy(group_list[i], command_token);
                  break;
               }

            } 
           
            // Adding to Group Member List
            // Commented Version copies 1 instead of client_id name
            //group_members[group_id_index][client_id_index] = 1;
            strcpy(group_members[group_id_index][client_id_index],client_list[client_id_index]);
 
            // Wait for disconnect mesage
            close(server_fifo);
            unlink(SERVER_PIPE);
            // Regenerate Fifo
            status = create_ServerFifo();
            server_fifo = connect_ServerPipe();

            printf("Generating Client FIFO.\n");

            //Make client write FIFO
            char* client_write_pipe = (char*)calloc(MESSAGE_SIZE,sizeof(char));
            strcpy(client_write_pipe,client_list[client_id_index]);
            strcat(client_write_pipe,"_w");

            printf("Attempting to Create Write Pipe: %s\n", client_write_pipe);
            w_client_pipe[client_id_index] = create_ClientFifo(client_write_pipe);
            
            //Make client read FIFO
            char* client_read_pipe = (char*)calloc(MESSAGE_SIZE,sizeof(char));;
            strcpy(client_read_pipe,client_list[client_id_index]);
            strcat(client_read_pipe,"_r");

            printf("Attempting to Create Write Pipe: %s\n", client_write_pipe);
            r_client_pipe[client_id_index] = create_ClientFifo(client_read_pipe);
            
         }      
      }
     
      /* 
       * ------------------- STDIN Detection Block -------------------
 		 *
       *    This block checks the server STDIN  for write activity
 		 *   
 		 *    When write is detected for the server STDIN a series of
 		 *    if statements is executed to determine the correct action.
 		 *
 		 *    Exit - Closes the server
 		 *    Help - Prints commands to STDIN
 		 *    Read - Reads a line from the server FIFO 
 		 *
 		 */

      
      // STDIN Select for Input 
      stdin_set = set_FileSelect_Clear(STDIN_FILENO);
      stdin_read = select(STDIN_FILENO + 1, &stdin_set, NULL, NULL, &read_timeout);
      // Check for commands from STDIN
      if(FD_ISSET(STDIN_FILENO, &stdin_set)){
         
         fgets(command_line, MESSAGE_SIZE, stdin);
         // Exit Command
         if(strncmp(S_COMMAND_EXIT,command_line, strlen(S_COMMAND_EXIT)) == 0){
            printf("Closing Server.\n");
            server_status = 0;

            // Destroy Pipe
            close(server_fifo);
            unlink(SERVER_PIPE);

            // Destroy Client Pipes
            close_ClientFifo_All(client_list, w_client_pipe, r_client_pipe);
 
            // Free MALLOCs
            free(command_line);
            free(fifo_pipe);
         }
         
         // Help Command
         if(strncmp(S_COMMAND_HELP,command_line, strlen(S_COMMAND_HELP)) == 0){
            print_commands();
         }

         // Read Command
         if(strncmp(S_COMMAND_READ, command_line, strlen(S_COMMAND_READ)) == 0){

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
   //if(group_usage[0] == 0){
     //group_members[0][0] = test_context.group_id;   
   //  group_usage[0] = 1;
   //}

   printf("%s\n",group_members[0][0]);
   printf("%s\n",group_members[0][1]);
   printf("%d\n",group_usage[0]);
//   free(client_list);
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

// Creates the master server fifo for connecting clients
int create_ServerFifo(){

   struct stat st;
   int status;
 
   if(stat(SERVER_PIPE, &st) != 0){
      status = mkfifo(SERVER_PIPE, 0666);
      if(status != 0){
         printf("Error: Server Pipe could not be created.\n");
         return -1;
      }
      else{
         printf("Server Pipe Opened\n");
         return status;
      }
   }
}

// Generates generic FIFOs using a pipe name
int create_ClientFifo(char* pipe_name){

   struct stat st;
   int status;

   if(stat(pipe_name, &st) != 0){
      status = mkfifo(pipe_name, 0666);
      if(status != 0){
         printf("Error: Client Pipe could not be created: %s\n", pipe_name);
         return -1;
      }
      else{
         printf("Client Pipe Opened: %s\n", pipe_name);
         return status;
      }
   }
}


// Closes and unlinks a client fifo pair
void close_ClientFifo(char* clientID, int write_desc, int read_desc){

   printf("\n\n--- Closing Pipes for Client: %s ---\n", clientID);

   //Make client write FIFO
   char* client_write_pipe = (char*)calloc(MESSAGE_SIZE,sizeof(char));
   strcpy(client_write_pipe,clientID);
   strcat(client_write_pipe,"_w");

   printf("Closing Write Pipe: %s\n", client_write_pipe);
   close(write_desc);
   unlink(client_write_pipe);
            
   //Make client read FIFO
   char* client_read_pipe = (char*)calloc(MESSAGE_SIZE,sizeof(char));;
   strcpy(client_read_pipe,clientID);
   strcat(client_read_pipe,"_r");

   printf("Closing Read Pipe: %s\n", client_write_pipe);
   close(read_desc);
   unlink(client_read_pipe);
         
   return;
}

// Closes all client fifo pairs
void close_ClientFifo_All(char** client_list, int* write_pipes, int* read_pipes){

   int i;
   for(i = 0; i < CLIENT_MAX; i++){
      if(strcmp(client_list[i],EMPTY_CLIENT) != 0){
         close_ClientFifo(client_list[i], write_pipes[i], read_pipes[i]);
      }
   }
   return;

}
