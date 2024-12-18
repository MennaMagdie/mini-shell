/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [ | cmd [arg]* ]* [ [> filename] [< filename] [>> filename] ]* [&]
 *
 */

%token	<string_val> WORD
%token  NOTOKEN GREAT NEWLINE LESS GREAT_GREAT AMPERSAND EXIT
%token  PIPE
%token CD

%union	{
	char   *string_val;
}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>

#include "command.h"
%}

%%

// Define the main parsing rules

goal:	
    commands
    ;

commands:
    command
    | commands command
    ;

command: 
    simple_command 
    | command PIPE simple_command {
        //printf("Yacc: Found a pipe\n");
        Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
    }
;


simple_command:	
    command_and_args iomodifier_opt NEWLINE {
        //printf("Yacc: Execute command ZZZZ\n");
        Command::_currentCommand.execute();
    }
    | CD WORD NEWLINE { 
        //printf("Changing directory to %s\n", $2);
        Command::_currentCommand.changeDirectory($2);
    }
    | CD NEWLINE { 
        //printf("Changing to home directory\n");
        Command::_currentCommand.changeDirectory(NULL); 
    }
    | NEWLINE 
    | error NEWLINE { yyerrok; }
    | EXIT NEWLINE {
        printf("Exiting shell..\n");
        exit(0);
    }
    ;

command_and_args:
    command_word arg_list {
        Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
    }
	| command_and_args PIPE command_word arg_list {
		//printf("Yacc: You inserted PIPE Operator \n");
		Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
	}
    ;

arg_list:
    arg_list argument
  | /* empty */ { //printf("Yacc: Empty arg_list\n"); 
    }
  ;


argument:
    WORD {
        //printf("Yacc: insert argument \"%s\"\n", $1);
        Command::_currentSimpleCommand->insertArgument($1);
    }
    ;

command_word:
    WORD {
        //printf("Yacc: insert command \"%s\"\n", $1);
        Command::_currentSimpleCommand = new SimpleCommand();
        Command::_currentSimpleCommand->insertArgument($1);
    }
    ;

iomodifier_opt:
    GREAT WORD {
        //printf("Yacc: insert output \"%s\"\n", $2);
        Command::_currentCommand._outFile = $2;
        Command::_currentSimpleCommand->_append = 0;
    }
    | LESS WORD {
        //printf("Yacc: insert input \"%s\"\n", $2);
        Command::_currentCommand._inputFile = $2;
    }
    | GREAT_GREAT WORD {
        //printf("Yacc: append output \"%s\"\n", $2);
        Command::_currentCommand._outFile = $2;
        Command::_currentSimpleCommand->_append = 1;
    }
    | AMPERSAND {
        //printf("Yacc: run in background\n");
        Command::_currentCommand._background = 1;
    }
	| GREAT WORD iomodifier_opt   { 
		//printf("Yacc: Found additional output redirection > %s\n", $2); 
		}
    | LESS WORD iomodifier_opt    {
		//printf("Yacc: Found additional input redirection < %s\n", $2); 
		}
    | /* can be empty */ 
    ;

%%

// Error handling function
void
yyerror(const char * s)
{
	fprintf(stderr, "%s\n", s);
}

