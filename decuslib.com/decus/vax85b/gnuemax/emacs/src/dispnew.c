/* Newly written part of redisplay code. 
   Copyright (C) 1985 Richard M. Stallman. 
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
#include <stdio.h> 
#include <fcntl.h> 
#include <signal.h> 
#include <sys/ioctl.h> 
#include <sys/time.h> 
#undef NULL 
#include "config.h" 
#include "display.h" 
#include "lisp.h" 
#include "buffer.h" 
#include "window.h" 
#include "commands.h" 
 
#define max(a, b) ((a) > (b) ? (a) : (b)) 
 
/* The functions on this page are the interface from xdisp.c to redisplay. 
 They take cursor position arguments in origin 0. 
 
 The only other interface into redisplay is through setting 
 cursX and cursY (in xdisp.c) and setting ScreenGarbaged. */ 
 
/* cancel_line eliminates any request to display a line at position `vpos' */ 
 
cancel_line (vpos) 
     int vpos; 
{ 
  ReleaseLine (DesiredScreen[vpos + 1]); 
  DesiredScreen[vpos + 1] = 0; 
} 
 
free_all_display_lines () 
{ 
  register struct display_line *p; 
  register int i; 
 
  for (p = FreeLines; p; p = p->next) 
    free (p); 
  FreeLines = 0; 
 
  for (i = 0; i <= MScreenLength; i++) 
    if (PhysScreen[i]) 
      { 
	free (PhysScreen[i]); 
	PhysScreen[i] = 0; 
      } 
  ScreenGarbaged = 1; 
} 
 
/* Get a display_line for displaying on line `vpos' 
 and set it up for outputting starting at `hpos' within it.  */ 
 
struct display_line * 
get_display_line (vpos, hpos) 
     int vpos; 
     register int hpos; 
{ 
  register struct display_line *line; 
  register char *p; 
 
  line = DesiredScreen[vpos + 1]; 
  if (line && line->length > hpos) 
    abort (); 
  if (!line) 
    line = new_display_line (); 
 
  if (hpos > line->length) 
    { 
      p = line->body + line->length; 
      hpos -= line->length; 
      line->length += hpos; 
      while (--hpos >= 0) 
	*p++ = ' '; 
    } 
 
  DesiredScreen[vpos + 1] = line; 
 
  line->hash = 0; 
 
  return line; 
} 
 
/* Scroll lines from vpos `from' up to but not including vpos `end' 
 down by `amount' lines (`amount' may be negative). 
 Returns nonzero if done, zero if terminal cannot scroll them. */ 
 
int 
scroll_screen_lines (from, end, amount) 
     int from, end, amount; 
{ 
  register int i; 
 
  if (tt.t_ILov == MissingFeature || tt.t_DLov == MissingFeature) 
    return 0; 
 
  if (amount == 0) 
    return 1; 
  if (amount > 0) 
    { 
      topos (end + 1, 1); 
      deletelines (amount); 
      topos (from + 1, 1); 
      insertlines (amount); 
 
      for (i = end + amount; i >= end + 1; i--) 
	ReleaseLine (PhysScreen[i]); 
      for (i = end; i >= from + 1; i--) 
	PhysScreen[i + amount] = PhysScreen[i]; 
      for (i = from + amount; i >= from + 1; i--) 
	PhysScreen[i] = 0; 
    } 
  if (amount < 0) 
    { 
      topos (from + amount + 1, 1); 
      deletelines (-amount); 
      topos (end + amount + 1, 1); 
      insertlines (-amount); 
 
      for (i = from + amount + 1; i <= from; i++) 
	ReleaseLine (PhysScreen[i]); 
      for (i = from + 1; i <= end ; i++) 
	PhysScreen[i + amount] = PhysScreen[i]; 
      for (i = end + amount + 1; i <= end; i++) 
	PhysScreen[i] = 0; 
    } 
  return 1; 
} 
 
/* After updating a window w that isn't the full screen wide, 
 copy all the columns that w does not occupy 
 into the DesiredScreen lines from the PhysScreen lines 
 so that UpdateScreen will not change those columns. */ 
 
preserve_other_columns (w) 
     struct window *w; 
{ 
  register int vpos; 
  register struct display_line *l1, *l2; 
  int start = XFASTINT (w->left); 
  int end = XFASTINT (w->left) + XFASTINT (w->width); 
  int bot = XFASTINT (w->top) + XFASTINT (w->height); 
 
  for (vpos = XFASTINT (w->top); vpos < bot; vpos++) 
    { 
      if ((l1 = DesiredScreen[vpos + 1]) 
	  && (l2 = PhysScreen[vpos + 1])) 
	{ 
	  if (start > 0) 
	    { 
	      bcopy (l2->body, l1->body, start); 
	      if (l1->length < l2->length) 
		l1->length = l2->length; 
	    } 
	  if (l2->length > end && l1->length < l2->length) 
	    { 
	      while (l1->length < end) 
		l1->body[l1->length++] = ' '; 
	      bcopy (l2->body + end, l1->body + end, l2->length - end); 
	      l1->length = l2->length; 
	    } 
	} 
    } 
} 
 
/* If window w does not need to be updated and isn't the full screen wide, 
 copy all the columns that w does occupy 
 into the DesiredScreen lines from the PhysScreen lines 
 so that UpdateScreen will not change those columns. */ 
 
preserve_my_columns (w) 
     struct window *w; 
{ 
  register int vpos, fin; 
  register struct display_line *l1, *l2; 
  int start = XFASTINT (w->left); 
  int end = XFASTINT (w->left) + XFASTINT (w->width); 
  int bot = XFASTINT (w->top) + XFASTINT (w->height); 
 
  for (vpos = XFASTINT (w->top); vpos < bot; vpos++) 
    { 
      if ((l1 = DesiredScreen[vpos + 1]) 
	  && (l2 = PhysScreen[vpos + 1])) 
	{ 
	  if (l2->length > start && l1->length < l2->length) 
	    { 
	      fin = l2->length; 
	      if (fin > end) fin = end; 
	      while (l1->length < start) 
		l1->body[l1->length++] = ' '; 
	      bcopy (l2->body + start, l1->body + start, fin - start); 
	      l1->length = fin; 
	    } 
	} 
    } 
} 
 
/* On discovering that the redisplay for a window was no good, 
 cancel the columns of that window, 
 so that when the window is displayed over again 
 get_display_line will not complain. */ 
 
cancel_my_columns (w) 
     struct window *w; 
{ 
  register int vpos, fin; 
  register struct display_line *l; 
  register int start = XFASTINT (w->left); 
  register int bot = XFASTINT (w->top) + XFASTINT (w->height); 
 
  for (vpos = XFASTINT (w->top); vpos < bot; vpos++) 
    { 
      if ((l = DesiredScreen[vpos + 1]) 
	  && l->length >= start) 
	l->length = start; 
    } 
} 
 
direct_output_for_insert (c) 
     int c; 
{ 
  register struct display_line *p = PhysScreen[cursY]; 
#ifndef SUN 
  register 
#endif SUN 
    struct window *w = XWINDOW (selected_window); 
#ifndef SUN 
  register 
#endif SUN 
    int hpos = cursX - 1; 
 
  /* Give up if about to continue line */ 
  if (hpos - XFASTINT (w->left) + 1 + 1 >= XFASTINT (w->width)) 
    return; 
 
  /* Avoid losing if cursor is in invisible text off left margin */ 
  if (XINT (w->hscroll) && hpos == XFASTINT (w->left)) 
    return; 
     
  /* Give up if cursor outside window (in minibuf, probably) */ 
  if (cursY - 1 < XFASTINT (w->top) 
      || cursY - 1 >= XFASTINT (w->top) + XFASTINT (w->height)) 
    return; 
 
  /* Give up if cursor not really at cursX, cursY */ 
  if (!display_completed) 
    return; 
 
  /* Give up if w is minibuffer and a message is being displayed there */ 
  if (EQ (selected_window, minibuf_window) && minibuf_message) 
    return; 
 
  p->body[hpos] = c; 
  p->hash = 0; 
  unchanged_modified = bf_modified; 
  beg_unchanged = bf_s1; 
  XFASTINT (w->last_dot) = dot; 
  XFASTINT (w->last_dot_x) = cursX; 
  XFASTINT (w->last_modified) = bf_modified; 
 
  INSmode (0); 
  dumpstring (p->body + hpos, p->body + hpos); 
  fflush (stdout); 
  p->length = max (p->length, cursX); 
  cursX++; 
} 
 
direct_output_forward_char (n) 
     int n; 
{ 
  register struct window *w = XWINDOW (selected_window); 
 
  /* Avoid losing if cursor is in invisible text off left margin */ 
  if (XINT (w->hscroll) && cursX - 1 == XFASTINT (w->left)) 
    return; 
 
  cursX += n; 
  XFASTINT (w->last_dot_x) = cursX; 
  XFASTINT (w->last_dot) = dot; 
  topos (cursY, cursX); 
  fflush (stdout); 
} 
 
/* Perform insert-lines and delete-lines operations 
 according to the results of CalcM. 
 Updates the contents of PhysScreen to record what was done. */ 
 
do_scrolling (matrix) 
     struct Msquare *matrix; 
{ 
  register struct Msquare *p; 
  register int i, j, insertcount; 
  struct { int count, pos; } queue[MScreenLength]; 
  int qi = 0; 
 
/* First do all deletions of lines; queue up insertions. 
  Also move lines to correct slots in PhysScreen */ 
 
  i = j = WindowSize; 
  insertcount = 0; 
 
  while (i > 0 || j > 0) 
    { 
      p = matrix + i * (WindowSize + 1) + j; 
      if (p -> fromi == i)	/* perform an insert */ 
	{ 
	  PhysScreen[j] = 0; 
	  j--; 
	  insertcount+= 1; 
	  continue; 
	} 
      if (insertcount) 
	queue[qi].count = insertcount, 
	queue[qi++].pos = j + 1, 
	insertcount = 0; 
      if (p -> fromj == j)  /* perform a delete */ 
	{ 
	  if (j != ScreenLength) 
	    { 
	      topos (p -> fromi + 1, 1); 
	      deletelines (i - p -> fromi); 
	    } 
	  i = p -> fromi; 
	} 
      else 
	{ 
	  /* Line was perhaps moved from position i to position j */ 
	  PhysScreen[j] = OPhysScreen[i]; 
	  j--, i--; 
	} 
    } 
 
  /* Check for insertion wanted at top of screen */ 
 
  if (insertcount) 
    queue[qi].count = insertcount, 
    queue[qi++].pos = j + 1, 
    insertcount = 0; 
 
  /* Now do all insertions */ 
 
  for (i = qi - 1; i >= 0; i--) 
    { 
      topos (queue[i].pos, 1); 
      insertlines (queue[i].count); 
    } 
} 
 
/* Called by terminal drivers when padding is required: first argument is 
   number of times to repeat, second is time delay in microseconds. */ 
pad (n, t) 
     int n; 
     register long t; 
{ 
  t +=  5L; 
  t /= 10L;			/* actual resolution in 10us chunks */ 
  t *= (long) n; 
  if (baud_rate >= 1200)	/* try very hard to avoid overflow */ 
    { 
      t *= (long) (baud_rate / 100); 
      t +=  5000L; 
      t /= 10000L; 
    } 
  else if (baud_rate >= 150) 
    { 
      t *= (long) (baud_rate / 10); 
      t +=  50000L; 
      t /= 100000L; 
    } 
  else 
    { 
      t *= (long) baud_rate; 
      t +=  500000L; 
      t /= 1000000L; 
    } 
  while (--t >= 0) 
    putchar (tt.t_padc); 
} 
 
DEFUN ("set-screen-height", Fset_screen_height, Sset_screen_height, 1, 1, 0, 
  "Set number of lines on screen available for use in windows.") 
  (n) 
     Lisp_Object n; 
{ 
  CHECK_NUMBER (n, 0); 
  ChangeScreenSize (XINT (n), 0); 
  return Qnil; 
} 
 
DEFUN ("set-screen-width", Fset_screen_width, Sset_screen_width, 1, 1, 0, 
  "Set number of columns on screen available for display.") 
  (n) 
     Lisp_Object n; 
{ 
  CHECK_NUMBER (n, 0); 
  ChangeScreenSize (0, XINT (n)); 
  return Qnil; 
} 
 
DEFUN ("screen-height", Fscreen_height, Sscreen_height, 0, 0, 0, 
  "Return number of lines on screen available for use in windows.") 
  () 
{ 
  return make_number (ScreenLength); 
} 
 
DEFUN ("screen-width", Fscreen_width, Sscreen_width, 0, 0, 0, 
  "Return number of columns on screen available for display.") 
  () 
{ 
  return make_number (ScreenWidth); 
} 
 
/* Change the screen height and/or width.  Values may be given as zero to 
   indicate no change is to take place. */ 
ChangeScreenSize (newlength, newwidth) 
     register int newlength, newwidth; 
{ 
  if ((newlength == 0 || newlength == ScreenLength) 
	  && (newwidth == 0 || newwidth == ScreenWidth)) 
      return; 
  if (newlength && newlength != ScreenLength) 
    { 
      free_all_display_lines (); 
      if (newlength > MScreenLength) 
	newlength = MScreenLength; 
      set_window_height (XWINDOW (minibuf_window)->prev, newlength - 1); 
      XFASTINT (XWINDOW (minibuf_window)->top) = newlength - 1; 
      set_window_height (minibuf_window, 1); 
      ScreenLength = newlength; 
      IDValid = 0; 
    } 
  if (newwidth && newwidth != ScreenWidth) 
    { 
      if (newwidth > MScreenWidth) 
	newwidth = MScreenWidth; 
      set_window_width (XWINDOW (minibuf_window)->prev, newwidth); 
      set_window_width (minibuf_window, newwidth); 
      ScreenWidth = newwidth; 
      DCICValid = 0; 
    } 
  ScreenGarbaged++; 
  DoDsp (1); 
} 
 
DEFUN ("ding", Fding, Sding, 0, 0, "", 
  "Beep, or flash the screen.") 
  () 
{ 
  Ding (); 
  return Qnil; 
} 
 
/* DJH common routine for a feep */ 
Ding () {			/* BOGUS!  this should really be terminal 
				   type specific! */ 
  /* Stop executing a keyboard macro. */ 
  if (!INTERACTIVE) 
    error ("Ding"); 
 
  if (VisibleBell && tt.t_flash) 
    (*tt.t_flash) (); 
  else 
    putchar (07); 
  fflush (stdout); 
} 
 
/* ACT 18-Oct-1982 sleep for n seconds, do not update display */ 
DEFUN ("sleep-for", Fsleep_for, Ssleep_for, 1, 1, 0, 
  "Pause, without updating display, for ARG seconds.") 
  (n) 
     Lisp_Object n; 
{ 
  register int t; 
  struct timeval timeout, end_time, garbage1; 
 
  CHECK_NUMBER (n, 0); 
  t = XINT (n); 
  if (t <= 0) 
    return Qnil; 
 
#ifdef subprocesses 
  wait_reading_process_input (t, 0, 0); 
#else 
  immediate_quit = 1; 
  QUIT; 
 
  if (time_limit) 
    { 
      gettimeofday (&end_time, &garbage1); 
      end_time.tv_sec += t; 
    } 
 
  while (1) 
    { 
      gettimeofday (&timeout, &garbage1); 
      timeout.tv_sec = end_time.tv_sec - timeout.tv_sec; 
      timeout.tv_usec = end_time.tv_usec - timeout.tv_usec; 
      if (timeout.tv_usec < 0) 
	timeout.tv_usec += 1000000, 
      timeout.tv_sec--; 
      if (timeout.tv_sec < 0) 
	break; 
      if (!select (1, 0, 0, 0, &timeout)) 
	break; 
    } 
 
  immediate_quit = 0; 
#endif subprocesses 
  return Qnil; 
} 
 
DEFUN ("sit-for", Fsit_for, Ssit_for, 1, 1, 0, 
  "Perform redisplay, then wait for ARG seconds or until input is available") 
  (n) 
     Lisp_Object n; 
{ 
  struct timeval timeout; 
  int waitchannels; 
  Lisp_Object tem; 
 
  CHECK_NUMBER (n, 0); 
 
  if (detect_input_pending ()) 
    return Qnil; 
 
  DoDsp (1);			/* Make the screen correct */ 
  if (XINT (n) <= 0) return Qnil; 
 
#ifdef subprocesses 
  wait_reading_process_input (XINT (n), 1, 1); 
#else 
  immediate_quit = 1; 
  QUIT; 
 
  timeout.tv_sec = XINT (n);   
  timeout.tv_usec = 0; 
  waitchannels = 1; 
  select (1, &waitchannels, 0, 0, &timeout); 
 
  immediate_quit = 0; 
#endif subprocesses 
  return Qnil; 
} 
 
syms_of_display () 
{ 
  FreeLines = 0; 
 
  defsubr (&Sding); 
  defsubr (&Ssit_for); 
  defsubr (&Sscreen_height); 
  defsubr (&Sscreen_width); 
  defsubr (&Sset_screen_height); 
  defsubr (&Sset_screen_width); 
  defsubr (&Ssleep_for); 
 
  DefBoolVar ("inverse-video", &InverseVideo, 
    "*Non-nil means use inverse-video."); 
  DefBoolVar ("visible-bell", &VisibleBell, 
    "*Non-nil means try to flash the screen to represent a bell."); 
  DefBoolVar ("mode-line-highlight", &tt.t_modeline, 
    "*Non-nil means use inverse video, or other suitable display mode, for the mode line."); 
} 
