/*   Author: Geoff Leach, Department of Computer Science, RMIT.
 *   email: gl@cs.rmit.edu.au
 *
 *   Date: 6/10/93
 *
 *   Version 1.0
 *   
 *   Copyright (c) RMIT 1993. All rights reserved.
 *
 *   License to copy and use this software purposes is granted provided 
 *   that appropriate credit is given to both RMIT and the author.
 *
 *   License is also granted to make and use derivative works provided
 *   that appropriate credit is given to both RMIT and the author.
 *
 *   RMIT makes no representations concerning either the merchantability 
 *   of this software or the suitability of this software for any particular 
 *   purpose.  It is provided "as is" without express or implied warranty 
 *   of any kind.
 *
 *   These notices must be retained in any copies of any part of this software.
 */

#include  <stdlib.h>
#include  "defs.h"
#include  "decl.h"

point *p_array;
static edge *e_array;
static edge **free_list_e;
static cardinal n_free_e;

void alloc_memory(cardinal n)
{
    edge *e;
    unsigned int i;

    /* Point storage. */
    p_array = (point *)calloc(n, sizeof(point));
    if (p_array == NULL)
	panic("Not enough memory\n");

    /* Edges. */
    n_free_e = 3 * n;   /* Eulers relation */
    e_array = e = (edge *)calloc(n_free_e, sizeof(edge));
    if (e_array == NULL)
	panic("Not enough memory\n");

    free_list_e = (edge **)calloc(n_free_e, sizeof(edge *));
    if (free_list_e == NULL)
	panic("Not enough memory\n");

    for (i = 0; i < n_free_e; i++, e++)
	free_list_e[i] = e;
}

void free_memory(void)
{
    free(p_array);  
    free(e_array);  
    free(free_list_e);  
}

edge *get_edge(void)
{
    if (n_free_e == 0)
	panic("Out of memory for edges\n");

    return (free_list_e[--n_free_e]);
}

void free_edge(edge *e)
{
    free_list_e[n_free_e++] = e;
}






