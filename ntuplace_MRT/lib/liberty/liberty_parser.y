
/* bison input file */
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
#include <stdio.h>
#include "liberty_structs.h"
#include "si2dr_liberty.h"
#include "mymalloc.h"
#include "group_enum.h"
#include "attr_enum.h"
	
static si2drGroupIdT gs[1000];
static int gsindex = 0;

static si2drErrorT   err;
static si2drAttrTypeT atype;
static si2drAttrIdT curr_attr;
static si2drDefineIdT curr_def;
void push_group(liberty_head *h );
void pop_group();
si2drValueTypeT convert_vt(char *type);
int lineno;
 int syntax_errors;
 static char PB[8000]; /* so as not to have a bunch of local buffers */
 
 extern char *curr_file;
 
void make_complex(liberty_head *h);
void make_simple(char *name, liberty_attribute_value *v);
 
 
%}

%union {
	char *str;
	double num;
	liberty_group *group;
	liberty_attribute *attr;
	liberty_attribute_value *val;
	liberty_define *def;
	liberty_head *head;
		
}


%token COMMA SEMI LPAR RPAR LCURLY RCURLY COLON KW_DEFINE KW_DEFINE_GROUP KW_TRUE KW_FALSE

%token <num> NUM
%token <str> STRING IDENT

%type <group>     group file statements statement
%type <def>       define define_group
%type <val> param_list attr_val
%type <str>	s_or_i
%type <head> head
%%

file	: {lineno = 1; syntax_errors= 0;} group {}
		;

group	: head LCURLY {push_group($1);} statements RCURLY {pop_group($1);}
        | head LCURLY {push_group($1);} RCURLY {pop_group($1);}
		;


statements 	: statement {}
		 	| statements statement  {}
			;


statement 	: simple_attr {}
			| complex_attr {}
			| define {}
			| define_group {}
			| group  {}
			;

simple_attr	: IDENT COLON attr_val { make_simple($1,$3);} SEMI
			| IDENT COLON attr_val { make_simple($1,$3);}
			;

complex_attr 	: head  SEMI  {make_complex($1);}
                | head  {make_complex($1);}
				;

head	: IDENT LPAR param_list RPAR { $$ = (liberty_head*)my_calloc(sizeof(liberty_head),1); $$->name = $1; $$->list = $3; $$->lineno = lineno;$$->filename = curr_file;}
        | IDENT LPAR RPAR            { $$ = (liberty_head*)my_calloc(sizeof(liberty_head),1); $$->name = $1; $$->list = 0; $$->lineno = lineno;$$->filename = curr_file;}
		;


param_list  : attr_val {$$=$1;}
            | param_list COMMA attr_val
              {
				  liberty_attribute_value *v;
				  for(v=$1; v; v=v->next)
				  {
					  if(!v->next)
					  {
						  v->next = $3;
						  break;
					  }
				  }
				  $$ = $1;
			  }
            | param_list attr_val
              {
				  liberty_attribute_value *v;
				  for(v=$1; v; v=v->next)
				  {
					  if(!v->next)
					  {
						  v->next = $2;
						  break;
					  }
				  }
				  $$ = $1;
			  }
			;

define 	: KW_DEFINE LPAR s_or_i COMMA s_or_i COMMA s_or_i RPAR SEMI  
		{curr_def = si2drGroupCreateDefine(gs[gsindex-1],$3,$5,convert_vt($7),&err);si2drObjectSetLineNo(curr_def,lineno,&err);si2drObjectSetFileName(curr_def,curr_file,&err);}
		;


define_group : KW_DEFINE_GROUP LPAR s_or_i COMMA s_or_i RPAR SEMI
            {curr_def = si2drGroupCreateDefine(gs[gsindex-1],$3,$5,SI2DR_UNDEFINED_VALUETYPE,&err);si2drObjectSetLineNo(curr_def,lineno,&err);si2drObjectSetFileName(curr_def,curr_file,&err);}
		;

s_or_i  : STRING {$$ = $1;}
		| IDENT {$$=$1;}
		;

attr_val : NUM { $$= (liberty_attribute_value*)my_calloc(sizeof(liberty_attribute_value),1);
                 $$->type = LIBERTY__VAL_DOUBLE;
				 $$->u.double_val = $1;
               }

         | s_or_i
               {
				   $$= (liberty_attribute_value*)my_calloc(sizeof(liberty_attribute_value),1);
				   $$->type = LIBERTY__VAL_STRING;
				   $$->u.string_val = $1;
			   }
         | KW_TRUE
               {
				   $$= (liberty_attribute_value*)my_calloc(sizeof(liberty_attribute_value),1);
				   $$->type = LIBERTY__VAL_BOOLEAN;
				   $$->u.bool_val = 1;
			   }
         | KW_FALSE
               {
				   $$= (liberty_attribute_value*)my_calloc(sizeof(liberty_attribute_value),1);
				   $$->type = LIBERTY__VAL_BOOLEAN;
				   $$->u.bool_val = 0;
			   }
		 ;
%%

void push_group(liberty_head *h )
{
	liberty_attribute_value *v,*vn;
	extern group_enum si2drGroupGetID(si2drGroupIdT group, 
									  si2drErrorT   *err);
	si2drErrorT err;
	
	si2drMessageHandlerT MsgPrinter;

	group_enum ge;
	MsgPrinter = si2drPIGetMessageHandler(&err); /* the printer is in another file! */
	
	
	if( gsindex == 0 )
	{
		
		gs[gsindex] = si2drPICreateGroup(0,h->name,&err);
	}
	
	else
	{
		gs[gsindex] = si2drGroupCreateGroup(gs[gsindex-1],0,h->name,&err);
	}
	

	ge = si2drGroupGetID(gs[gsindex],&err);
	
	if( err == SI2DR_OBJECT_ALREADY_EXISTS && ( ge != LIBERTY_GROUPENUM_internal_power ) )
	{
		sprintf(PB,"%s:%d: The group name %s is already being used in this context.",curr_file,lineno,h->name);
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  PB, 
					  &err);
	}
	if( err == SI2DR_INVALID_NAME )
	{
		sprintf(PB,"%s:%d: The group name \"%s\" is invalid.",curr_file,lineno,h->name);
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  PB, 
					  &err);
	}
	gsindex++;
	
	si2drObjectSetLineNo(gs[gsindex-1],h->lineno, &err);si2drObjectSetFileName(gs[gsindex-1],h->filename, &err);
	for(v=h->list;v;v=vn)
	{
		if( v->type != LIBERTY__VAL_STRING )
		{
			char buf[1000],*buf2;
			sprintf(buf,"%g",v->u.double_val);
			
			si2drGroupAddName(gs[gsindex-1],buf,&err);
			
			if( err == SI2DR_OBJECT_ALREADY_EXISTS && ( ge != LIBERTY_GROUPENUM_internal_power ))
			{
				sprintf(PB,"%s:%d: The group name %s is already being used in this context.",
					   curr_file,lineno,buf);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  PB, 
							  &err);
			}
			if( err == SI2DR_INVALID_NAME )
			{
				sprintf(PB,"%s:%d: The group name \"%s\" is invalid.",curr_file,lineno,buf);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  PB, 
							  &err);
			}
		}
		else
		{
			
			si2drGroupAddName(gs[gsindex-1],v->u.string_val,&err);

			if( err == SI2DR_OBJECT_ALREADY_EXISTS && ( ge != LIBERTY_GROUPENUM_internal_power ))
			{
				sprintf(PB,"%s:%d: The group name %s is already being used in this context.",
						curr_file,lineno,v->u.string_val);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  PB, 
							  &err);
			}
			if( err == SI2DR_INVALID_NAME )
			{
				sprintf(PB,"%s:%d: The group name \"%s\" is invalid. It will not be added to the database\n",
					   curr_file,lineno,v->u.string_val);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  PB, 
							  &err);
			}
		}
		
		vn = v->next;
		my_free(v);
	}
}

void pop_group(liberty_head *h)
{
	gsindex--;
}

void make_complex(liberty_head *h)
{
	liberty_attribute_value *v,*vn;
	
	curr_attr=si2drGroupCreateAttr(gs[gsindex-1],h->name,SI2DR_COMPLEX,&err);
	
	si2drObjectSetLineNo(curr_attr,h->lineno, &err);
	si2drObjectSetFileName(curr_attr,h->filename, &err);
	for(v=h->list;v;v=vn)
	{
		if( v->type == LIBERTY__VAL_BOOLEAN )
			si2drComplexAttrAddBooleanValue(curr_attr,v->u.bool_val,&err);
		else if( v->type != LIBERTY__VAL_STRING )
			si2drComplexAttrAddFloat64Value(curr_attr,v->u.double_val,&err);
		else
			si2drComplexAttrAddStringValue(curr_attr,v->u.string_val,&err);		

		vn = v->next;
		my_free(v);
	}
}

void make_simple(char *name, liberty_attribute_value *v)
{
	curr_attr=si2drGroupCreateAttr(gs[gsindex-1],name,SI2DR_SIMPLE,&err);
	
	si2drObjectSetLineNo(curr_attr,lineno, &err);
	si2drObjectSetFileName(curr_attr,curr_file, &err);
	if( v->type == LIBERTY__VAL_BOOLEAN )
		si2drSimpleAttrSetBooleanValue(curr_attr,v->u.bool_val,&err);
	else if( v->type != LIBERTY__VAL_STRING )
		si2drSimpleAttrSetFloat64Value(curr_attr,v->u.double_val,&err);
	else
		si2drSimpleAttrSetStringValue(curr_attr,v->u.string_val,&err);
	my_free(v);
}



si2drValueTypeT convert_vt(char *type)
{
	if( !strcmp(type,"string") )
		return SI2DR_STRING;
	if( !strcmp(type,"integer") )
		return SI2DR_INT32;
	if( !strcmp(type,"float") )
		return SI2DR_FLOAT64;
	
}

yyerror(char *s)
{
	si2drErrorT err;
	
	si2drMessageHandlerT MsgPrinter;

	MsgPrinter = si2drPIGetMessageHandler(&err); /* the printer is in another file! */

	sprintf(PB,"===\nERROR === %s file: %s, line number %d\nERROR ===", s, curr_file, lineno);
	(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
				  PB, 
				  &err);
	
	syntax_errors++;
}
