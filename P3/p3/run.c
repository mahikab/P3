#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/limits.h>
#include <sys/stat.h>

int isBuiltIn( char *token ){
	return strcmp( token, "cd" ) == 0 || strcmp( token , "pwd" ) == 0 || strcmp( token , "which" ) == 0 || strcmp( token, "exit" ) == 0;
}

void abortCommand( command *cmd ){
	if( cmd -> fdInput != 0 ){
		close( cmd -> fdInput );
	}
	if( cmd -> fdOutput != 0 ){
		close( cmd -> fdOutput );
	}
	if( cmd -> next != NULL ){
		abortCommand( cmd -> next );
	}
}

int initializeCommand( command *cmd ){
	if( cmd -> input != NULL ){
		if( cmd -> fdInput != 0 ){
			fprintf( stderr , "Input redirection when command has input pipe\n");
			abortCommand( cmd );
			return -1;
		}
		// cmd -> fdInput = open( cmd -> input , O_RDONLY );
		// if( cmd -> fdInput < 0 ){
		// 	perror( "Unable to open file" );
		// 	abortCommand( cmd );
		// 	return -1;
		// }
	}
	// if( cmd -> output != NULL ){
	// 	cmd -> fdOutput = open( cmd -> output , O_WRONLY | O_CREAT );
	// 	if( cmd -> fdOutput < 0 ){
	// 		perror( "Unable to open file" );
	// 		abortCommand( cmd );
	// 		return -1;
	// 	}
	// }
	if( cmd -> next != NULL ){
		if( cmd -> output != NULL ){
			fprintf( stderr , "Output redirectiong when command has output pipe\n" );
			abortCommand( cmd );
			return -1;
		}
		int pipefd[ 2 ];
		if( pipe( pipefd ) == 0 ){
			cmd -> fdOutput = pipefd[ 1 ];
			cmd -> next -> fdInput = pipefd[ 0 ];
			return initializeCommand( cmd -> next );
		} else {
			perror( "Unable to create pipe" );
			abortCommand( cmd );
			return -1;
		}
	}
	return 0;
}

int executeBuiltIn( command *cmd , int isInteractive ) {
	if ( strcmp( cmd -> args[ 0 ] , "cd" ) == 0 ){
		char *token = cmd -> args[ 1 ];
		char *newDirectory = NULL;

		if ( cmd -> argCount <= 1 ) {
			fprintf( stderr , "cd: expected argument\n");
			return 1;
		} else if ( cmd -> argCount > 2 ) {
			fprintf( stderr , "cd: too many arguments\n");
			return 1;
		} else if( strcmp( token , "-" ) == 0 ){
			// change to the previous working directory
			char *prev_dir = getenv( "OLDPWD" );
			if( prev_dir != NULL ) {
				newDirectory = prev_dir;
				if ( chdir( prev_dir ) != 0 ) {
					fprintf( stderr , "cd: failed to changed the directory to '%s'\n" , prev_dir );
					return 1;
				}
			} else {
				fprintf( stderr , "cd: old pwd environment variable not set\n" );
				return 1;
			}
		} else {
			newDirectory = token;
		} 
		char current_dir[ PATH_MAX ];
		if ( getcwd( current_dir , sizeof( current_dir ) ) != NULL ){
			if( setenv( "OLDPWD" , current_dir , 1 ) != 0 ){
				perror( "cd: Unable to set OLDPWD" );
				return 1;
			}
		} else {
			perror( "cd: Cannot get PWD" );
			return 1;
		} 
		if( chdir( newDirectory ) != 0 ){
				perror( "cd: Failed to change the directory" );
				return 1;
		}
		if( getcwd( current_dir , sizeof( current_dir ) ) != NULL ){
			if( setenv( "PWD" , current_dir , 1 ) != 0 ){
				perror( "cd: Unable to set PWD" );
				return 1;
			}
		} else {
			perror( "cd: Cannot get PWD" );
			return 1;
		} 
		return 0;

	} else if( strcmp( cmd -> args[ 0 ] , "pwd" ) == 0 ){
		if( cmd -> argCount != 1 ){
			fprintf( stderr , "pwd: Too many arguments\n" );
			return 1;
		}
		char current_dir[ PATH_MAX ];

		if( getcwd( current_dir , sizeof( current_dir ) ) != NULL ){
			printf( "%s\n" , current_dir );
		} else {
			perror( "pwd" );
			return 1;
		}
		return 0;
	} else if( strcmp( cmd -> args[ 0 ] , "which" ) == 0 ) {
		if( cmd -> argCount <= 1 ) {
			fprintf( stderr , "which: expected argument\n");
			return 1;
		} else if( cmd -> argCount > 2 ) {
			fprintf( stderr , "which: too many arguments\n");
			return 1;
		} 

		char *token = cmd -> args[ 1 ];
		
		if( token != NULL ) {
			if( isBuiltIn( token ) ){
				printf( "which: %s is a built in command\n" , token );
			} else {
				char *path = getenv( "PATH" );

				if( path == NULL ){
					fprintf( stderr , "which: PATH environment variable not set\n" );
					return 1;
				}

				while( *path != '\0' ){
					if( *path == ':' ){
						path++;
					}
					char *pathEnd = strchr( path , ':' );
					if( pathEnd == NULL ){
						pathEnd = strchr( path , '\0' );
					}
					char program_path[ PATH_MAX ];
					int length = pathEnd - path;

					snprintf( program_path , sizeof( program_path ) , "%.*s/%s" , length , path , token );
					if( access( program_path , X_OK ) == 0 ){
						printf( "%s\n" , program_path );
						return 0;
					}
					path = pathEnd;
				}
				fprintf( stderr , "which: %s not found in PATH\n" , token );
				return 1;
			}
		} 
		return 0;
	} else if( cmd -> argCount > 0 && strcmp( cmd -> args[ 0 ] , "exit" ) == 0 ){
		char *message = "It was nice interacting with you!\n";
		if( cmd -> argCount > 2 ){
			fprintf( stderr , "exit: Too many arguments\n" );
			return -1;
		} else if( cmd -> argCount == 2 ){
			if( isInteractive ){
				puts( message );
			}
			exit( strtol( cmd -> args[ 1 ] , NULL , 10 ) );
		} else {
			if( isInteractive ){
				puts( message );
			}
			exit( 0 );
		}
	} else {
		return 2; // if did not process as built in
	}
}

void execute_command( command *cmd ){
    pid_t pid = fork();

    if( pid == -1 ){
        perror( "fork" );
        exit( EXIT_FAILURE );
    } else if( pid == 0 ) {
		if( cmd -> input != NULL ){
			cmd -> fdInput = open( cmd -> input , O_RDONLY | __O_CLOEXEC , S_IRWXU | S_IRGRP );
			if( cmd -> fdInput < 0 ){
				perror( "Unable to open file" );
				exit( 1 );
			}
		}
		if( cmd -> output != NULL ){
			cmd -> fdOutput = open( cmd -> output , O_WRONLY | O_CREAT | __O_CLOEXEC , S_IRWXU | S_IRGRP );
			if( cmd -> fdOutput < 0 ){
				perror( "Unable to open file" );
				exit( 1 );
			}
			if( dup2( cmd -> fdOutput , STDOUT_FILENO ) < 0 ){
				perror( "Unable to duplicate file descriptor" );
				exit( 1 );
			}
			close( cmd -> fdOutput );
			cmd -> fdOutput = 0;
		}
		if( cmd -> fdInput != 0 ){
			if( dup2( cmd -> fdInput , STDIN_FILENO ) < 0 ){
				perror( "Unable to duplicate file descriptor" );
				exit( 1 );
			}
			close( cmd -> fdInput );
			cmd -> fdInput = 0;
		}
		if( cmd -> fdOutput != 0 ){
			if( dup2( cmd -> fdOutput , STDOUT_FILENO ) < 0 ){
				perror( "Unable to duplicate file descriptor" );
				exit( 1 );
			}
			close( cmd -> fdOutput );
			cmd -> fdOutput = 0;
		}
		execvp( cmd -> args[ 0 ] , cmd -> args );

        // prints the command and why it failed 
        fprintf( stderr , "Failed to execute command '%s': %s\n" , cmd -> args[ 0 ] , strerror( errno ) );

        perror( "execv" );
        exit( EXIT_FAILURE );
    } else {
        cmd -> pid = pid;
		// if( cmd -> fdOutput != 0 ){
		// 	close( cmd -> fdOutput );
		// }
		//fprintf( stderr , "child pid: %d\n" , pid );
		if( cmd -> fdInput != 0 ){
			close( cmd -> fdInput );
		}
		if( cmd -> fdOutput != 0 ){
			close( cmd -> fdOutput );
		}
    }
}

int isBuiltInPresent( command *cmd ){
	while( cmd != NULL ){
		if( isBuiltIn( cmd -> args[ 0 ] ) ){
			return 1;
		}
		cmd = cmd -> next;
	}
	return 0;
}

int execute( command *cmd , int isInteractive ){
	int status = 0;
	int wstatus;
	command *iter = cmd;
	int runningCount = 0;

	if( isBuiltInPresent( cmd ) ){
		if( cmd -> next != NULL ){
			fprintf( stderr , "Cannot execute built-in in a pipeline\n" );
			return 1;
		}
		if( cmd -> input != NULL || cmd -> output != NULL ){
			fprintf( stderr , "Cannot use re-directions with a built-in\n" );
			return 1;
		}
		return executeBuiltIn( cmd , isInteractive );
	}
	if( initializeCommand( cmd ) < 0 ){
		return 1;
	}

	while( iter != NULL ){
		iter -> args[ iter -> argCount ] = NULL;
		execute_command( iter );
		iter = iter -> next;
	}
	iter = cmd;

	while( iter != NULL ){
		runningCount++;
		iter = iter -> next;
	}
	while( runningCount > 0){
		pid_t pid = wait( &wstatus );

		if( pid == cmd -> pid ){
			status = WEXITSTATUS( wstatus );
		}
		if( pid < 0 ){
			perror( "wait" );
			exit( EXIT_FAILURE );
		}
		iter = cmd;
		while( iter != NULL ){
			if( iter -> pid == pid ){
				runningCount--;
				iter -> pid = 0;
			}
			iter = iter -> next;
		}
	}
	return status;
}
