/*
 *     This  file  is  part  of  the  Cadence  LEF/DEF  Open   Source
 *  Distribution,  Product Version 5.5, and is subject to the Cadence
 *  LEF/DEF Open Source License Agreement.   Your  continued  use  of
 *  this file constitutes your acceptance of the terms of the LEF/DEF
 *  Open Source License and an agreement to abide by its  terms.   If
 *  you  don't  agree  with  this, you must remove this and any other
 *  files which are part of the distribution and  destroy any  copies
 *  made.
 *
 *     For updates, support, or to become part of the LEF/DEF Community,
 *  check www.openeda.org for details.
 */


void AddStringDefine(char *token, char *string);
void AddBooleanDefine(char *token, int val);
void AddNumDefine(char *token, double val);
void yyerror(const char *s);
void yydefwarning(const char *s);
void* defMalloc(int def_size);
void* defRealloc(void *name, int def_size);
void defFree(void *name);
#ifdef __cplusplus
extern "C" int yylex();
extern "C" void lex_init();
extern "C" int defyyparse();
extern "C" void lex_un_init();
#else
extern int yylex();
extern void lex_init();
extern int defyyparse();
extern void lex_un_init();
#endif
int fake_ftell();
