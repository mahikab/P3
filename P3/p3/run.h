#pragma once
#include "parse.h"

int execute( command *cmd , int isInteractive );
int maybe_exit( command *cmd , int isInteractive );