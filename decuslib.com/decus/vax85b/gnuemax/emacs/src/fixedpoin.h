/* fixed point arithmetic with 8 fraction bits, 
    used for calculations of redisplay costs. 
   Copyright (C) 1984 Richard M. Stallman, 
    based mostly on public domain code written by Chris Torek. 
 
This file is part of GNU Emacs. 
 
GNU Emacs is distributed in the hope that it will be useful, 
but without any warranty.  No author or distributor 
accepts responsibility to anyone for the consequences of using it 
or for whether it serves any particular purpose or works at all, 
unless he says so in writing. 
 
Everyone is granted permission to copy, modify and redistribute 
GNU Emacs, but only under the conditions described in the 
document "GNU Emacs copying permission notice".   An exact copy 
of the document is supposed to have been given to you along with 
GNU Emacs so that you can know how you may redistribute it all. 
It should be in a file named COPYING.  Among other things, the 
copyright notice and this notice must be preserved on all copies.  */ 
 
 
 
typedef long fixedpoint; 
#define fltofixp(f)	((long) ((f)*256)) 
#define itofixp(i)	(((long) (i))<<8) 
#define fixptofl(f)	((f)/256.) 
#define fixptoi(f)	((f)>>8) /* presumably these are unsigned */ 
/* addition/subtraction of fixed point is equiv. to integer */ 
#define fixpmul(a,b)	(((a)*(b))>>8) 
#define fixpdiv(a,b)	(((a)<<8)/(b)) 
 
/* See CalcIDCosts for comments on the arrays below */ 
fixedpoint ILcost[MScreenLength];/* ov(n) + 1*mf(n) */ 
fixedpoint DLcost[MScreenLength];/* ov(n) + 1*mf(n) */ 
fixedpoint ILncost[MScreenLength];/* mf(n) */ 
fixedpoint DLncost[MScreenLength];/* mf(n) */ 
int IDValid;		/* true iff ILcost etc valid */ 
 
fixedpoint DC_ICcost[1 + 2 * MScreenWidth]; 
#define DCICcost (&DC_ICcost[MScreenWidth]) 
int DCICValid;		/* true iff DC_ICcost valid */ 
 
/* It turns out that the values across the top of the matrix computed by 
   calcM are (constant + variable); in yet another optimization we put 
   the constants in ILinit. */ 
fixedpoint ILinit[MScreenLength]; 
