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

#include <stdio.h>

/* divide_and_conquer.c */
void divide(point *p_sorted[], unsigned int l, unsigned int r, edge **l_ccw, edge **r_cw);

/* edge.c */
edge *join(edge *a, point *u, edge *b, point *v, side s);
void delete_edge(edge *e);
void splice(edge *a, edge *b, point *v);
edge *make_edge(point *u, point *v);

/* error.c */
void panic(char *m);

/* i_o.c */
void read_points(cardinal n, FILE* fp);
void print_results(cardinal n, char o);

/* memory.c */
void alloc_memory(cardinal n);
void free_memory(void);
edge *get_edge(void);
void free_edge(edge *e);

/* sort.c */
void merge_sort(point *p[], point *p_temp[], unsigned int l, unsigned int r);
