
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

#include "chat.h"


int main(int argc, char *argv[]){
 
   // Initialize loop iterators
   int i;
   int j;

   if (argc != 3) {
      fprintf(stderr,"The program must have 2 arguments (groupId and userName).\n");      
      exit(1)
   }
   else {
      groupId = argv[1]
      userName = argv[2]
   }
