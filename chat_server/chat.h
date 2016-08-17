#ifndef CHAT_H
#define CHAT_H
// Max message size
#define MESSAGE_SIZE         1024
// Pipe Constants
#define SERVER_PIPE          "./serverfifo"
#define R_PIPE_CONSTRUCT     "_r"
#define W_PIPE_CONSTRUCT     "_w"
#define S_PIPE_PERMISSIONS   0777

// Command Delimiter
//#define COMMAND_DELIM        " xx "
#define COMMAND_JOIN         "j|"
#define EMPTY_CLIENT         "empty"
// Server Command Input
#define S_COMMAND_EXIT       "/exit"
#define S_COMMAND_HELP       "/help"
#define S_COMMAND_READ       "/read"
// Join Group
#define J_CLIENT_GROUP       "j"
#define J_CLIENT_CONN        "..."
//Client Command Inputs
#define C_COMMAND_EXIT       "/x"
#define C_COMMAND_GROUP      "/g"


#define PIPE_READONLY        33
#define PIPE_WRITEONLY       44

#define COMMAND_GCHAT         "gc|"
#define COMMAND_WCHAT         "w|"
#define COMMAND_CEXIT         "x|"

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

void build_ClientWrite_ID(char* pipe_name, char* client_id);
void build_ClientRead_ID(char* pipe_name, char* client_id);

int connect_ServerPipe_Read();
int connect_ClientPipe_Server(char* path);
int connect_ClientPipe_Read(char* path);
int connect_ClientPipe_Write(char* path);

static char* COMMAND_DELIM = "|";

fd_set set_FileSelect_Clear(int filedesc);
fd_set set_FileSelect_NoClear(fd_set fds, int filedesc);

#endif
