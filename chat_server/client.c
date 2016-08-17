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
void build_MessageGroup(char* command_line, char* clientID, char* groupID);
void build_MessageWhisper(char* command_line, char* clientID, char* whispID);

int write_MessageToPipe(char* message, int pipe);

int main(int argc, char *argv[]){
 
   // Initialize loop iterators
   int i;
   int j;
   int client_status;
   int server_fifo_status;
   int stdin_status;
   int client_w_status;
   int client_r_status;
   // Command Line Args
   char* groupID;
   char* clientID;

   // Command Messages
   char* command_line = (char*)calloc(MESSAGE_SIZE, sizeof(char));
   char* command_message = (char*)calloc(MESSAGE_SIZE, sizeof(char));
   char* client_pipe_w = (char *)calloc(MESSAGE_SIZE, sizeof(char)); 
   char* client_pipe_r = (char *)calloc(MESSAGE_SIZE, sizeof(char));

   char* exit_command = (char*)calloc(MESSAGE_SIZE, sizeof(char));
   strcpy(exit_command, "xxxx");

   int server_fifo;
   int write_fifo;
   int read_fifo;

   int connect_count;
   int connect_status;

   fd_set stdin_set;
   fd_set server_fifo_write;
   fd_set client_read;
   fd_set client_write;

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
      printf("Server Not Online, Closing Client.\n");
      return 0;
   }
   else{
      printf("Pipe open with value: %d\n",server_fifo);
      build_JoinGroup(command_line,clientID,groupID);
      client_w_status = write(server_fifo, command_line, MESSAGE_SIZE*sizeof(char));
      close(server_fifo);
      printf("Wrote join: %d\n", client_w_status);

      // Build Pipe Names         
      build_ClientRead_ID(client_pipe_r,clientID);
      build_ClientWrite_ID(client_pipe_w,clientID);
      sleep(1); 
      //usleep(100);
      read_fifo = open(client_pipe_r, O_NONBLOCK | O_RDONLY);
      write_fifo = open(client_pipe_w, O_WRONLY);

      printf("Write Pipe open with value: %d\n",write_fifo);
      printf("Read Pipe open with value: %d\n",read_fifo);
   }

   

   printf("Client: %s. Connected to Group: %s.\n", clientID, groupID);
   while(client_status){
      if(read_fifo != -1){   
         client_read = set_FileSelect_Clear(read_fifo);
         client_r_status = select(read_fifo+1, &client_read, NULL, NULL, &read_timeout);
         if(FD_ISSET(read_fifo, &client_read)){

            printf("Got data from server.\n");
            read(read_fifo,command_line, MESSAGE_SIZE*sizeof(command_line));
            printf("Message: %s\n", command_line);
            if(strcmp(command_line, exit_command) == 0){
               close(read_fifo);
               printf("Closed Read Pipe.\n");
               read_fifo = -1;
            }

         }
        }  

      stdin_set = set_FileSelect_Clear(STDIN_FILENO);
      stdin_status = select(STDIN_FILENO+1, &stdin_set, NULL, NULL, &read_timeout); 
      if(FD_ISSET(STDIN_FILENO, &stdin_set)){
         // Get input
         fgets(command_line, MESSAGE_SIZE, stdin);
         // Exit Command
         if(strncmp(C_COMMAND_EXIT,command_line, 2) == 0){
            printf("Closing Server.\n");
            client_status = 0;
            close(write_fifo);
            close(read_fifo);

            free(command_line);
            free(command_message);
         }
         
         // Join Group Command
         if(strncmp(C_COMMAND_GROUP,command_line, 2) == 0){
            build_MessageGroup(command_line, clientID, groupID);
            printf("%s\n",command_line);
            client_w_status = write(write_fifo, command_line, MESSAGE_SIZE*sizeof(char));
            printf("Wrote to file: %d.\n", client_w_status);
            memset(command_line, 0, MESSAGE_SIZE);
         }
/*         if(strncmp("/w",command_line, 2) == 0){
            command_message = strtok(command_line, "|");
            printf("%s\n", command_message);
            command_message = strtok(NULL, "|");      
            printf("%s\n", command_message);
            build_MessageWhisper(command_line, clientID, command_message);
            printf("%s\n",command_line);
            client_w_status = write(write_fifo, command_line, MESSAGE_SIZE*sizeof(char));
            printf("Wrote to file: %d.\n", client_w_status);
            memset(command_line, 0, MESSAGE_SIZE);
         }*/


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

int write_MessageToPipe(char* message, int pipe){

   int status;
   status = write(pipe, message, MESSAGE_SIZE*sizeof(char));
   if(status < 0){
      printf("Error Sending Message");
   }
   return status;
}

void build_MessageGroup(char* command_line, char* clientID, char* groupID){

   char message_buff[MESSAGE_SIZE];
   memcpy(message_buff, &command_line[3],(MESSAGE_SIZE*sizeof(char)) -3);
   memset(command_line, 0, MESSAGE_SIZE);

   strcpy(command_line,COMMAND_GCHAT);// Add command to head

   // Add client and group to string
   strcat(command_line,groupID);
   strcat(command_line,COMMAND_DELIM);

   strcat(command_line,clientID);
   strcat(command_line,COMMAND_DELIM);

   strcat(command_line, message_buff);
   
}

void build_MessageWhisper(char* command_line, char* clientID, char* whispID){

   char message_buff[MESSAGE_SIZE];
   int message_offset;

   message_offset = 4+ strlen(whispID);
   memcpy(message_buff, &command_line[message_offset],(MESSAGE_SIZE*sizeof(char)));
   memset(command_line, 0, MESSAGE_SIZE);

   strcpy(command_line,COMMAND_GCHAT);// Add command to head

   // Add client and group to string
   strcat(command_line,whispID);
   strcat(command_line,COMMAND_DELIM);

   strcat(command_line,clientID);
   strcat(command_line,COMMAND_DELIM);

   strcat(command_line, message_buff);
   
}
