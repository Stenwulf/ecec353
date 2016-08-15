#ifndef CHAT_H
#define CHAT_H

#define MESSAGE_SIZE         1024

#define SERVER_PIPE          "./serverfifo"
#define S_PIPE_PERMISSIONS   0777

#define S_COMMAND_EXIT       "/exit"
#define S_COMMAND_HELP       "/help"
#define S_COMMAND_READ       "/read"

#define C_COMMAND_EXIT       "/exit"
#define C_COMMAND_GROUP      "/group"

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
//
//    peer_flag - Flag to indicate if the message should be a whisper

struct message_struct{

   char* client_id;
   char* message_text;

   int peer_flag;

};


#endif
