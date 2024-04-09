# P3
Mahika Busireddy (mb2164) Veena Vrushi (vv274)


For this project, we created a simple shell program (named "mysh"). Our shell supports 
both batch and interactive modes of operation and has multiple functionalities. Our code 
can execute commands, handle input and output redirection, implement conditional execution 
("then" and "else"), support wildcard expansion, and execute built-in commands (“ls”, "cd", 
"pwd", "which", "echo", and "exit"). Within our code we used both dynamic arrays and a 
created command struct('command'). Our command struct represents a single command with input/
output redirection and conditional execution flags. For the input/output redirection 
functionality, we made it so the user can redirect command input from files (<) and output to 
files (>). 

To test our code, we have test cases created in the tests directory and runtests.sh which is 
similar to a makefile for the tests. These test cases are created to ensure the behavior and 
accuracy of our code. We also created additional test scripts('test*.sh) to ensure the 
accuracy of the shell when wildcard expansion and conditional execution are involved.
The test.sh(test1.sh, test2.sh, etc.) files are our test cases and the equivalent test.out
(test1.sh, test2.sh, etc.) files are the expected outputs. We ran the code both interactively 
(./mysh) as well as by doing a make test in the terminal, which runs all test cases 
simultaneously. 


Without our program we have 8 files (including the Makefile and not including any test 
files). In glob.c, we implement wildcard expansion functionality through the function 
expand_wildcard(). We use a recursive algorithm to match filenames against glob patterns, 
thus enabling our shell to handle wildcard characters (* and ?) in command arguments. In the 
file run.c, we include functions related to executing commands (built-in + external), 
handling input/output redirection, and managing command execution flow. In the file parse.c 
we implement the parsing functionality and include our functions for parsing user input into 
commands, identifying the input/output redirection, and detecting conditional execution 
statements. In addition we also have three header files (parse.h, run.h, glob.h)that contain 
the function definition and structs for parsing functionalities, command execution 
functionalities, and wildcard expansion functionalities (respectively). In addition, we have 
main.c which is the entry point of our shell program. This file contains the main function 
that determines whether or not the shell will run interactively or in batch mode (based on 
command-line arguments). Also, main.c will handle user input and will invoke functions to 
parse and commands and execute them. Commands such as ls, pwd, echo, etc. can be used. 


