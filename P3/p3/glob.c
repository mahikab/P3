#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include "parse.h"
#include "run.h"
#include <linux/limits.h>

int matchGLob( char *glob , char *str ){
    // if( str[ 0 ] == '.' && glob[ 0 ] != '.' ){
    //     return 0;
    // }
    // if( str[ 0 ] != '.' && glob[ 0 ] == '.' ){
    //     return 0;
    // }
    int i = 0;
    for( ; str[ i ] && *glob; i++ ){
        if( *glob == '*' ){
            break;
        }
        if( str[ i ] != *glob ){
            return 0;
        }
            glob++;
    } 
    if( !*glob ){
        if( str[ i ] ){
            return 0;
        } else {
            return 1;
        }
    }  
    glob++;
    if( !*glob ){
        return 1;
    }
    for( int k = i; str[ k ]; k++ ){
        if( matchGLob( glob , str + k ) ){
            return 1;
        }
    }
    return 0; 
}

//for wildcards
int expand_wildcard( char *token , char ***args , int *arg_count , int *arg_capacity , char *base_name ) {
    char *pattern = strchr( token, '/' );

    // if ( pattern == NULL ) {
    //     // No wildcard, add the token as is
    //     if ( *arg_count == *arg_capacity ) {
    //         *arg_capacity *= 2;
    //         *args = realloc( *args , *arg_capacity * sizeof( char * ) );
    //     }
    //     ( *args )[ ( *arg_count )++ ] = strdup( token );
    //     return;
    // }
    if( base_name == NULL ){
        if( token[ 0 ] == '/' ){
            base_name = strdup( "/" );
        } else {
            base_name = strdup( "." );
        }
    }
    
    // Extract the directory path and base name
    char *glob = strndup( token , pattern - token );
    DIR *dir = opendir( base_name );

    if( dir == NULL ) {
        perror( "opendir" );
        free( glob );
        return -1;
    }
    struct dirent *entry;
    int matches = 0;

    while( ( entry = readdir( dir ) ) != NULL) {
        if( matchGLob( glob , entry -> d_name ) ) {
            if( pattern == NULL ){
                if( *arg_count == *arg_capacity ) {
                    *arg_capacity *= 2;
                    *args = realloc( *args , *arg_capacity * sizeof( char * ) );
                }
                char path[ PATH_MAX ];

                snprintf( path , sizeof( path ) , "%s/%s" , base_name , entry -> d_name );
                ( *args )[ ( *arg_count )++ ] = strdup( path );
                matches++;
            } else {
                char path[ PATH_MAX ];

                snprintf( path , sizeof( path ) , "%s/%s" , base_name , entry -> d_name );
                matches += expand_wildcard( pattern + 1 , args , arg_count , arg_capacity , path );
            }
        }
    }
    if( matches == 0 ) {
        // No matches, add the original token
        if ( *arg_count == *arg_capacity ){
            *arg_capacity *= 2;
            *args = realloc( *args , *arg_capacity * sizeof( char * ) );
        }
        ( *args )[ ( *arg_count )++ ] = strdup( token );
    }
    free( glob );
    closedir( dir );
    return matches;
}
