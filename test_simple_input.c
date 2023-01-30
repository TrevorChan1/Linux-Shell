#include "myshell_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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
						
						//If current command is not the last in the pipeline, set read from stdout to pipe write
						if(currentCommand->next != NULL){
							dup2(pipeline[1], STDOUT_FILENO);	//Replace stdout with pipe write
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
			pipeline_free(my_pipeline);
		}
	}
}


/*
int main(){
	// struct pipeline* my_pipeline = pipeline_build("| test");
	// struct pipeline* my_pipeline = pipeline_build("| &test");
	// struct pipeline* my_pipeline = pipeline_build("ls|wc -l > counts.txt\n woo test &");
	// struct pipeline* my_pipeline = pipeline_build("ls -l | test test 2 theee > woo.txt \n \t");
	// struct pipeline* my_pipeline = pipeline_build("a b< d -ls\n| test");
	// struct pipeline* my_pipeline = pipeline_build("arg1 <out a&\n");
	// struct pipeline* my_pipeline = pipeline_build(" a      < -ls    > arg1\n");
	// struct pipeline* my_pipeline = pipeline_build("a  b < arg > d | arg1 arg2&\n");
	// struct pipeline* my_pipeline = pipeline_build("ab cd e< fg&\n");
	// struct pipeline* my_pipeline = pipeline_build("arg1 c>a&\n");
	// struct pipeline* my_pipeline = pipeline_build("arg1 c<ab >b\n");
	// struct pipeline* my_pipeline = pipeline_build("arg<&");
	// struct pipeline* my_pipeline = pipeline_build("ls");
	struct pipeline * my_pipeline = pipeline_build("ls | ls");
	pipeline_print(my_pipeline);
	// // Test that a pipeline was returned
	// TEST_ASSERT(my_pipeline != NULL);
	// TEST_ASSERT(!my_pipeline->is_background);
	// TEST_ASSERT(my_pipeline->commands != NULL);
	
	// // Test the parsed args
	// TEST_ASSERT(strcmp("ls", my_pipeline->commands->command_args[0]) == 0);
	// TEST_ASSERT(my_pipeline->commands->command_args[1] == NULL);

	// // Test the redirect state
	// TEST_ASSERT(my_pipeline->commands->redirect_in_path == NULL);
	// TEST_ASSERT(my_pipeline->commands->redirect_out_path == NULL);

	// // Test that there is only one parsed command in the pipeline
	// TEST_ASSERT(my_pipeline->commands->next == NULL);
	pipeline_free(my_pipeline);
}
*/
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