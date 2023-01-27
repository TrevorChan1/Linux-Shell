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
	command->command_args[0] = args;
	command->redirect_in_path = NULL;
	command->redirect_out_path = NULL;
	pipe->commands = command;
	pipe->is_background = false;
	
	//Copy command_line to a character pointer (convert const char * to char * for strtok)
	char com[MAX_LINE_LENGTH];
	strcpy(com, command_line);
	char * currentCommand;
	char * remainingCommand;


	//Lexing: Iterate through command line characters and separate into tokens
	char * whitespace = " \n\t\v\f\r";
	char * delimiters = "|&<>";

	//Loop through whole command until the command is empty
	currentCommand = strtok_r(com, delimiters, &remainingCommand);
	int currentLen = 0;
	
	while(currentCommand != NULL){
		
		// printf("%s\n",currentCommand);
		
		//printf("%d\n",strlen(currentCommand));
		// printf("%s\n", currentCommand);
		//Separate the currentCommand into word tokens (left side should always be a command)
		char* token;
		char* rest;
		if(currentLen > 0)
			currentLen += 1;
		// printf("here\n");
		currentLen += strlen(currentCommand);
		printf("Len: %d\n", strlen(currentCommand));
		char delim = command_line[currentLen];
		
		printf("%d: ", currentLen);
		printf("%c\n", delim);

		// printf("%s\n", remainingCommand);
		
		// if(strlen(currentCommand)>0){
			token = strtok_r(currentCommand, whitespace, &rest);
			int num = 0;
			while(token != NULL){
				//Dynamically allocate the memory
				char *argument = (char*) malloc(sizeof(char*));
				strcpy(argument,token);
				command->command_args[num++] = argument;
				token = strtok_r(rest, whitespace, &rest);
			}
		// }
		// printf("%c\n",delim);
		if(remainingCommand != NULL){
			switch(delim){
				//Case |: Dynamically allocate a new pipeline command, set current pipeline command to point
				//to this next one, and set current command being looked at to the rest of the command statement
				case '|':
				{
					// if(command->command_args[0] != NULL){
						struct pipeline_command * newCommand = (struct pipeline_command*) malloc(sizeof(struct pipeline_command));
						command->next = newCommand;
						command = newCommand;
						currentCommand = remainingCommand;
						//Handle case of i
						if(remainingCommand[0] == '&'){
							pipe->is_background= true;
							currentCommand = NULL;
							break;
						}
						currentCommand = strtok_r(currentCommand, delimiters, &remainingCommand);
					// }
					// else{
					// 	printf("Syntax error near unexpected token '|'\n)");
					// 	return NULL;
					// }
					break;
				}
				//Case &: If there's more than just the & then create a new pipeline with background set to true
				// If there isn't more, then set current pipeline to background true
				case '&':
				{
					//Assume & is at the end and just set is_background for current command to true!
					// remainingCommand++;
					// struct pipeline * back = pipeline_build(remainingCommand);
					// back->is_background = true;
					pipe->is_background = true;
					currentCommand = NULL;
					break;
				}
				//Case > or <: Set the remainder of the command to be the redirect_out_path
				case '<':
				{
					char * start = remainingCommand;

					char * p = strtok_r(remainingCommand, whitespace, &remainingCommand);
					char *path = (char*) malloc(sizeof(char*));
					strcpy(path,p);

					command->redirect_in_path = path;

					//Handle cases where it is immediately followed by an &
					if(remainingCommand[0] == '&'){
						pipe->is_background= true;
						currentCommand = NULL;
						break;
					}
					currentCommand = strtok_r(remainingCommand, delimiters, &remainingCommand);
					if(remainingCommand == NULL){
						currentCommand = NULL;
					}
					else{
						currentLen += remainingCommand- start - 2;
						//Make currentCommand empty to move onto the next delimiter in the while loop
						currentCommand=" ";
					}
					break;

				}
				case '>':{
					
					char * start = remainingCommand;

					char * p = strtok_r(remainingCommand, whitespace, &remainingCommand);
					char *path = (char*) malloc(sizeof(char*));
					strcpy(path,p);

					command->redirect_out_path = path;

					//Handle cases where it is immediately followed by an &
					if(remainingCommand[0] == '&'){
						pipe->is_background= true;
						currentCommand = NULL;
						break;
					}
					currentCommand = strtok_r(remainingCommand, delimiters, &remainingCommand);
					if(remainingCommand == NULL){
						currentCommand = NULL;
					}
					else{
						currentLen += remainingCommand- start - 2;
						//Make currentCommand empty to move onto the next delimiter in the while loop
						currentCommand=" ";
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
	// TODO: Implement this function
	//return NULL;
	return pipe;
}

void command_free(struct pipeline_command *command){
	//Recursively free all commands in the pipeline (first frees end and goes inward)
	if(command->next != NULL){
		command_free(command->next);
		free(command->next);
	}
	if(command->redirect_in_path) free(command->redirect_in_path);
	if(command->redirect_out_path) free(command->redirect_out_path);
	int index = 0;
	while(command->command_args[index] != NULL){
		free(command->command_args[index++]);
	}
}

//Function to free the dynamically allocated memory of an inputted pipeline structure
void pipeline_free(struct pipeline *pipeline)
{
	//Free pipeline command struct stored in the pipeline (if any)
	if(pipeline->commands != NULL) command_free(pipeline->commands);
	//Free pipeline struct
	free(pipeline);
}
