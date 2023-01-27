#include "myshell_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_ASSERT(x) do { \
	if (!(x)) { \
		fprintf(stderr, "%s:%d: Assertion (%s) failed!\n", \
				__FILE__, __LINE__, #x); \
	       	abort(); \
	} \
} while(0)

//1. Main 
void pipeline_print(struct pipeline* pipe);

int
main(void)
{
	// struct pipeline* my_pipeline = pipeline_build("| test");
	// struct pipeline* my_pipeline = pipeline_build("ls|wc -l > counts.txt\n woo test &");
	// struct pipeline* my_pipeline = pipeline_build("ls -l | test test 2 theee > woo.txt \n \t");
	// struct pipeline* my_pipeline = pipeline_build("a b< d -ls\n| test");
	// struct pipeline* my_pipeline = pipeline_build("arg1 <out a&\n");
	struct pipeline* my_pipeline = pipeline_build(" a      < -ls    > arg1\n");
	// struct pipeline* my_pipeline = pipeline_build("a  b < arg > d | arg1 arg2&\n");
	// struct pipeline* my_pipeline = pipeline_build("ab cd e< fg&\n");
	// struct pipeline* my_pipeline = pipeline_build("arg1 c>a&\n");
	// struct pipeline* my_pipeline = pipeline_build("arg1 c<ab >b\n");

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