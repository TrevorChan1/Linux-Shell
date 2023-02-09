# Challenge 1: Shell

This is a project for EC440: Operating Systems to build a shell in the REPL format using the C language to better understand Unix operating systems processes. This project consists of 2 main source files: myshell_parser.c which contains the functions for evaluating strings into pipeline datatypes, and myshell.c which contains the general REPL layout of the shell.

## Parser: myshell_parser.c

This file contains the pipeline_build and pipeline_free commands, used for building pipelines from an inputted string and for freeing the dynamically allocated memory in a pipeline respectively. 

### pipeline_build

pipeline_build makes a lot of use of the strtok and strtok_r commands. The basic layout of the function is to take the input string, and if it's not empty it will continuously loop through the input using the delimiters '|', '&', '<,' or '>.' Anytime it encounters one of these special tokens, the code will parse through the command arguments found by strtoking through the currently separated portion of command using whitespace delimiters. Essentially, the code loops through the string until it's empty by sectioning off parts of the string based on the special tokens, and then parsing each section into word tokens by tokenizing using whitespace characters. 

The function keeps track of which delimiter was used by using the length of the current sectioned off part of the command, and based on which delimiter was used the function will choose what to do next:

#### Pipe: |

When the pipe token is the delimiter, the function will dynamically allocate space for another pipeline_command datatype, set the current command's next piped command to the new command, and set the operating currentCommand pointer to the new command. This essentially prepares the function to parse the next set of command arguments. It will exit and print an error if a piped command is empty.

#### Background: &

When the background token is set, the function assumes it is the end of the command (as per the instructions), sets the pipeline to is_background true, and returns the pipe.

#### Redirect in or Redirect out: < or >

When the function is set to have redirects, it first parses to get the filename. If there are any more word tokens after the filename that are not separated by a special token, it returns an error. For <, the function checks that the current command in the pipe is the first command and returns an error if not. For >, the function checks that the current command in the pipe is the last command in the pipe (rest of command is empty) and returns an error if not. Afterwards, the function dynamically allocates memory for and stores the filename into redirect_in_path or redirect_out_path based on the delimiter.

References used: 
https://cplusplus.com/reference/cstring/strtok/

### pipeline_free

This function simply goes through the pipeline and recursively goes through the pipeline commands to free each piece of dynamically allocated data.

## Shell: myshell.c

This file contains the int main() function which contains the layout of the shell. The flow of the function is to initialize variables, then in a while loop it will print the prompt (if -n is not an argument in the executable call), take in user input, build a pipeline command using pipeline_build, then execute that command. The shell is designed to exit if the user enters a Ctrl-D (exit process), and if so it will wait in a while loop until all children processes are finished to prevent the creation of zombie processes.

### Forking processes

After building the pipeline, the function will fork (create a child process) for the purpose of executing the command put into the pipeline. The parent process will check if the pipeline is a background command and, if not, will wait until the child process has exited.

### Executing commands

In the child process, it initializes a pipeline file descriptor for piped commands. The code goes into a while loop iterates through each command in the current pipeline by forking then setting pipes and executing in the child process and moving onto the next command in the parent process. If the current command is not the first one, the loop uses dup2 to set its read in to the pipe read (so it takes in input from the previous command output), and if the command is not the last command in the pipe the loop uses dup2 to set its write out to the pipe read (so it will output to the input of the next command in the pipeline). If there is a redirect_in path in the first command in the pipeline, the read file descriptor will point to that filename, and if there is a redirect_out path in the last command it will point its write out to that filename.

After the while loop, the parent process waits for the child to be done (meaning each of the command have successfully executed), and closes the pipe file descriptors.

References used:
https://www.geeksforgeeks.org/fork-system-call/
https://www.digitalocean.com/community/tutorials/execvp-function-c-plus-plus
https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-waitpid-wait-specific-child-process-end
https://www.geeksforgeeks.org/dup-dup2-linux-system-call/
https://codechacha.com/en/fork-waitpid-and-timeout/

(Sorry for not using the textbook for these we didn't go over them in class yet)
