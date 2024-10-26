%{
#include <stdio.h>
#include <stdlib.h>

void yyerror(const char *s);  // Error handling function
int yylex();  // Lexer function declaration
#define YYDEBUG 1
%}

%token NUMBER  // Define NUMBER token

%%
calculation:
    expr '\n' { 
        printf("Result: %d\n", $1);  // Print the result
    }
    | '\n' { /* Handle empty input */ }
    ;

expr:
    NUMBER '+' NUMBER { $$ = $1 + $3; }
    | NUMBER '-' NUMBER { $$ = $1 - $3; }
    | NUMBER '*' NUMBER { $$ = $1 * $3; }
    | NUMBER '/' NUMBER { 
        if ($3 == 0) {
            yyerror("Error: Division by zero");
            $$ = 0;
        } else {
            $$ = $1 / $3;
        }
    }
    ;

%%
void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}
