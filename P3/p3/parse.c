#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>
#include "glob.h"

char *nextToken( char **strptr ){
	char *str = *strptr;

	while( *str != '\0' && isspace( *str ) ){ // passing white space
		str++;
	}
	*strptr = str;
	if( *str == '\0' ){
		//printf( "Null\n" );
		return NULL;
	} else if( *str == '<' || *str == '>' || *str == '|' ){
		*strptr = str + 1;
		char *token = malloc( 2 );
		token[ 0 ] = *str;
		token [ 1 ] = '\0';
		//printf( "token:'%s'\n" , token );
		return token;
	} else {
		while( *str != '\0' && !isspace( *str ) && *str != '<' && *str != '>' && *str != '|' ){
			str++;
		}
		int length = str - *strptr;
		char *token = malloc( length + 1 );

		memcpy( token , *strptr , length );
		token[ length ] = '\0';
		*strptr = str;
		//printf( "token:'%s'\n" , token );
		return token;
	}
}

command *parseCommand( char *line ){
	command *cmd = malloc( sizeof( command ) );
	if( cmd == NULL ){
		fprintf( stderr , "Allocation failed\n" );
		exit( 1 );
	}
	command *result = cmd;

	memset( cmd , 0 , sizeof( command ) );

	cmd -> input = NULL;
	cmd -> output = NULL;
	cmd -> next = NULL;
	cmd -> rawArgCount = 0;

	char *token;

	while( ( token = nextToken( & line ) ) ){
		if( token[ 0 ] == '|' ){
			cmd -> next = malloc( sizeof( command ) );
			memset( cmd -> next, 0 , sizeof( command ) );
			cmd = cmd -> next;
			cmd -> input = NULL;
			cmd -> output = NULL;
			cmd -> rawArgCount = 0;
			free( token );

		} else if( token[ 0 ] == '<' ){
			free( token );
			token = nextToken( & line );
			if( token == NULL || token[ 0 ] == '<' || token[ 0 ] == '>' || token[ 0 ] == '|' ){
				fprintf( stderr , "Expected file name after <\n" );
				return NULL;
			}
			cmd -> input = token;
		} else if( token[ 0 ] == '>' ){
			free( token );
			token = nextToken( & line );
			if( token == NULL || token[ 0 ] == '<' || token[ 0 ] == '>' || token[ 0 ] == '|' ){
				fprintf( stderr , "Expected file name after >\n" );
				return NULL;
			}
			cmd -> output = token;
		} else if( strcmp( token , "then" ) == 0 ){
			if( cmd -> rawArgCount != 0 ){
				fprintf( stderr , "'then' only valid at start of the command\n" );
				return NULL;
			}
			cmd -> cond = CONDITION_THEN;
		} 
		else if( strcmp( token , "else" ) == 0 ){
			if( cmd -> rawArgCount != 0 ){
				fprintf( stderr , "'else' only valid at start of the command\n" );
				return NULL;
			}
			cmd -> cond = CONDITION_ELSE;
		} else {
			cmd -> rawArgs[ cmd -> rawArgCount ] = token;
			cmd -> rawArgCount++;
			// need to check this and error check!!!
		}
	}
	return result;
}
int expandArgs( command *cmd ){
	cmd -> argCapacity = 64;
	cmd -> args = malloc( sizeof( char * ) * cmd -> argCapacity );
	cmd -> argCount = 0;
	for( int i = 0; i < cmd -> rawArgCount; i++ ){
		expand_wildcard( cmd -> rawArgs[ i ] , &cmd -> args , &cmd -> argCount , &cmd -> argCapacity , NULL );
	}
	return 0;
}
