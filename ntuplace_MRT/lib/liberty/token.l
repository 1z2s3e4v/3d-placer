
%{
/******************************************************************************
    Copyright (c) 1996-2000 Synopsys, Inc.    ALL RIGHTS RESERVED

  The contents of this file are subject to the restrictions and limitations
  set forth in the SYNOPSYS Open Source License Version 1.0  (the "License"); 
  you may not use this file except in compliance with such restrictions 
  and limitations. You may obtain instructions on how to receive a copy of 
  the License at

  http://www.synopsys.com/partners/tapin/tapinprogram.html. 

  Software distributed by Original Contributor under the License is 
  distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either 
  expressed or implied. See the License for the specific language governing 
  rights and limitations under the License.

******************************************************************************/
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "liberty_structs.h"
#include "si2dr_liberty.h"
#include "liberty_parser.h"
#include "mymalloc.h"

/* I added this code to avoid a confusion between bus notation and a complex attr with a single integer value. */

typedef struct token_q
{
  int retval;  /* the token value to return */
  int set_num; /* set the yylval to contain the num */
  double num;  /* number to put to yylval */
  char *str;   /* if non-zero, put to yylval */
} token_q;

int token_q_front = 9;
int token_q_rear = 9;

token_q token_q_buf[10];

int token_q_empty(void)
{
	if (token_q_front == token_q_rear)
		return 1;
	else
		return 0;
}

static void remove_token(int *retval, int *set_num, double *num, char **str)
{
	if( token_q_empty() )
		return;
		
	if( token_q_front == 9 )
		token_q_front = 0;
	else
		token_q_front++;
	*retval = token_q_buf[token_q_front].retval;
	*set_num = token_q_buf[token_q_front].set_num;
	*num = token_q_buf[token_q_front].num;
	if( token_q_buf[token_q_front].str )
	{
		*str = token_q_buf[token_q_front].str;
	}
}

static void add_token(int retval, int set_num, double num, char *str)
{
	if( token_q_rear == 9 )
		token_q_rear = 0;
	else
		token_q_rear++;

	if( token_q_rear == token_q_front )
	{
		printf("Token Queue Stack Overflow-- notify program maintainers!\n");
		return;
	}
	token_q_buf[token_q_rear].str = str;
	token_q_buf[token_q_rear].retval = retval;
	token_q_buf[token_q_rear].num = num;
	token_q_buf[token_q_rear].set_num = set_num;
}

/* this routine is called from liberty_front_lex.c; which is acting
    as a front end to this lexer */

int injected_token(void)
{
	char *str; 
	double num; 
	int retval; 
	int set_num;

	remove_token(&retval,&set_num,&num,&str);

	if( set_num)
    { 
		liberty_parser_lval.num = num;
	}
	else if(str)
	{
		liberty_parser_lval.str = str;
	}
	return retval;
}

char *my_strdup(char *str){ char *x = my_malloc(strlen(str)+3 /* just in case of insertions */); strcpy(x,str); return x;}

static int lline;
extern int lineno;
int save_lineno, save_lline;
static char filenamebuf[1000];
static char string_buf[10240];
static char *string_buf_ptr;
struct nl
{
	char *fname;
	struct nl *next;
};
struct nl  *file_name_list;

char *curr_file, *curr_file_save; 
#define MAX_INCLUDE_DEPTH 1
void clean_file_name(char *,char *);
int include_stack_index = 0;
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];

char *search_string_for_linefeeds(char *str);
%}
%option   noyywrap
%option prefix="liberty_parser2_"
%option outfile="lex.yy.c"
%x comment
%x stringx
%x include

VOLTAGE VDD|VCC|VSS
EXPR_OP "+"|"-"|"*"|"/"
FLOAT [-+]?([0-9]+\.?[0-9]*([Ee][-+]?[0-9]+)?|[0-9]*\.[0-9]*([Ee][-+]?[0-9]+)?)
SP  [ \t]*
%%

\,	{lline = lineno;return COMMA;}
\;	{lline = lineno;return SEMI;}
\(	{lline = lineno;return LPAR;}
\)	{lline = lineno;return RPAR;}
\{	{if(lline != lineno) {printf("Warning: line %d: Opening Curly must be on same line as group declaration!\n", lineno);} return LCURLY;}
\}[ \t]*\;?	{lline = lineno;return RCURLY;}
[ \t]?\:	{lline = lineno;if( yyleng == 1 )printf("Error: space must precede Colon (:) at line %d\n", lineno); return COLON;}
include_file[ \t]*\(	BEGIN(include);


[-+]?([0-9]+\.?[0-9]*([Ee][-+]?[0-9]+)?|[0-9]*\.[0-9]*([Ee][-+]?[0-9]+)?)	{lline = lineno;liberty_parser_lval.num = strtod(yytext,(char**)NULL); return NUM;}


{VOLTAGE}{SP}{EXPR_OP}{SP}{FLOAT}|{FLOAT}{SP}{EXPR_OP}{SP}{VOLTAGE} {lline = lineno;liberty_parser_lval.str = my_strdup(yytext); 
													     return STRING;}

[A-Za-z!@#$%^&_+=\|~\?:][A-Za-z0-9!@#$%^&_+=\|~\?:]*[\<\{\[\(][-0-9:]+[\]\}\>\)]		{
 lline = lineno;
 if( !strncmp(yytext,"values(",7) )
 {
   /* ugh -- a values() with a single unquoted number in it! let's translate it into a values with a single quoted value instead! */
   char *ident = (char*)malloc(7);
   char *str = (char*)malloc(strlen(yytext)-4);
   strcpy(ident,"values");
   liberty_parser_lval.str = ident;
   
   strcpy(str,yytext+7);
   str[strlen(str)-1] = 0;
   add_token(LPAR, 0, 0.0, 0);
   add_token(STRING, 0, 0.0, str);
   add_token(RPAR, 0, 0.0, 0);
   return IDENT;
 }

 else
 {
   char *str = (char*)malloc(strlen(yytext)+1);
  strcpy(str,yytext);  
	liberty_parser_lval.str = str;
  return STRING;
 }
}

[-+]?[0-9]*\.?[0-9]+([Ee][-+]?[0-9]+)?[ 	]*[-\+\*\/][ 	]*[-+]?[0-9]*\.?[0-9]+([Ee][-+]?[0-9]+)? {lline = lineno;liberty_parser_lval.str = my_strdup(yytext); 
                                                                                     return STRING;}
"define" {lline = lineno;return KW_DEFINE;}
"define_group" {lline = lineno;return KW_DEFINE_GROUP;}
[Tt][Rr][Uu][Ee]   {lline = lineno;return KW_TRUE;}
[Ff][Aa][Ll][Ss][Ee]  {lline = lineno;return KW_FALSE;}
\\?\n	{lineno++;}
\\[ \t]+\n	{printf("ERROR-- %s:%d -- Continuation char followed by spaces or tabs!\n\n", curr_file, lineno); lineno++; }
\r  {}
\t  {}
" "	{}


"/*"	BEGIN(comment);
\"		string_buf_ptr = string_buf; BEGIN(stringx);

<comment>[^*\n]*        /* eat anything that's not a '*' */
<comment>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<comment>\n             ++lineno;
<comment>"*"+"/"	BEGIN(INITIAL);

<stringx>\"	{ char *x; BEGIN(INITIAL); *string_buf_ptr = 0;
			liberty_parser_lval.str = my_strdup(string_buf); return STRING; }

<stringx>\n { char *x; printf("Error, line %d: unterminated string constant-- use a back-slash to continue a string to the next line.\n", lineno++); 
			BEGIN(INITIAL); *string_buf_ptr = 0;
			liberty_parser_lval.str = my_strdup(string_buf); return STRING;
		}
<stringx>\\\n 	{ *string_buf_ptr++ = '\\'; *string_buf_ptr++ = '\n'; lineno++;}
<stringx>\\. 	{ *string_buf_ptr++ = '\\'; *string_buf_ptr++ = yytext[1];}
<stringx>[^\\\n\"]+		{ char *yptr = yytext; while (*yptr) *string_buf_ptr++ = *yptr++; }

<include>[ \t]* {}
<include>[^ \t\n);]+	{  
                 if ( include_stack_index >= MAX_INCLUDE_DEPTH )
                     {
                     fprintf( stderr, "******Error: Includes nested too deeply! An included file cannot include another file!\n\n" );
                     exit( 1 );
                     }
				 clean_file_name(yytext,filenamebuf);
                }
<include>")"		{}
<include>";"		{ FILE *yyin_save = yyin; struct nl *nlp;
 
                 include_stack[include_stack_index++] = YY_CURRENT_BUFFER;
					printf("       ... including file %s\n", filenamebuf);
                 yyin = fopen( filenamebuf, "r" );
			
                 if ( ! yyin )
				{
					
					fprintf(stderr,"******Error: Couldn't find the include file: %s; ignoring the Include directive!\n", filenamebuf);
                     include_stack_index--;
					yyin = yyin_save;
					BEGIN(INITIAL);
				}
				else
				{
                 yy_switch_to_buffer(
                     yy_create_buffer( yyin, YY_BUF_SIZE ) );
				 nlp = (struct nl*)calloc(sizeof(struct nl),1);
				 nlp->fname = (char*)malloc(strlen(filenamebuf)+1);
				 strcpy(nlp->fname,filenamebuf);
				 nlp->next = file_name_list;
				 file_name_list = nlp;
				 curr_file_save = curr_file;
				 curr_file = nlp->fname;
				 save_lineno = lineno;
				 save_lline = lline;
				 lineno = 1;
				 lline = 1;
                 BEGIN(INITIAL);
				}
         }


[a-zA-Z0-9!@#$%^&_+=\|~\?<>\.\-]+ {lline = lineno;liberty_parser_lval.str = my_strdup(yytext); return IDENT;}

<<EOF>>  {
                 if ( --include_stack_index < 0 )
                     {
                     yyterminate();
                     }

                 else
                     {
						 yy_delete_buffer( YY_CURRENT_BUFFER );
						 yy_switch_to_buffer(include_stack[include_stack_index] );
						 lineno = save_lineno;
						 lline = save_lline;
						 curr_file = curr_file_save;
                     }
}

%%

void clean_file_name(char *dirty, char *clean)
{
	/* clean (no quotes, no spaces) the name;
       then make sure it exists in the current dir or the path; if 
       it exists in the path, plug in the path component into the name,
       so the later fopen will find it */
	char cleanbuf[4096],*p,anotherbuf[4096];
	p = dirty;
	
    while( *p && (*p == '"' || *p == ' ' || *p == '\t' ) )
	{
		*p++;
	}
	
	strcpy( cleanbuf, p);
	
	p = cleanbuf + strlen(cleanbuf) -1;

	while (*p && p > cleanbuf && (*p == '"' || *p == ' ' || *p == '\t' ) )
		*p-- = 0;
	if( access(cleanbuf, F_OK ) == 0 )
	{
		strcpy(clean, cleanbuf);
		return;
	}
	else
	{
		/* hmmm, not in the current directory? */
		char enbuf[20256];
		
		char *en = getenv("LIBERTY_PARSER_SEARCH_PATH");
		if( !en )
		{
			strcpy(clean, cleanbuf);
			return;
		}
		else
		{
			char *q;
			
			strcpy(enbuf,en);
			strcat(enbuf,":");
			q = enbuf;
			
			while( q && *q && (p = strchr(q,':') ) )
			{
				*p = 0;
				
				if( p>enbuf && (*(p-1) == '/' || *(p-1) == '\\'))
					*(p-1) = 0;
				
				strcpy(anotherbuf,q);
				strcat(anotherbuf,"/");
				strcat(anotherbuf,cleanbuf);
				
				if( access(anotherbuf,F_OK) == 0 )
				{
					strcpy(clean,anotherbuf);
					return;
				}
				
				q = p+1;
			}
		}
	}
}

char *search_string_for_linefeeds(char *str)
{
   char *s;
   s = str;
   while (*s){ if( *s++ == '\n' ) {lineno++; if( *(s-2) != '\\' ){printf("Warning: line %d: String constant spanning input lines does not use continuation character.\n",lineno);} } }
   return str;
}

