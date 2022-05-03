%{
#define YYSTYPE void*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.hpp"
#include "output.h"
using namespace output;

%}


%option yylineno
%option noyywrap

%%

void		return VOID;
int			return INT;
byte		return BYTE;
b			return B;
bool		return BOOL;
and			return AND;
or			return OR;
not			return NOT;
true		return TRUE;
false		return FALSE;
return		return RETURN;
if			return IF;
else		return ELSE;
while		return WHILE;
break		return BREAK;
continue	return CONTINUE;
;			return SC;
,			return COMMA;
\(			return LPAREN;
\)			return RPAREN;
\{			return LBRACE;
\}			return RBRACE;
=			return ASSIGN;
(<|>|>=|<=)	{ yylval= new string(yytext); return RELOP; }
==|!=		{ yylval= new string(yytext); return RELOPII; }
\+			return PLUS;
-			return MINUS;
\*			return MULT;
\/			return DIV;
[a-zA-Z][a-zA-Z0-9]*	{ yylval= new string(yytext); return ID; }
0|[1-9][0-9]*	  { yylval= new string(yytext); return  NUM; }
\"([^\n\r\"\\]|\\[rnt"\\])+\"   { yylval= new string(yytext); return  STRING; }
[\r\n\t ]	;
\/\/[^\r\n]*(\r|\n|\r\n)?		;
.			{errorLex(yylineno); exit(1); }


%%




    
