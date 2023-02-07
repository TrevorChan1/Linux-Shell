#include "myshell_parser.h"
#include "stddef.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Function to convert a character of the command line into a pipeline struct with dynamically allocated memory
struct pipeline *pipeline_build(const char *command_line)
{
	//Lexing: Iterate through command line characters and separate into tokens
	char * whitespace = " \n\t\v\f\r";
	char * delimiters = "|&<>";

	//Copy command_line to a character pointer (convert const char * to char * for strtok)
	char com[MAX_LINE_LENGTH];
	strcpy(com, command_line);
	char * currentCommand;
	char * remainingCommand;

	//If the command is empty, return NULL (don't create a pipeline)
	if(strtok(com,whitespace) == NULL){
		return NULL;
	}
	strcpy(com, command_line);

	//Dynamically allocate memory for the output pipeline struct & initialize values
	struct pipeline * pipe = (struct pipeline *) malloc(sizeof(struct pipeline));
	struct pipeline_command * command = (struct pipeline_command*) malloc(sizeof(struct pipeline_command));
	char * args = (char*) malloc(sizeof(char*));
	command->command_args[0] = args;
	command->redirect_in_path = NULL;
	command->redirect_out_path = NULL;
	pipe->commands = command;
	pipe->is_background = false;

	//Grab first string delimited by a non-word token
	currentCommand = strtok_r(com, delimiters, &remainingCommand);
	int currentLen = 0;
	
	//Loop through segments delimited by non-word token until empty
	while(currentCommand != NULL){

		//Separate the currentCommand into word tokens (left side should always be a command)
		char* token;
		char* rest;
		if(currentLen > 0)
			currentLen += 1;
		currentLen += strlen(currentCommand);
		char delim = command_line[currentLen];
		
		token = strtok_r(currentCommand, whitespace, &rest);
		int num = 0;
		while(token != NULL){
			//Dynamically allocate the memory
			command->command_args[num] = (char*) malloc(sizeof(char*));
			strcpy(command->command_args[num++],token);
			token = strtok_r(rest, whitespace, &rest);
		}
		//Make it so there's always a trailing NULL (If don't do, can lead to errors if using previously freed memory)
		command->command_args[num] = NULL;

		//After getting command arguments, decide what to do next based on the token
		if(remainingCommand != NULL){
			switch(delim){
				//Case |: Dynamically allocate a new pipeline command, set current pipeline command to point
				//to this next one, and set current command being looked at to the rest of the command statement
				case '|':
				{
					//Allocate memory for new pipeline command and set current command to the next one
					struct pipeline_command * newCommand = (struct pipeline_command*) malloc(sizeof(struct pipeline_command));
					command->next = newCommand;
					command = newCommand;
					currentCommand = remainingCommand;

					//Handle case of if & immediately follows the | (assume commands end with &)
					if(remainingCommand[0] == '&'){
						pipe->is_background= true;
						currentCommand = NULL;
						break;
					}

					currentCommand = strtok_r(currentCommand, delimiters, &remainingCommand);
					break;
				}
				//Case &: If there's more than just the & then create a new pipeline with background set to true
				// If there isn't more, then set current pipeline to background true
				case '&':
				{
					//Assume & is at the end and just set is_background for current command to true!
					pipe->is_background = true;
					currentCommand = NULL;
					break;
				}
				//Case > or <: Set the remainder of the command to be the redirect_out_path
				case '<':
				case '>':{
					char * start = remainingCommand;

					//Cut remaining command into the segment until the next non-word token
					currentCommand = strtok_r(remainingCommand, delimiters, &remainingCommand);

					//Get the first word token in the first segment
					char * p = strtok(currentCommand, whitespace);
					//If empty, then return nothing and print an error
					if(p == NULL){
						pipeline_free(pipe);
						printf("ERROR: syntax error near unexpected token '%s'\n",p);
						return NULL;
					}
					char *path = (char*) malloc(sizeof(char*));
					strcpy(path,p);

					//Set redirect path to the dynamically allocated path string (based on delimiter)
					if(delim == '>'){
						
						//On Mac, empty remainingCommand becomes NULL, but on Linux it has whitespace so doing this:
						if(remainingCommand != NULL){
							if(strtok(remainingCommand,whitespace) == NULL){
								remainingCommand = NULL;
							}
						}

						//If command tries to redirect out to anything other than the last command in a pipe, return NULL
						if(strtok(remainingCommand,whitespace) == NULL){
							command->redirect_out_path = path;
						}
						//If command is not last command in pipeline, print error and return NULL
						else{
							printf("ERROR: syntax error near unexpected token '>'\n");
							pipeline_free(pipe);
							return NULL;
						}
					}
					else if(delim == '<'){
						//If command tries to redirect in from anything other than the first command in a pipe, return NULL
						if(command == pipe->commands){
							command->redirect_in_path = path;
						}
						//If command is not first command in pipeline, print error and return NULL
						else{
							printf("ERROR: syntax error near unexpected token '<'\n");
							pipeline_free(pipe);
							return NULL;
						}
					}
					//If there are more tokens ahead, set it so the current command is empty and prepare to execute next part
					if(remainingCommand != NULL){
						currentCommand = " ";	//Prevent parser from exiting and from overwriting arguments
						currentLen += remainingCommand - start - 2;
					}
					else{
						currentCommand = NULL;
					}
					break;
				}
				default:
					currentCommand = NULL;
					break;
			}
		}
		else
			currentCommand = NULL;
	}
	return pipe;
}

//Helper function that recursively goes through all pipeline commands in the pipeline and frees them
void command_free(struct pipeline_command *command){
	//Recursively free all commands in the pipeline (first frees end and goes inward)
	if(command->next != NULL){
		//Recursive call, then free memory and set
		command_free(command->next);
		free(command->next);
		command->next = NULL;
	}
	//Free redirect in path
	if(command->redirect_in_path){
		free(command->redirect_in_path);
		command->redirect_in_path = NULL;
	}
	//Free redirect out path
	if(command->redirect_out_path) {
		free(command->redirect_out_path);
		command->redirect_out_path = NULL;
	}
	//Free the command arguments
	int index = 0;
	while(command->command_args[index] != NULL){
		free(command->command_args[index]);
		command->command_args[index++] = NULL;
	}
}


//Function to free the dynamically allocated memory of an inputted pipeline structure
void pipeline_free(struct pipeline *pipeline)
{
	//Free pipeline command struct stored in the pipeline (if any)
	if(pipeline->commands != NULL){
		command_free(pipeline->commands);
		free(pipeline->commands);
		pipeline->commands = NULL;
	}
	//Free pipeline struct
	free(pipeline);
}
