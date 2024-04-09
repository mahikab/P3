#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include "parse.h"
#include "run.h"

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_COUNT 64
#define MAX_PATH_LENGTH 1024

int process_command( char *string , int isInteractive , int status ){
    command *cmd = parseCommand( string );

    if( cmd == NULL ){
        fprintf( stderr , "Could not parse command\n" );
        return 1;
    }
    command *iter = cmd;
    while( iter != NULL ){
        expandArgs( iter );
        iter = iter -> next;
    }

    if( status == 0 && cmd -> cond == CONDITION_ELSE ){
        return status;
    }
    if( status != 0 && cmd -> cond == CONDITION_THEN ){
        return status;
    }
    return execute( cmd , isInteractive );
}

void interactive_mode(){
    int status = 0;

    printf( "Hello, running in interactive_mode\n" );
    char input[ MAX_INPUT_SIZE ];

    while( 1 ) {
        printf( "mysh> " );
        if( fgets( input , MAX_INPUT_SIZE , stdin ) == NULL ){
            break;
        }
        input[ strcspn( input , "\n" ) ] = '\0';

        if( strcmp(input, "exit" ) == 0 ){
            break;
        }
        status = process_command( input , 1 , status );
    }
}

void batch_mode( FILE *batch_file ){
    int status = 0;
    //printf( "Running in batch_mode\n" );
    if ( batch_file == NULL ){
        perror( "Error opening batch file" );
        exit( EXIT_FAILURE );
    }
    char input[ MAX_INPUT_SIZE ];

    while( fgets( input , MAX_INPUT_SIZE , batch_file ) != NULL ){
        input[ strcspn( input , "\n" ) ] = '\0';
        status = process_command( input , 0 , status );
    }
}

int main( int argc , char *argv[] ){
    if( isatty( STDIN_FILENO ) && argc == 1 ) {
        interactive_mode();
    } else if( argc == 2 ){
        FILE *batch_file = fopen( argv[ 1 ] , "r" );
        batch_mode( batch_file );
        fclose( batch_file );
    } else if( argc == 1 ){
        batch_mode( stdin );
    } else {
        fprintf( stderr , "Usage: %s [batch_file]\n" , argv[ 0 ] );
        exit( EXIT_FAILURE );
    }
    return 0;
}