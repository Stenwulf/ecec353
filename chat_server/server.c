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
int largest_FileNum(int* read_pipes);

void close_ClientFifo(char* clientID, int write_desc, int read_desc);
void close_ClientFifo_All(char** client_list, int* write_pipes, int* read_pipes);


fd_set set_ReadFifoSet(int* read_pipes);
//fd_set set_FileSelect_Server(int filedesc);
//fd_set set_FileSelect_STDIN();

int main(int argc, char **argv){
 
   // Initialize loop iterators
   int i;
   int j;
   int k;

   // Initalize Status Indicators
   int status; 
   int server_status;
   int server_read;
   int server_write;
   int stdin_read;
   int establishing;
   int largest_fifo;


   // Intalize Client Index Ints
   int client_id_index;
   int group_id_index;

   // Select Parameters
   fd_set s_read_set;
   fd_set stdin_set;
   fd_set write_set;

   fd_set pipe_r_set;
   fd_set pipe_w_set;

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
         group_members[i][j] = calloc(MESSAGE_SIZE, sizeof(char));
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
   server_fifo = connect_ServerPipe_Read();
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
            //   Commented Version copies 1 instead of client_id name
            //   group_members[group_id_index][client_id_index] = 1;
            strcpy(group_members[group_id_index][client_id_index],client_list[client_id_index]);
 
            /*
             * Create Client FIFO Block
             *
             * Generates Two FIFOs for reading and writing with client
             * in the following structures:
             *    w_client_pipe
             *    r_client_pipe
             *
             **/

            // Wait for disconnect mesage
            close(server_fifo);
            unlink(SERVER_PIPE);
            // Regenerate Fifo
            status = create_ServerFifo();
            server_fifo = connect_ServerPipe_Read();

            printf("Generating Client FIFO.\n");

            //Make client write FIFO
            char* client_write_pipe = (char*)calloc(MESSAGE_SIZE,sizeof(char));
            build_ClientWrite_ID(client_write_pipe, client_list[client_id_index]);
            
            //Make client read FIFO
            char* client_read_pipe = (char*)calloc(MESSAGE_SIZE,sizeof(char));;
            build_ClientRead_ID(client_read_pipe, client_list[client_id_index]);

            // Create both pipes
            r_client_pipe[client_id_index] = create_ClientFifo(client_read_pipe);
            w_client_pipe[client_id_index] = create_ClientFifo(client_write_pipe); 
            // Connect to reading end of pipe
            //printf("Attempting to Read Client Write Pipe: %s\n", client_write_pipe);
            w_client_pipe[client_id_index] = open(client_write_pipe,  O_RDWR);//connect_ClientPipe_Server(client_write_pipe);
            //   sleep(1);
            r_client_pipe[client_id_index] = open(client_read_pipe,  O_RDWR);//;onnect_ClientPipe_Server(client_read_pipe);

            printf("Client %s write pipe file: %d\n",client_list[client_id_index], w_client_pipe[client_id_index]);
            printf("Client %s read pipe file: %d\n",client_list[client_id_index], r_client_pipe[client_id_index]);
            largest_fifo = largest_FileNum(w_client_pipe);
         }      
      }

           
      pipe_r_set = set_ReadFifoSet(w_client_pipe);
      server_read = select(largest_fifo + 1, &pipe_r_set, NULL, NULL, &read_timeout);
      for(i = 0; i < CLIENT_MAX; i++){
         if(FD_ISSET(w_client_pipe[i], &pipe_r_set)){
            printf("Server Got Pipe Data on %d file %d%\n", i, w_client_pipe[0]);
            memset(command_line, 0, MESSAGE_SIZE);
            read(w_client_pipe[i], command_line, MESSAGE_SIZE*sizeof(char));
            printf("Client %d Says: %s",i , command_line);

            command_token = strtok(command_line,COMMAND_DELIM);
            if(strcmp(command_line,"gc") == 0){
               printf("Got group message\n");
               command_token = strtok(NULL, COMMAND_DELIM);
               for(j = 0; j < GROUP_MAX; j++){
                  if(strcmp(group_list[j], command_token) == 0){
                     char broadcast[MESSAGE_SIZE];
                     command_token = strtok(NULL, COMMAND_DELIM);
                     strcpy(broadcast, command_token);
                     strcat(broadcast," :: ");
                     command_token = strtok(NULL, COMMAND_DELIM);
                     strcat(broadcast, command_token);
                     for(k = 0; k < CLIENT_MAX; k++){
                        if(strcmp(group_members[j][k], EMPTY_CLIENT)!=0){
                            printf("Found Client.\n");
                            write(r_client_pipe[k], broadcast, MESSAGE_SIZE*sizeof(char)); 
                        }
                     }
                  }
               }   
            }

            if(strcmp(command_line,"w") == 0){
               printf("Got whisper message\n");
               command_token = strtok(NULL, COMMAND_DELIM);
               for(j = 0; j < GROUP_MAX; j++){
                  if(strcmp(group_list[j], command_token) == 0){
                     char broadcast[MESSAGE_SIZE];
                     command_token = strtok(NULL, COMMAND_DELIM);
                     strcpy(broadcast, command_token);
                     strcat(broadcast," :: ");
                     command_token = strtok(NULL, COMMAND_DELIM);
                     strcat(broadcast, command_token);
                     for(k = 0; k < CLIENT_MAX; k++){
                        if(strcmp(group_members[j][k], EMPTY_CLIENT)!=0){
                            printf("Found Client.\n");
                            write(r_client_pipe[k], broadcast, MESSAGE_SIZE*sizeof(char)); 
                        }
                     }
                  }
               }   
            }
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

            strcpy(command_line, "xxxx");
            // Destroy Client Pipes
            for(i = 0; i < CLIENT_MAX; i++){
               if(r_client_pipe[i] > 0){
                  server_write = write(r_client_pipe[i], command_line, MESSAGE_SIZE*sizeof(command_line));
                  printf("Write Operation: %d\n",server_write);
                  printf("File Desc: %d\n",r_client_pipe[i]);
               }
            }
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
            read(w_client_pipe[0], command_line, MESSAGE_SIZE*sizeof(command_line));
            printf("Command: %s\n", command_line);
         }

         if(strncmp("/write", command_line, strlen(S_COMMAND_READ)) == 0){

            printf("File Opened\n");
            //memset(command_line, 0 , MESSAGE_SIZE);
            server_write = write(r_client_pipe[0], command_line, MESSAGE_SIZE*sizeof(char));
            printf("Write: %d", server_write);
         }
         
         fflush(stdin);
      }
      
   }

   return 0;

}

// Print Commands
//    Prints a list of the server commands
void print_commands(){

   printf("\n\nCommand List:\n");
   printf("/help -- Prints this list of commands\n");
   printf("/exit -- Exits the server\n");
//   printf("/read -- Test Command :: Read Line from FIFO\n");

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
   int file_desc;

   if(stat(pipe_name, &st) != 0){
      status = mkfifo(pipe_name, 0666);
      if(status != 0){
         printf("Error: Client Pipe could not be created: %s\n", pipe_name);
         return -1;
      }
      printf("FIFO Created: %s\n", pipe_name);
      return status;
   }
}


// Closes and unlinks a client fifo pair
void close_ClientFifo(char* clientID, int write_desc, int read_desc){

   int status;
   printf("\n\n--- Closing Pipes for Client: %s ---\n", clientID);

   status = write(read_desc, "xxxx", MESSAGE_SIZE*sizeof(char));
   printf("Closing Client %s with write status %d\n", clientID, status);
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

   printf("Closing Read Pipe: %s\n", client_read_pipe);
   close(read_desc);
   unlink(client_read_pipe);
   
   printf("------------------------------------\n\n");     
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

int largest_FileNum(int* read_pipes){
   int i = 0;
   int max = 0;
   for(i = 0; i < CLIENT_MAX; i++){
      if(read_pipes[i] > max){
         max = read_pipes[i];
      }

   }
   return max;
}

fd_set set_ReadFifoSet(int* read_pipes){
   
   int i;
   fd_set read_all = set_FileSelect_Clear(read_pipes[0]);
   for(i = 1; i < CLIENT_MAX; i++){
      if(read_pipes[i] != 0){
         read_all = set_FileSelect_NoClear(read_all,read_pipes[i]);
      }
   }
   return read_all;

}

