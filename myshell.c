#include "myshell_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>


#define TEST_ASSERT(x) do { \
	if (!(x)) { \
		fprintf(stderr, "%s:%d: Assertion (%s) failed!\n", \
				__FILE__, __LINE__, #x); \
	       	abort(); \
	} \
} while(0)


//1. Main 
void pipeline_print(struct pipeline* pipe);

int main(int argc, char **argv)
{
	//Initialization
	bool print_shell = true;
	char command[MAX_LINE_LENGTH];

	//See if -n is an argument
	if(argc > 1){
		for(int i = 1; i < argc; i++){
			if(strcmp(argv[i],"-n") == 0)
				print_shell = false;
		}
	}

	//Main loop
	while(1){
		//Only print my_shell$ if -n is not an argument
		if(print_shell)
			printf("my_shell$");
		
		//Grab the command from the user, if fgets returns NULL that means Ctrl-D => Stop the command line
		if(fgets(command, MAX_LINE_LENGTH, stdin) == NULL){

			printf("\n");

			//If want to exit, wait for ALL child processes to exit until leave code
			while(waitpid(-1, NULL, 0) > 0){
			}
			break;
		}
		//Build a pipeline struct based on the user's command input
		struct pipeline* my_pipeline = pipeline_build(command);

		//If a valid pipeline was created, then fork and execute the command
		if(my_pipeline){
			pid_t m = fork();
			struct pipeline_command * currentCommand = my_pipeline->commands;
			//Parent process
			if(m){
				//If the current child process is NOT a background process, wait for it to finish before continuing
				if(!my_pipeline->is_background){
					waitpid(m, 0,0);
					//Free the pipeline memory
					pipeline_free(my_pipeline);
				}
			}
			//Child process
			else{

				//Initialize pipeline of commands
				int prevPipe[2] = {-1, -1};
				int currPipe[2] = {-1, -1};

				//Initialize pid_t value to keep track of last child, and currentPipe
				pid_t n;

				//Loop through each command in the pipeline
				while(currentCommand != NULL){

					//Create the new pipeline
					if(currentCommand->next != NULL){
						if(pipe(currPipe) == -1){
							printf("ERROR: Failed to create pipe\n");
							exit(-2);
						}
					}

					//Create new child process to run each command in pipeline
					//Parent process: Create new fork for next command in pipeline then move on to next iteration of loop
					n = fork();

					//Child Process: Set read and write as needed then execute command
					if(n == 0){

						//File Read: If current command is not the first in the pipeline, set read from stdin to pipe read
						if(currentCommand != my_pipeline->commands){
							if(dup2(prevPipe[0], STDIN_FILENO) < 0){ //Replace stdin with pipe read
								printf("ERROR: Failed to set up read pipe\n");
								exit(-3);
							}
							//Close previous command pipe
							close(prevPipe[0]);
							close(prevPipe[1]);
						}
						//If first command in pipeline has a redirect_in_path, set pipeline input to that
						else{
							if(currentCommand->redirect_in_path != NULL){
								//Open the redirect_in_path file as the input for the first command (ignoring closing the file since will close when process ends)
								int fileIn = open(currentCommand->redirect_in_path, O_RDONLY);
								if(fileIn == -1){
									printf("ERROR: %s: No such file or directory\n", currentCommand->redirect_in_path);
									exit(-1);
								}
								//Replace stdin with reading that file
								dup2(fileIn, STDIN_FILENO);
								close(fileIn);
							}
						}

						//File Write: If current command is not the last in the pipeline, set read from stdout to pipe write
						if(currentCommand->next != NULL){
							if(dup2(currPipe[1], STDOUT_FILENO) < 0){	//Replace stdout with pipe write
								printf("ERROR: Failed to set up read pipe\n");
								exit(-3);
							}
							//Close current command pipe
							close(currPipe[0]);
							close(currPipe[1]);
						}
						//If last command in pipeline has a redirect_out_path, set pipeline output to that
						else{
							if(currentCommand->redirect_out_path != NULL){
								
								//Open the redirect_out_path file as the output for the last command
								int fileOut = open(currentCommand->redirect_out_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
								if(fileOut == -1){
									printf("ERROR: %s: No such file or directory\n", currentCommand->redirect_out_path);
									exit(-1);
								}
								//Replace stdin with reading that file
								dup2(fileOut, STDOUT_FILENO);
								close(fileOut);
							}
						}

						//Execute command, print error message if not a correct command
						if(execvp(currentCommand->command_args[0], currentCommand->command_args)){
							printf("ERROR: %s: No such file or directory\n", currentCommand->command_args[0]);
							exit(-1);
						}
						exit(1);
					}

					//Parent process: Wait for child process to end then set currentCommand to next in pipeline
					currentCommand = currentCommand->next;
					if(prevPipe[0] != -1){
						close(prevPipe[0]);
						close(prevPipe[1]);
					}
					prevPipe[0] = currPipe[0];
					prevPipe[1] = currPipe[1];

				}
				//After command pipeline is done executing, close last [o[e;ome]]
				close(prevPipe[0]);
				close(prevPipe[1]);
				waitpid(n,0,0);

				//Exit with return code 0
				return 0;
			}
		}
	}
}



void pipeline_print(struct pipeline* pipe){
	if(pipe != NULL){
		printf("Pipeline exists\n");

		if(pipe->is_background) printf("Pipeline is a background process\n");
		else printf("Pipeline is not a background process\n");

		if(pipe->commands != NULL){
			printf("Pipe_commands exists\n");

			struct pipeline_command * currentCommand = pipe->commands;

			while(currentCommand != NULL){
				int index = 0;
				while(currentCommand->command_args[index] != NULL){
					printf("\t%s\n", currentCommand->command_args[index++]);
				}
				if(index == 0)
					printf("\tNo commands\n");

				if(currentCommand->redirect_in_path != NULL) printf("\tRedirect in path: %s\n", currentCommand->redirect_in_path);
				else printf("\tNo redirect in path\n");

				if(currentCommand->redirect_out_path != NULL) printf("\tRedirect out path: %s\n", currentCommand->redirect_out_path);
				else printf("\tNo redirect out path\n");

				currentCommand = currentCommand->next;
				if(currentCommand != NULL){
					printf("Next command:\n");
				}
				else{
					printf("No next command\n");
				}
			}

			
		}
		else
			printf("Pipeline commands do not exist");

	}
	else
		printf("Pipeline does not exist\n");
}