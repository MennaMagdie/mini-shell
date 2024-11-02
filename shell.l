
/*
 *
 * CS-413 Spring 98
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include "y.tab.h"

%}

%%

\n 	{
		return NEWLINE;
	}


">" 	{
		return GREAT;
	}

">>"	{
		return GREAT_GREAT;
}

"<"		{
		return LESS;
}

"&"		{
		return AMPERSAND;
}

"|"    { 
		printf("Lex: Found a PIPE token\n"); return PIPE; }

"exit" {
		return EXIT;
}


[^ \t\n][^ \t\n]*  {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}	

'.'	{
		/* Invalid character in input */
		return NOTOKEN;
	}

%%
	
