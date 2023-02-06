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
			// pipeline_print(my_pipeline);
			pid_t m = fork();
			struct pipeline_command * currentCommand = my_pipeline->commands;
			//Parent process
			if(m){
				//If the current child process is NOT a background process, wait for it to finish before continuing
				if(!my_pipeline->is_background)
					waitpid(m, 0,0);
			}
			//Child process
			else{
				//Initialize pipeline of commands
				int pipeline[2];
				if(pipe(pipeline) == -1){
					printf("ERROR: Failed to create pipe\n");
					return -2;
				}
				//Initialize pid_t value to keep track of last child
				pid_t n;

				//Loop through each command in the pipeline
				while(currentCommand != NULL){
					//Create new child process to run each command in pipeline
					//Parent process: Create new fork for next command in pipeline then move on to next iteration of loop
					n = fork();

					//Child Process: Set read and write as needed then execute command
					if(n == 0){
						
						//If current command is not the first in the pipeline, set read from stdin to pipe read
						if(currentCommand != my_pipeline->commands){
							dup2(pipeline[0], STDIN_FILENO); //Replace stdin with pipe read
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
							}
						}
						
						//If current command is not the last in the pipeline, set read from stdout to pipe write
						if(currentCommand->next != NULL){
							dup2(pipeline[1], STDOUT_FILENO);	//Replace stdout with pipe write
						}
						//If last command in pipeline has a redirect_out_path, set pipeline output to that
						else{
							if(currentCommand->redirect_out_path != NULL){
								
								//Open the redirect_out_path file as the output for the last command
								int fileOut = open(currentCommand->redirect_out_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
								if(fileOut == -1){
									perror("open");
									printf("ERROR: %s: No such file or directory\n", currentCommand->redirect_out_path);
									exit(-1);
								}
								//Replace stdin with reading that file
								dup2(fileOut, STDOUT_FILENO);
							}
						}

						//Close any part of the pipeline that is not being used
						close(pipeline[1]);
						close(pipeline[0]);

						//Execute command, print error message if not a correct command
						if(execvp(currentCommand->command_args[0], currentCommand->command_args)){
							printf("ERROR: %s: No such file or directory\n", currentCommand->command_args[0]);
							return -1;
						}
						
					}

					//Parent process: Wait for child process to end then set currentCommand to next in pipeline
					currentCommand = currentCommand->next;
					
				}
				//After command pipeline is done executing, close pipeline and wait for commands to finish running
				close(pipeline[0]);
				close(pipeline[1]);
				waitpid(n,0,0);

				//Exit with return code 0
				return 0;
			}
			//Free the pipeline memory
			pipeline_free(my_pipeline);
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