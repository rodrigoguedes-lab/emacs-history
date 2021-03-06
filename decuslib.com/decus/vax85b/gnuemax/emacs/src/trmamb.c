/* terminal control module for Ann Arbor Ambassador */ 
 
/*		Copyright (c) 1981,1980 James Gosling 
   Enhancements copyright (c) 1984 Fen Labalme and Chris Torek 
   Distributed by Fen Labalme, with permission from James Gosling. 
 
This file is part of GNU Emacs. 
 
GNU Emacs is distributed in the hope that it will be useful, 
but there is no warranty of any sort, and no contributor accepts 
responsibility for the consequences of using this program 
or for whether it serves any particular purpose. 
 
Everyone is granted permission to copy, modify and redistribute 
GNU Emacs, but only under the conditions described in the 
document "GNU Emacs copying permission notice".   An exact copy 
of the document is supposed to have been given to you along with 
GNU Emacs so that you can know how you may redistribute it all. 
It should be in a file named COPYING.  Among other things, the 
copyright notice and this notice must be preserved on any copy 
distributed.  */ 
 
 
#include <stdio.h> 
#include "config.h" 
#include "Trm.h" 
#include "cm.h" 
 
/* Above 4800 baud, the terminal usually needs padding.  Below 4800 baud, 
   it usually doesn't.  However, whenever a Graphics Master is attached, 
   all padding should be eliminated.  Thus the following defines: */ 
#define	PAD(n,f)	if (baud_rate > 4800) DOPAD (n, f); else 
#define DOPAD(n,f)	if (!HasGraphicsMaster) pad (n, f); else 
#define	CMD(n,o,m)	if (n == 1) printf (o); else printf (m, n) 
 
/* Note: I don't know if the Graphics Master works right at 19200 baud */ 
 
static int CurHL, DesHL, FrozenLines, DesIns, HasGraphicsMaster; 
extern int InverseVideo, baud_rate; 
 
static char *TEstr, *TIstr; 
 
extern char *getenv (); 
 
static 
INSmode (new) { 
    DesIns = new; 
} 
 
static 
HLmode (new) { 
    DesHL = new; 
} 
 
static 
SetHL (override) { 
    register int des = override ? 0 : DesHL; 
    if (des == CurHL) 
	return; 
    printf ("\033[%cm", InverseVideo ? (des ? '0' : '7') 
				     : (des ? '7' : '0')); 
    PAD (1, 300L);		/* 300us */ 
    CurHL = des; 
} 
 
static 
inslines (n) { 
    SetHL (1); 
    CMD (n, "\033[L", "\033[%dL"); 
    PAD (tt.t_length - curY, 1000L);/* 1ms per line */ 
} 
 
static 
dellines (n) { 
    SetHL (1); 
    CMD (n, "\033[M", "\033[%dM"); 
    PAD (tt.t_length - curY, 1000L);/* 1ms per line */ 
} 
 
static 
writechars (start, end) 
register char *start, *end; 
{ 
    register int n = end - start + 1; 
    SetHL (0); 
    if (DesIns) { 
	CMD (n, "\033[@", "\033[%d@"); 
	PAD (80 - curX, 160L);	/* 160us per char */ 
    } 
    cmplus (n); 
    while (start <= end) { 
	if (start + 5 < end && *start == start[1]) { 
	    register char *p = start; 
	    n = 0; 
	    while (++p <= end && *p == *start) 
		n++; 
	    if (n >= 5) { 
		printf ("%c\033[%db", *start, n); 
		PAD (n, 1000L);/* 1ms per char */ 
		start = p; 
		continue; 
	    } 
	} 
	putchar (*start++); 
    } 
} 
 
static 
update_end () 
{ 
  SetHL (1); 
} 
 
static 
blanks (n) 
register int n; 
{ 
    SetHL (0); 
    cmplus (n); 
    if (DesIns) { 
	CMD (n, "\033[@", "\033[%d@"); 
	PAD (80 - curX, 160L);	/* 160us per char */ 
    } 
    if (n > 5) { 
	printf (" \033[%db", n - 1); 
	PAD (n - 1, 1000L);	/* 1ms per char */ 
    } 
    else 
	while (--n >= 0) 
	    putchar (' '); 
} 
 
static 
topos (row, column) { 
    cmgoto (row - 1, column - 1); 
    PAD (1, 1000L);		/* 1ms */ 
} 
 
static 
init (BaudRate, tabok) { 
    tt.t_DLov = tt.t_ILov = 4; 
    tt.t_ICov = 4; 
    tt.t_DCov = 4; 
    if (BaudRate > 4800) { 
	register double BaudFactor = BaudRate / 10000.; 
	tt.t_DLnpf = tt.t_ILnpf = /* 1.0 * */ BaudFactor; 
	tt.t_ICmf = .16 * BaudFactor; 
	tt.t_DCmf = .20 * BaudFactor; 
    } 
    else 
	tt.t_DLnpf = tt.t_ILnpf = tt.t_ICmf = tt.t_DCmf = 0.0; 
    MetaFlag++; 
    UseTabs = tabok; 
    cmcostinit (); 
} 
 
static 
reset () { 
    ScreenRows = tt.t_length; 
    ScreenCols = tt.t_width; 
    CurHL = -1; 
    SetHL (1); 
    if (TIstr) 
      tputs (TIstr, 1, cmputc); 
    else 
      printf ("\033[%d;%d;0;%dp", tt.t_length + FrozenLines, 
	      FrozenLines, tt.t_length + FrozenLines); 
    DOPAD (1, 10000L);		/* 10ms */ 
    printf ("\033[>52;54h");	/* SM Metakey, Ignore NUL */ 
    printf ("\033[>30;33;34;37;38;39l"); 
				/* RM ZDBM,ZWFM,ZWBM,ZAXM,ZAPM,ZSSM */ 
    printf ("\033[1Q");		/* SEE */ 
    printf ("\033[%cm", InverseVideo ? '7' : '0'); 
    DesIns = 0; 
    losecursor (); 
    wipescreen (); 
} 
 
static 
cleanup () { 
/*  InverseVideo = 0;		/* reset to normal video */ 
    SetHL (1); 
    printf ("\033[>33;34h");	/* SM ZWFM,ZWBM */ 
    DOPAD (20, 1000L); 
    printf ("\033[>37h");	/* SM ZAXM */ 
    DOPAD (20, 1000L); 
    printf ("\033[>52l");	/* RM Metakey */ 
    DOPAD (10, 1000L); 
    if (TEstr) 
      tputs (TEstr, 1, cmputc); 
    else 
      { 
	if (tt.t_length != 60) 
	  printf ("\033[60;%d;0;%dp\033[%dH", FrozenLines, 
		  tt.t_length + FrozenLines, 
		  tt.t_length); 
	printf ("\033[J"); 
      } 
    DOPAD (10, 1000L);		/* 10ms */ 
} 
 
static 
wipeline () { 
    SetHL (1); 
    printf ("\033[K"); 
    PAD (80 - curX, 90L);	/* 90us per char */ 
} 
 
static 
wipescreen () { 
    SetHL (1); 
    printf ("\033[H\033[2J"); 
    DOPAD (tt.t_length, 7200L);	/* 7.2ms per line */ 
    cmat (0, 0); 
} 
 
static 
delchars (n) { 
    SetHL (1); 
    CMD (n, "\033[P", "\033[%dP"); 
    PAD (80 - curX, 200L);	/* 200us per char */ 
} 
 
TrmAmb (term) 
     char *term; 
{ 
  char tbuf[1024]; 
  char xbuf[1024]; 
  char *xbptr = xbuf; 
  register char *tem; 
 
  tt.t_length = 30; 
  FrozenLines = 0; 
  TEstr = 0; 
  TIstr = 0; 
  if (tgetent (tbuf, term) > 0) 
    { 
      tt.t_length = tgetnum ("li"); 
      if (tt.t_length < 0) 
	tt.t_length = 30; 
      FrozenLines = tgetnum ("fn"); 
      if (FrozenLines < 0) 
	{ 
	  if (tgetflag("hs")) 
	    FrozenLines = 1; 
	  else 
	    FrozenLines = 0; 
	} 
      TEstr = (char *) tgetstr ("te", &xbptr); 
      TIstr = (char *) tgetstr ("ti", &xbptr); 
      if (TEstr) 
	{ 
	  tem = (char *) malloc (strlen (TEstr) + 1); 
	  strcpy (tem, TEstr); 
	  TEstr = tem; 
	} 
      if (TIstr) 
	{ 
	  tem = (char *) malloc (strlen (TIstr) + 1); 
	  strcpy (tem, TIstr); 
	  TIstr = tem; 
	} 
    } 
  if (getenv ("NOPAD")) 
    HasGraphicsMaster++; 
  tt.t_INSmode = INSmode; 
  tt.t_HLmode = HLmode; 
  tt.t_inslines = inslines; 
  tt.t_dellines = dellines; 
  tt.t_blanks = blanks; 
  tt.t_init = init; 
  tt.t_cleanup = cleanup; 
  tt.t_wipeline = wipeline; 
  tt.t_wipescreen = wipescreen; 
  tt.t_topos = topos; 
  tt.t_reset = reset; 
  tt.t_delchars = delchars; 
  tt.t_writechars = writechars; 
  tt.t_UpdateEnd = update_end; 
  tt.t_window = 0; 
  tt.t_width = 80; 
 
  Wcm_clear (); 
  Up = "\033[A"; 
  Down = "\n"; 
  Left = "\b"; 
  Right = "\033[C"; 
  Home = "\033[H"; 
  CR = "\r"; 
  Tab = "\t"; 
  TabWidth = 8; 
  AbsPosition = "\033[%i%d;%dH"; 
  ColPosition = "\033[%i%dG"; 
  RowPosition = "\033[%i%dd"; 
  ScreenRows = tt.t_length; 
  ScreenCols = tt.t_width; 
  Wcm_init (); 
} 
