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

#include  <stddef.h>
#include  "defs.h"
#include  "decl.h"
#include  "dc.h"
#include  "edge.h"

static void lower_tangent(edge *r_cw_l, point *s, edge *l_ccw_r, point *u,
			  edge **l_lower, point **org_l_lower,
			  edge **r_lower, point **org_r_lower);

static void merge(edge *r_cw_l, point *s, edge *l_ccw_r, point *u, edge **l_tangent);

void divide(point *p_sorted[], unsigned int l, unsigned int r, edge **l_ccw, edge **r_cw)
{
  cardinal n;
  unsigned int split;
  edge *l_ccw_l, *r_cw_l, *l_ccw_r, *r_cw_r, *l_tangent;
  edge *a, *b, *c;
  real c_p;
  
  n = r - l + 1;
  if (n == 2) {
    /* Bottom of the recursion. Make an edge */
    *l_ccw = *r_cw = make_edge(p_sorted[l], p_sorted[r]);
  } else if (n == 3) {
    /* Bottom of the recursion. Make a triangle or two edges */
    a = make_edge(p_sorted[l], p_sorted[l+1]);
    b = make_edge(p_sorted[l+1], p_sorted[r]);
    splice(a, b, p_sorted[l+1]);
    c_p = Cross_product_3p(p_sorted[l], p_sorted[l+1], p_sorted[r]);
    
    if (c_p > 0.0)
    {
      /* Make a triangle */
      c = join(a, p_sorted[l], b, p_sorted[r], right);
      *l_ccw = a;
      *r_cw = b;
    } else if (c_p < 0.0) {
      /* Make a triangle */
      c = join(a, p_sorted[l], b, p_sorted[r], left);
      *l_ccw = c;
      *r_cw = c;
    } else {
      /* Points are collinear,  no triangle */ 
      *l_ccw = a;
      *r_cw = b;
    }
  } else if (n  > 3) {
    /* Continue to divide */

    /* Calculate the split point */
    split = (l + r) / 2;
  
    /* Divide */
    divide(p_sorted, l, split, &l_ccw_l, &r_cw_l);
    divide(p_sorted, split+1, r, &l_ccw_r, &r_cw_r);

    /* Merge */
    merge(r_cw_l, p_sorted[split], l_ccw_r, p_sorted[split+1], &l_tangent);

    /* The lower tangent added by merge may have invalidated 
       l_ccw_l or r_cw_r. Update them if necessary. */
    if (Org(l_tangent) == p_sorted[l])
      l_ccw_l = l_tangent;
    if (Dest(l_tangent) == p_sorted[r])
      r_cw_r = l_tangent;

    /* Update edge refs to be passed back */ 
    *l_ccw = l_ccw_l;
    *r_cw = r_cw_r;
  }
}

/*
 *  Determines the lower tangent of two triangulations. 
 */
static void lower_tangent(edge *r_cw_l, point *s, edge *l_ccw_r, point *u,
			  edge **l_lower, point **org_l_lower,
			  edge **r_lower, point **org_r_lower)
{
  edge *l, *r ;
  point *o_l, *o_r, *d_l, *d_r ;
  boolean finished ;

  l = r_cw_l ;
  r = l_ccw_r ;
  o_l = s ;
  d_l = Other_point(l, s) ;
  o_r = u ;
  d_r = Other_point(r, u) ;
  finished = FALSE ;

  while (!finished)
    if (Cross_product_3p(o_l, d_l, o_r) > 0.0) {
      l = Prev(l, d_l) ;
      o_l = d_l;
      d_l = Other_point(l, o_l) ;
    } else if (Cross_product_3p(o_r, d_r, o_l) < 0.0) {
      r = Next(r, d_r) ;
      o_r = d_r ;
      d_r = Other_point(r, o_r) ;
    } else
      finished = TRUE ;

  *l_lower = l ;
  *r_lower = r ;
  *org_l_lower = o_l ;
  *org_r_lower = o_r ;
}

/* 
 *  The merge function is where most of the work actually gets done.  It is
 *  written as one (longish) function for speed.
 */ 
static void merge(edge *r_cw_l, point *s, edge *l_ccw_r, point *u, edge **l_tangent)
{
  edge *base, *l_cand, *r_cand;
  point *org_base, *dest_base;
  real u_l_c_o_b, v_l_c_o_b, u_l_c_d_b, v_l_c_d_b;
  real u_r_c_o_b, v_r_c_o_b, u_r_c_d_b, v_r_c_d_b;
  real c_p_l_cand, c_p_r_cand;
  real d_p_l_cand, d_p_r_cand;
  boolean above_l_cand, above_r_cand, above_next, above_prev;
  point *dest_l_cand, *dest_r_cand;
  real cot_l_cand, cot_r_cand;
  edge *l_lower, *r_lower;
  point *org_r_lower, *org_l_lower;

  /* Create first cross edge by joining lower common tangent */
  lower_tangent(r_cw_l, s, l_ccw_r, u, &l_lower, &org_l_lower, &r_lower, &org_r_lower);
  base = join(l_lower, org_l_lower, r_lower, org_r_lower, right);
  org_base = org_l_lower;
  dest_base = org_r_lower;
  
  /* Need to return lower tangent. */
  *l_tangent = base;

  /* Main merge loop */
  do 
  {
    /* Initialise l_cand and r_cand */
    l_cand = Next(base, org_base);
    r_cand = Prev(base, dest_base);
    dest_l_cand = Other_point(l_cand, org_base);
    dest_r_cand = Other_point(r_cand, dest_base);

    /* Vectors for above and "in_circle" tests. */
    Vector(dest_l_cand, org_base, u_l_c_o_b, v_l_c_o_b);
    Vector(dest_l_cand, dest_base, u_l_c_d_b, v_l_c_d_b);
    Vector(dest_r_cand, org_base, u_r_c_o_b, v_r_c_o_b);
    Vector(dest_r_cand, dest_base, u_r_c_d_b, v_r_c_d_b);

    /* Above tests. */
    c_p_l_cand = Cross_product_2v(u_l_c_o_b, v_l_c_o_b, u_l_c_d_b, v_l_c_d_b);
    c_p_r_cand = Cross_product_2v(u_r_c_o_b, v_r_c_o_b, u_r_c_d_b, v_r_c_d_b);
    above_l_cand = c_p_l_cand > 0.0;
    above_r_cand = c_p_r_cand > 0.0;
    if (!above_l_cand && !above_r_cand)
      break;        /* Finished. */

    /* Advance l_cand ccw,  deleting the old l_cand edge,  until the 
       "in_circle" test fails. */
    if (above_l_cand)
    {
      real u_n_o_b, v_n_o_b, u_n_d_b, v_n_d_b;
      real c_p_next, d_p_next, cot_next;
      edge *next;
      point *dest_next;

      d_p_l_cand = Dot_product_2v(u_l_c_o_b, v_l_c_o_b, u_l_c_d_b, v_l_c_d_b);
      cot_l_cand = d_p_l_cand / c_p_l_cand;

      do 
      {
        next = Next(l_cand, org_base);
        dest_next = Other_point(next, org_base);
        Vector(dest_next, org_base, u_n_o_b, v_n_o_b);
        Vector(dest_next, dest_base, u_n_d_b, v_n_d_b);
        c_p_next = Cross_product_2v(u_n_o_b, v_n_o_b, u_n_d_b, v_n_d_b);
        above_next = c_p_next > 0.0;

        if (!above_next) 
          break;    /* Finished. */

        d_p_next = Dot_product_2v(u_n_o_b, v_n_o_b, u_n_d_b, v_n_d_b);
        cot_next = d_p_next / c_p_next;

        if (cot_next > cot_l_cand)
          break;    /* Finished. */

        delete_edge(l_cand);
        l_cand = next;
        cot_l_cand = cot_next;
  
      } while (TRUE);
    }

    /* Now do the symmetrical for r_cand */
    if (above_r_cand)
    {
      real u_p_o_b, v_p_o_b, u_p_d_b, v_p_d_b;
      real c_p_prev, d_p_prev, cot_prev;
      edge *prev;
      point *dest_prev;

      d_p_r_cand = Dot_product_2v(u_r_c_o_b, v_r_c_o_b, u_r_c_d_b, v_r_c_d_b);
      cot_r_cand = d_p_r_cand / c_p_r_cand;

      do
      {
        prev = Prev(r_cand, dest_base);
        dest_prev = Other_point(prev, dest_base);
        Vector(dest_prev, org_base, u_p_o_b, v_p_o_b);
        Vector(dest_prev, dest_base, u_p_d_b, v_p_d_b);
        c_p_prev = Cross_product_2v(u_p_o_b, v_p_o_b, u_p_d_b, v_p_d_b);
        above_prev = c_p_prev > 0.0;

        if (!above_prev) 
          break;    /* Finished. */

        d_p_prev = Dot_product_2v(u_p_o_b, v_p_o_b, u_p_d_b, v_p_d_b);
        cot_prev = d_p_prev / c_p_prev;

        if (cot_prev > cot_r_cand)
          break;    /* Finished. */

        delete_edge(r_cand);
        r_cand = prev;
        cot_r_cand = cot_prev;

      } while (TRUE);
    }

    /*
     *  Now add a cross edge from base to either l_cand or r_cand. 
     *  If both are valid choose on the basis of the in_circle test . 
     *  Advance base and  whichever candidate was chosen.
     */
    dest_l_cand = Other_point(l_cand, org_base);
    dest_r_cand = Other_point(r_cand, dest_base);
    if (!above_l_cand || (above_l_cand && above_r_cand && cot_r_cand < cot_l_cand))
    {
      /* Connect to the right */
      base = join(base, org_base, r_cand, dest_r_cand, right);
      dest_base = dest_r_cand;
    } else {
      /* Connect to the left */
      base = join(l_cand, dest_l_cand, base, dest_base, right);
      org_base = dest_l_cand;
    }

  } while (TRUE);
}
