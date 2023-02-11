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
	if(strtok(com,whitespace) == NULL || strlen(com) == 0){
		return NULL;
	}

	//If the command starts with | or &, return NULL (don't create a pipeline and return error)
	if(com[0] == '|' || com[0] == '&'){
		if(com[0] == '|')
			printf("ERROR: syntax error near unexpected token '|'\n");
		else
			printf("ERROR: syntax error near unexpected token '&'\n");
		return NULL;
	}

	strcpy(com, command_line);

	//Dynamically allocate memory for the output pipeline struct & initialize values
	struct pipeline * pipe = (struct pipeline *) malloc(sizeof(struct pipeline));
	struct pipeline_command * command = (struct pipeline_command*) malloc(sizeof(struct pipeline_command));
	command->command_args[0] = NULL;
	command->redirect_in_path = NULL;
	command->redirect_out_path = NULL;
	command->next = NULL;
	pipe->commands = command;
	pipe->is_background = false;

	//Grab first string delimited by a non-word token
	currentCommand = strtok_r(com, delimiters, &remainingCommand);
	int currentLen = 0;
	bool firstCmdEmpty = false;
	bool sameCommand = false;

	//Deal with the case where the first character is a special token (strtok won't catch it)
	if(command_line[0] == '|' || command_line[0] == '&' || command_line[0] == '>' || command_line[0] == '<')
		firstCmdEmpty = true;
	
	//Loop through segments delimited by non-word token until empty
	while(currentCommand != NULL){

		//Separate the currentCommand into word tokens (left side should always be a command)
		char* token;
		char* rest;
		char delim;
		if(currentLen > 0)
			currentLen += 1;

		//Only parse through the characters in the currentCommand if it's not the first command and the first character isn't a special token
		if(!firstCmdEmpty){
			currentLen += strlen(currentCommand);
			//Only parse through command if in a new command (in the cases of multiple special tokens in same command)
			if(!sameCommand){
				token = strtok_r(currentCommand, whitespace, &rest);
				int num = 0;
				while(token != NULL){
					//Dynamically allocate the memory, each command gets allocated MAX_ARG_LENGTH + 1 characters to include \0
					command->command_args[num] = (char*) malloc((MAX_ARGV_LENGTH + 1)*sizeof(char));
					strcpy(command->command_args[num++],token);
					token = strtok_r(rest, whitespace, &rest);
				}
				//Make it so there's always a trailing NULL (If don't do, can lead to errors if using previously freed memory)
				command->command_args[num] = NULL;
				sameCommand = true;
			}
		}
		else{
			firstCmdEmpty = false;
			remainingCommand = currentCommand;
		}
		
		//Set delimiter used to the character
		delim = command_line[currentLen];

		//After getting command arguments, decide what to do next based on the token
		if(remainingCommand != NULL){
			if(currentLen < strlen(command_line)-1){
				if(delim == command_line[currentLen+ 1]){
					printf("ERROR: syntax error near unexpected token '%c%c'\n", delim, command_line[currentLen+1]);
					pipeline_free(pipe);
					return NULL;
				}
			}
			switch(delim){
				//Case |: Dynamically allocate a new pipeline command, set current pipeline command to point
				//to this next one, and set current command being looked at to the rest of the command statement
				case '|':
				{
					if(command->command_args[0] == NULL){
						printf("ERROR: syntax error near unexpected token '|'\n");
						pipeline_free(pipe);
						return NULL;
					}

					//Allocate memory for new pipeline command and set current command to the next one
					struct pipeline_command * newCommand = (struct pipeline_command*) malloc(sizeof(struct pipeline_command));
					
					//Initialize unused values to NULL
					newCommand->command_args[0] = NULL;
					newCommand->next = NULL;
					newCommand ->redirect_in_path = NULL;
					newCommand->redirect_out_path = NULL;

					//Set newly allocated command to next command
					command->next = newCommand;
					command = newCommand;
					currentCommand = remainingCommand;

					//Set it to parse through command arguments
					sameCommand = false;

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
					if(command->command_args[0] == NULL){
						printf("ERROR: syntax error near unexpected token '&'\n");
						pipeline_free(pipe);
						return NULL;
					}
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
						printf("ERROR: syntax error near unexpected token '(null)'\n");
						return NULL;
					}
					//Initialize path to be of length max_argv_length +1 since file path can be length of largest argv
					char *path = (char*) malloc((MAX_ARGV_LENGTH + 1)*sizeof(char));
					strcpy(path,p);

					//Set redirect path to the dynamically allocated path string (based on delimiter)
					if(delim == '>'){
						
						//On Mac, empty remainingComsmand becomes NULL, but on Linux it has whitespace so doing this:
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
