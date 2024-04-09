#pragma once
#include <sys/types.h>

#define MAX_ARGS 1000

typedef enum condition{
   CONDITION_NONE = 0,
   CONDITION_THEN ,
   CONDITION_ELSE

}condition;

typedef struct command{
   char *input;
   char *output;
   int fdInput;
   int fdOutput;
   struct command *next;
   pid_t pid;
   condition cond;
   char *rawArgs[ MAX_ARGS ];
   int rawArgCount;
   char **args;
   int argCount;
   int argCapacity;
}command;

int expandArgs( command *cmd );
command *parseCommand( char *line );
