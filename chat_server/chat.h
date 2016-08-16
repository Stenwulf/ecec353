#ifndef CHAT_H
#define CHAT_H

#define MESSAGE_SIZE         1024

#define SERVER_PIPE          "./serverfifo"
#define S_PIPE_PERMISSIONS   0777

#define S_COMMAND_EXIT       "/exit"
#define S_COMMAND_HELP       "/help"
#define S_COMMAND_READ       "/read"
#define S_CLIENT_GROUP       "g"

#define C_COMMAND_EXIT       "/x"
#define C_COMMAND_GROUP      "/g"

// Group Context
//    group_id - Containts the group id that the client will connect to
//    client_id - Name/ID of the client that is connecting to the group

struct group_context{

   char* group_id;
   char* client_id;  

};

// Message
//    client_id - Name/ID of the client that is sending the message
//    message_text - THe contents of the message being sent

struct message_struct{

   char* client_id;
   char* message_text;

};

// Command Token
//    command - the command from the message
//    contents - the value of the command

struct command_token{
   
   char* command;
   char* contents;
};

int connect_ServerPipe();

fd_set set_FileSelect_Clear(int filedesc);
fd_set set_FileSelect_NoClear(fd_set fds, int filedesc);

#endif
