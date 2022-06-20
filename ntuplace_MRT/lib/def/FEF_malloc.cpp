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


/* FEF_malloc.cpp */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "lex.h"
#include "FEF_malloc.h"
#include "FEF_util.h"
#include "defiDebug.hpp"

extern char defDebug[100];


/**********************************************************************/
/**********************************************************************/
void *FEF_calloc(int num_elem,int elem_size,const char *comment)
{
int n = num_elem*elem_size;
void *ptr;
if(defDebug[1])
	printf("%s calloc -> %d * %d = %d bytes"
		,comment,num_elem,elem_size,n);
if(num_elem<1) defrNerr(169);
if(elem_size<1) defrNerr(169);
ptr =  calloc(num_elem,elem_size);
if( ! ptr ) NoMemory();
return ptr;
}
/**********************************************************************/
/**********************************************************************/
void *FEF_malloc(int num_bytes,const char *comment)
{
void *ptr;
if(defDebug[1])
	printf("%s malloc -> %d bytes",comment,num_bytes);
if(num_bytes<1) defrNerr(115);
ptr =  defMalloc(num_bytes);
if( ! ptr ) NoMemory();
return ptr;
}
/**********************************************************************/
void *FEF_free_then_malloc(void *ptr,int num_bytes,const char *comment)
{
if(defDebug[1])
	printf("%s free, then malloc -> %d bytes",comment,num_bytes);
if(num_bytes<1) defrNerr(116);
if(ptr)
	{
	free((char*)ptr);
#	ifdef WIN32
	_heapmin();
#	endif
	}
ptr =  defMalloc(num_bytes);
if( ! ptr ) NoMemory();
return ptr;
}
/**********************************************************************/
void *FEF_realloc(void *ptr,int num_bytes,const char *comment)
{
if(defDebug[1])
	printf("%s realloc -> %d bytes",comment,num_bytes);
if(num_bytes<1) defrNerr(117);
if(ptr)
	ptr = defRealloc((char*)ptr,num_bytes);
else
	ptr = defMalloc(num_bytes);
if( ! ptr ) NoMemory();
return ptr;
}
/**********************************************************************/
/**********************************************************************/
void *FEF_relax(void *ptr,int num_bytes,const char *comment)
{
void *old_ptr;
void *new_ptr;
if(defDebug[1])
	printf("%s FEF_relax(defRealloc) -> %d bytes",comment,num_bytes);
if(!ptr) defrNerr(168);
old_ptr = ptr;
new_ptr =  defRealloc((char*)ptr,num_bytes);
if(new_ptr != old_ptr)
   if(defDebug[0])
	defrNwarn(944);
return new_ptr;
}
/**********************************************************************/
/**********************************************************************/
void FEF_free(void *ptr,const char *comment)
{
if(defDebug[1])
	printf("%s free",comment);
if(ptr)
	{
	free((char*)ptr);
#	ifdef WIN32
	_heapmin();
#	endif
	}
else  defrNerr(118);
}
/**********************************************************************/
/**********************************************************************/
void NoMemory() {
  defiError("ERROR **** out of memory ***\n");
  exit(1);
}
/**********************************************************************/
/**********************************************************************/
/* Replacement for malloc() that keeps track of what's out, and can free it all*/
static int nout = 0;
static int array_size = 0;
static char **what;

char *memory_malloc(unsigned n)
{
if (array_size == 0) {  /* first time */
    array_size = 64;
    what = (char **)defMalloc(array_size * sizeof(char *));
    if (what == NULL)
	NoMemory();
    }
nout++;
if (nout > array_size) {
    array_size <<= 1;
    what = (char **)defRealloc((char *)what, array_size*sizeof(char *));
    if (what == NULL)
	NoMemory();
    }
what[nout-1] = (char *)defMalloc(n);
if (what[nout-1] == NULL)
    NoMemory();
return what[nout-1];
}

void memory_free()
{
int i;
for(i=0; i<nout; i++)
    free(what[i]);
if(what != NULL)
    free((char *)what);
#ifdef WIN32
_heapmin();
#endif
}
