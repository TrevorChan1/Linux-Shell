#include "myshell_parser.h"
#include "stddef.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Function to convert a character of the command line into a pipeline struct with dynamically allocated memory
struct pipeline *pipeline_build(const char *command_line)
{
	//Dynamically allocate memory for the output pipeline struct
	struct pipeline * pipe = (struct pipeline *) malloc(sizeof(struct pipeline));
	struct pipeline_command * command = (struct pipeline_command*) malloc(sizeof(struct pipeline_command));
	char * args = (char*) malloc(sizeof(char*));
	command->redirect_in_path = "test";
	command->command_args[0] = args;
	command->redirect_in_path = NULL;
	command->redirect_out_path = NULL;
	pipe->commands = command;
	
	//Copy command_line to a character pointer (convert const char * to char * for strtok)
	char com[MAX_LINE_LENGTH];
	strcpy(com, command_line);

	//Lexing: Iterate through command line characters and separate into tokens
	char * delimiters = " \n\t\v\f\r";
	char * delimiters1st = "|&";
	char * delimiters2nd = "<>";

	//Get string delimited by word tokens
	char* token;
	char* rest;

	//token = strtok_r(com, delimiters1st, &rest);
	
	//if(rest != NULL){
		//If there is a background or next pipeline command, recursively run the function on that character string
		
	//}



	token = strtok_r(com, delimiters, &rest);
	int num = 0;

	while(token != NULL){
		//Dynamically allocate the memory
		char *argument = (char*) malloc(sizeof(char*));
		strcpy(argument,token);
		command->command_args[num++] = argument;
		printf("%s\n", command->command_args[num-1]);
		token = strtok_r(rest, delimiters, &rest);
	}
	
	

	// TODO: Implement this function
	//return NULL;
	return pipe;
}


//Function to free the dynamically allocated memory of an inputted pipeline structure
void pipeline_free(struct pipeline *pipeline)
{
	//Free memory of inputted pipeline
	//free(pipeline);
}
