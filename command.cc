
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
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "command.h"
#include <ostream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>

#include <errno.h>
#include <limits.h>  // For PATH_MAX

#include <iomanip>

using namespace std;





SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments, _numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}



Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
													_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}
	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		_simpleCommands[i]->_append = false;
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	if (_errFile)
	{
		free(_errFile);
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
		   _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
		   _background ? "YES" : "NO");
	printf("\n\n");
}

void Command::changeDirectory(const char *dir) {
    if (dir == NULL || strcmp(dir, "") == 0) {
        // dir = getenv("HOME");
		dir = "/";
        if (dir == NULL) {
            dir = "/";
            fprintf(stderr, "Warning: HOME environment variable not set, changing to root directory\n");
        }
    }
    if (chdir(dir) != 0) {
        perror("cd failed");
    }

    prompt();
}


void Command::logTerminatedChild(pid_t pid, int i)
{
     auto now = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&end_time), "%d-%m-%Y %H:%M");

    std::ofstream logfile("logfile.log", std::ios_base::app);
    if (!logfile) {
        std::cerr << "Error opening logfile.log" << std::endl;
        return;
    }

    logfile << oss.str() << " Child process " << pid << " ";
    
    for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++) {
        logfile << " " << _simpleCommands[i]->_arguments[j];
    }
    
    logfile << " terminated" << std::endl;
	logfile.close();
}

void Command::redirect(int i, int myinput, int myoutput)
{
    if (_inputFile && _outFile)
    {
        int infd = open(_inputFile, O_RDONLY, 0666);
        int outfd = _simpleCommands[i]->_append ? open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666)
                                                 : open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);

        if (infd < 0 || outfd < 0)
        {
            perror("Error: could not open file");
            exit(2);
        }

        dup2(infd, 0); 
        close(infd);

        dup2(outfd, 1); 
        close(outfd);
        
        return;
    }

    if (_inputFile)
    {
        int infd = open(_inputFile, O_RDONLY, 0666);
        if (infd < 0)
        {
            perror("Error: could not open input file");
            exit(2);
        }
        dup2(infd, 0); 
        close(infd);
    }
    else if (myinput != 0)
    {
        dup2(myinput, 0); 
        close(myinput);
    }
    if (_outFile)
    {
        int outfd = _simpleCommands[i]->_append ? open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666)
                                                 : open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);

        if (outfd < 0)
        {
            perror("Error: could not open output file");
            exit(2);
        }
        dup2(outfd, 1);
        close(outfd);
    }
    else if (myoutput != 1)
    {
        dup2(myoutput, 1);
        close(myoutput);
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
			{
				dup2(defaultin, 0);
				dup2(currentPipe[1], 1);
			}

			else if (i < _numberOfSimpleCommands - 1)
			{ 
				dup2(previousPipe[0], 0);
				dup2(currentPipe[1], 1);
			}
			else
			{ 
				this->redirect(i, previousPipe[0], defaultout);
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
			perror("Command Failed-->no command found\n");
			exit(2);
		}
		else
		{
			if (i > 0)
			{
				close(previousPipe[0]);
				close(previousPipe[1]);
			}
		
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
			
			waitpid(lastChild, NULL, 0);
			logTerminatedChild(pid, i);

		}
	}
}

void Command::execute()
{
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}
	int defaultin = dup(0);
	int defaultout = dup(1);
	
	if (_numberOfSimpleCommands == 1)
	{
		pid_t pid;
		pid = fork();
		if (pid < 0)
		{
			fprintf(stderr, "Fork Failed");
			return;
		}

		else if (pid == 0)
		{
			this->redirect(0, defaultin, defaultout);
			close(defaultin);
			close(defaultout);
			char path[20] = "/bin/";
			strcat(path, _simpleCommands[0]->_arguments[0]);
			execvp(path, _simpleCommands[0]->_arguments);
			perror("Command Failed-->no command found\n");
			exit(2);
		}

		else
		{
			dup2(defaultin, 0);
			dup2(defaultout, 1);
			close(defaultin);
			close(defaultout);
			if(!_background)
			{
			waitpid(pid, NULL, 0);
			}
			logTerminatedChild(pid,0);
		}
	}
	else
	{
		this->handlePipes(defaultin, defaultout);
	}
	// Clear to prepare for next command
	clear();
	// Print new prompt
	prompt();
	//}
}

// Shell implementation

// void Command::prompt()
// {
// 	printf("myshell>");
// 	fflush(stdout);
// }

void Command::prompt()
{
    char cwd[PATH_MAX]; 

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s>", cwd);
    } else {
        perror("getcwd");  // Handle error if getcwd fails
    }

    fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

int main()
{
	signal(SIGINT, SIG_IGN);
	Command::_currentCommand.prompt();
	yyparse();

	return 0;
}
