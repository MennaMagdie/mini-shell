
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#include <fcntl.h> //for open fn

// #include <csignal>  // For signal handling
// #include <iostream> // For printing to console


#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Create available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}


void Command::redirect_(int cmd_no, int in, int out){

	if(_inputFile){

		int infd=open(_inputFile,O_RDONLY,0666);

		if(infd<0){
			perror("walahy ma3raf | input");
			exit(2);
		}

		dup2(infd, 0);
		close(infd);
		dup2(out, 1);
		close(out);
		return;

	} 
	else if(_outFile){ //2 cases df

		int outfd;

		if(_simpleCommands[cmd_no]->_append)
			outfd = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
		else
			outfd = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);

		if(outfd < 0){
			perror("walahy ma3raf | output");
			exit(2);
		}

		dup2(in, 0);
		close(in);
		dup2(outfd, 1);
		close(outfd);
		return;

	}
	else{
		close(in);
		close(out);
		return;
	}
}


void Command::handlePipes(int defaultin, int defaultout)
{
    int previousPipe[2];
    pid_t lastChild;
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        int currentPipe[2];
        if (pipe(currentPipe) == -1)
        {
            perror("pipe creation error");
            exit(2);
        }
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork error");
            exit(2);
        }
        else if (pid == 0)
        {
            if (i == 0)
            { // first Pipe ->default input , put output in current pipe
                dup2(defaultin, 0);
                dup2(currentPipe[1], 1);
            }
            else if (i < _numberOfSimpleCommands - 1)
            { // mid pipes-> input from previous Pipe, output in current Pipe
                dup2(previousPipe[0], 0);
                dup2(currentPipe[1], 1);
            }
            else
            { // Last pipe -> input from previous Pipe, output to terminal or file
                this->redirect_(i, previousPipe[0], defaultout);
                lastChild = pid;
            }
            close(previousPipe[0]);
            close(previousPipe[1]);
            close(currentPipe[0]);
            close(currentPipe[1]);
            close(defaultin);
            close(defaultout);
            char path[20] = "/bin/";
            strcat(path, _simpleCommands[i]->_arguments[0]);
            execvp(path, _simpleCommands[i]->_arguments);
            perror("Command Failed-->no command with this name\n");
            exit(2);
        }
        else
        {
            if (i > 0)
            {
                close(previousPipe[0]);
                close(previousPipe[1]);
            }
            // Save Last operation output/input
            previousPipe[0] = currentPipe[0];
            previousPipe[1] = currentPipe[1];
            if (i == _numberOfSimpleCommands - 1)
            {
                close(currentPipe[0]);
                close(currentPipe[1]);
                dup2(defaultin, 0);
                dup2(defaultout, 1);
                close(defaultin);
                close(defaultout);
            }
            // Wait only for the last child process
            waitpid(lastChild, NULL, 0);
            // childTerminated(pid, i);
        }
    }
}


void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::execute()
{

	int defaultin = dup(0);
	int defaultout = dup(1);
	int defaulterr = dup(2);

	/*We need to modify execute() so that:
	It forks a new process for each command.
	Uses execvp() to run the command.
	Handles redirection (like > or <).
	Supports pipes and background commands (&*/


	// printf("I am trying execution :)\n");
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}
	else if(_numberOfSimpleCommands == 1 ){

		// prompt();

		pid_t pid = fork();

		if (pid == -1) {
			perror("Fork failed");
			exit(1);

		}else if (pid == 0) { //CHILD

			this->redirect_(0, defaultin, defaultout);
			close(defaultin);
			close(defaultout);

			char cmd_path[20] = "/bin/";
			strcat(cmd_path, _simpleCommands[0]->_arguments[0]);
			execvp(cmd_path	, _simpleCommands[0]->_arguments);
			
			perror("Execution failed");
			exit(1);

		}else { //PARENT

				dup2(defaultin, 0);
				dup2(defaultout, 1);
				dup2(defaulterr, 2);
				close(defaultin);
				close(defaultout);
				close(defaulterr);

				if (_background) {
				printf("Running in background with PID: %d\n", pid);
				} else {
				waitpid(pid, NULL, 0);
			}

		}


	}
	else {
		this->handlePipes(defaultin, defaultout);
	}




	// Print contents of Command data structure
	print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec

	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

void handle_sigint(int sig){
	printf("\nCaught Ctrl+C (SIGINT), but shell will not exit.\n");
}

int main()
{
	signal(SIGINT, handle_sigint);
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

