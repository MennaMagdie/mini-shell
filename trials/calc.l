%{
#include "calc.tab.h"
%}

%%
[0-9]+    { 
    printf("Token: NUMBER = %s\n", yytext); 
    yylval = atoi(yytext); 
    return NUMBER; 
}
[ \t]+    ;  // Ignore spaces and tabs
\n        { return '\n'; }  // Handle newlines
"+"       { printf("Token: PLUS\n"); return '+'; }
"-"       { printf("Token: MINUS\n"); return '-'; }
"*"       { printf("Token: MULT\n"); return '*'; }
"/"       { printf("Token: DIV\n"); return '/'; }
.         { return yytext[0]; }
%%
