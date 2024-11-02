
#ifndef command_h
#define command_h


#include <fcntl.h>

// Command Data Structure
struct SimpleCommand {
	// Available space for arguments currently preallocated
	int _numberOfAvailableArguments;

	// Number of arguments
	int _numberOfArguments;
	char ** _arguments;
	int _append;
	
	SimpleCommand();
	void insertArgument( char * argument );
};

struct Command {
	int _numberOfAvailableSimpleCommands;
	int _numberOfSimpleCommands;
	SimpleCommand ** _simpleCommands;
	char * _outFile;
	char * _inputFile;
	char * _errFile;
	int _background;

	void prompt();
	void print();
	void execute();
	void clear();

	void changeDirectory(const char *dir);

	void logTerminatedChild(pid_t pid, int i);

	// void redirect_(int cmd_no, int in, int out);
	void redirect(int i, int myinput, int myoutput);
	void handlePipes(int defaultin, int defaultout);
	
	Command();
	void insertSimpleCommand( SimpleCommand * simpleCommand );

	static Command _currentCommand;
	static SimpleCommand *_currentSimpleCommand;
};

#endif
