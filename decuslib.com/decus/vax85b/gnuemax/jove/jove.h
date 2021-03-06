/* jove.h header file to be included by EVERYONE */ 
 
#include <setjmp.h> 
#include <sys/types.h> 
 
#include "jove_tune.h" 
 
#ifdef MENLO_JCL 
#	ifndef EUNICE 
#		ifndef JOVE4.2 
#			define signal	sigset 
#		endif JOVE4.2 
#	endif EUNICE 
#endif MENLO_JCL 
 
#ifndef JOVE4.2 
extern int	(*sigset())();		/* Sigset returns a pointer to a procedure */ 
#endif 
 
#define EOF	-1 
#define NULL	0 
 
#define OKAY	0		/* Return codes for when telling */ 
#define ABORT	1 
#define STOP	2 
 
/* Return codes for command completion (all < 0 because >= 0 are 
   legitimate offsets into array of strings. */ 
 
#define AMBIGUOUS	-2	/* Matches more than one at this point */ 
#define UNIQUE		-3	/* Matches only one string */ 
#define ORIGINAL	-4	/* Matches no strings at all! */ 
#define NULLSTRING	-5	/* Just hit return without typing anything */ 
 
/* Values for the `flags' argument to complete */ 
#define NOTHING		0	/* Opposite of RET_STATE */ 
#define RET_STATE	1	/* Return state when we hit return */ 
#define RCOMMAND	2	/* We are reading a joverc file */ 
#define LF_FORCE	4	/* LF means eat this literally */ 
#define DIR_ONLY	8       /* asking for dir spec */ 
#define FIL_ONLY	16      /* asking for fil spec */ 
 
#define FUNCTION	1 
#define VARIABLE	2 
#define MACRO		3 
 
#define DEFINE		1 
#define EXECUTE		2 
 
#define	LBSIZE		BUFSIZ	/* logical disk block */ 
#define	ESIZE		128	/* RE size */ 
#define	GBSIZE		256 
 
#define RMARGIN		72	/* Default right margin */ 
 
#define flusho()	flushout(-1, &termout) 
#define Placur(l, c)	if (l != CapLine || c != CapCol) DoPlacur(l, c)  
 
#define CTL(c)		('c' & 037) 
#define META(c)		('c' | 0200) 
 
#define	eolp()		(linebuf[curchar] == '\0') 
#define bolp()		(curchar == 0) 
#define	lastp(line)	(line == curbuf->b_last) 
#define	firstp(line)	(line == curbuf->b_first) 
#define eobp()		(lastp(curline) && eolp()) 
#define bobp()		(firstp(curline) && bolp()) 
#define HALF(wp)	((wp->w_height - 1) / 2) 
#define SIZE(wp)	(wp->w_height - 1) 
#define makedirty(line)	line->l_dline |= DIRTY 
#define isdirty(line)	(line->l_dline & DIRTY) 
#define IsModified(b)	(b->b_modified) 
 
#define DoTimes(f, n)	exp_p = 1, exp = n, f() 
 
extern int	CheckTime, 
		BufSize; 
 
/* This procedure allows redisplay to be aborted if the buffer is 
 * ready to be flushed, and there are some characters waiting 
 */ 
 
#define outchar(c) Putc(c, (&termout)) 
#define Putc(x,p) (--(p)->io_cnt>=0 ? ((int)(*(p)->io_ptr++=(unsigned)(x))):flushout(x, p)) 
 
/* 
 * C doesn't have a (void) cast, so we have to fake it for lint's sake. 
 */ 
#ifdef lint 
#	define	ignore(a)	Ignore((char *) (a)) 
#	define	ignorf(a)	Ignorf((int (*) ()) (a)) 
#else 
#	define	ignore(a)	a 
#	define	ignorf(a)	a 
#endif 
 
#define ARG_CMD		1 
#define line_CMD	2 
#define KILLCMD		3	/* So we can merge kills */ 
#define YANKCMD		4 
#define TABCMD		5       /* 2 tabs in a row is a quoted insert */ 
 
/* Buffer type */ 
 
#define SCRATCH		1	/* For internal things, e.g. list-buffers ... */ 
#define FILE		2	/* Normal file. (We Auto-save these.) */ 
#define PROCESS		3	/* Process output in this buffer */ 
#define IPROCESS	4	/* Interactive process attached to this buffer */ 
 
/* Major modes */ 
#define TEXT		0	/* Normal mode */ 
#define CMODE		1	/* C mode */ 
#define CLUMODE		2 
#define EUCLIDMODE	3 
#define SCRIBEMODE	4 
#define TEXMODE		5 
#define ASMMODE		6 
#define PASCALMODE	7 
#define FUNDMODE	8 
 
/* Minor Modes */ 
#define Indent		(1 << 0)	/* Indent same as previous line after return */ 
#define ShowMatch	(1 << 1)	/* In show matching mode */ 
#define Fill		(1 << 2)	/* Text fill mode */ 
#define Save		(1 << 3)	/* Auto Save this buffer */ 
#define OverWrite	(1 << 4)	/* Over write mode */ 
 
#define BufMinorMode(b, x)	(b->b_minor & x) 
 
#define MinorMode(x)	BufMinorMode(curbuf, x) 
#define MajorMode(x)	(curbuf->b_major == x) 
#define SetMajor(x)	(curbuf->b_major = x) 
#define SetMinor(x)	(curbuf->b_minor |= x) 
 
extern int	DefMinor; 
 
#define FIRSTCALL	0 
#define ERROR		1 
#define COMPLAIN	2	/* Do the error without a getDOT */ 
#define QUIT		3	/* Leave this level of recusion */ 
 
extern char	*Mainbuf, 
		*Shflags,	/* See jove_proc.c */ 
		*DefShell,	/* See main() */ 
		key_strokes[],	/* Strokes that make up current command */ 
		*Inputp, 
		CommentBegin[],	/* see jove_insert.c */ 
		CommentEnd[];	/* and jove_delete.c */ 
 
extern char	NullStr[]; 
extern disk_line	NullLine;	/* NullLine = putline("") */ 
 
extern char	genbuf[];	/* Scatch pad */ 
 
#define	READ	0 
#define	WRITE	1 
 
extern jmp_buf	mainjmp; 
 
typedef struct window	Window; 
typedef struct position	Bufpos; 
typedef struct mark	Mark; 
typedef struct buffer	Buffer; 
typedef struct line	Line; 
typedef struct iobuf	IOBUF; 
 
typedef int	(*FUNC)(); 
 
struct iobuf { 
	char	*io_ptr; 
	int	io_cnt; 
	char	*io_base; 
	char	io_fd; 
}; 
 
struct line { 
	Line	*l_prev,	/* Pointer to prev */ 
		*l_next;	/* Pointer to next */ 
	disk_line	l_dline;	/* Pointer to disk location */ 
}; 
 
struct window { 
	Window	*w_prev,	/* Circular list */ 
		*w_next; 
	Buffer	*w_bufp;	/* Buffer associated with this window */ 
	Line	*w_top,		/* The top line */ 
		*w_line;	/* The current line */ 
	int	w_char, 
		w_height,	/* Height of the window */ 
		w_topnum,	/* Line number of the topline */ 
		w_offset,	/* Printing offset for the current line */ 
		w_numlines,	/* Should we number lines in this window? */ 
		w_dotcol,	/* find_pos sets this */ 
		w_dotline,	/* UpdateWindow sets this */ 
		w_flags; 
}; 
 
extern Window	*fwind,		/* First window in list */ 
		*curwind;	/* Current window */ 
 
struct position { 
	Line	*p_line;	/* In the list */ 
	int	p_char;		/* Char pos */ 
}; 
 
struct mark { 
	Line	*m_line; 
	int	m_char;		/* Char pos of the mark */ 
	Mark	*m_next;	/* List of marks */ 
#define FLOATER	1 
	char	m_floater;	/* FLOATERing mark? */ 
}; 
 
struct buffer { 
	Buffer	*b_next;		/* Next buffer in chain */ 
	char	*b_name,		/* Buffer name */ 
		*b_fname;		/* File name associated with buffer */ 
	ino_t	b_ino;			/* Inode of file name */ 
	time_t	b_ctime;		/* Last create time ... 
					   to detect two people writing 
					   to the same file ... eventually */ 
	Line	*b_first,		/* Pointer to first line in list */ 
		*b_dot,			/* Current line */ 
		*b_last;		/* Last line in list */ 
	int	b_char;			/* Current character in line */ 
 
#define NMARKS	16			/* Number of marks in the ring */ 
 
	Mark	*b_markring[NMARKS],	/* New marks are pushed saved here */ 
		*b_marks;		/* All the marks for this buffer */ 
	int	b_themark;		/* Current mark */ 
	char	b_type;			/* Scratch? */ 
	int	b_modified,		/* Number of modifications since 
					   last (auto) save */ 
		b_major,		/* Major mode for this buffer */ 
		b_minor;		/* and minor mode */ 
}; 
 
extern Buffer	*world,			/* First buffer */ 
		*curbuf;		/* Pointer into world for current buffer */ 
 
/* Max terminal lines */ 
#define MAXWIDTH	132	/* for allocation purposes... */ 
#define MAXNLINES	80	/* Change it if you want to! */ 
 
#define NUMKILLS	10	/* Number of kills saved in the kill ring */ 
 
#define DIRTY		01 
#define MODELINE	02 
 
struct scrimage { 
	int	StartCol,	/* Physical column start */ 
		Sflags;		/* DIRTY or MODELINE */ 
	Line	*Line;		/* Which buffer line */ 
	Window	*Window;	/* Window that contains this line */ 
}; 
 
extern struct scrimage 
	*nimage, 
	*oimage;		/* See jove_screen.c */ 
 
extern IOBUF	termout; 
 
extern int 
	CommentColumn,		/* Begin comments in this column */ 
	OKXonXoff,		/* Disable start/stop characters */ 
	MetaKey,		/* This terminal has a meta key */ 
	VisBell,		/* Use visible bell (if possible) */ 
	phystab,		/* Terminal's tabstop settings */  
	tabstop,		/* Expand tabs to this number of spaces */ 
	RMargin,		/* Right margin */ 
	MakeAll,		/* Should we make -k or just make */ 
	ScrollStep,		/* How should we scroll */ 
	WtOnMk,			/* Write files on compile-it command */ 
	UseBufferEdit,		/* Use buffer editor for list-buffers */ 
	UseBuffers,		/* Use a buffer during list-buffers */ 
	EndWNewline,		/* End files with a blank line */ 
	MarkThresh,		/* Moves greater than MarkThresh 
				   will SetMark */ 
	PDelay,			/* Paren flash delay in tenths of a second */ 
	ASInterval,		/* Perform auto-save every ASInterval chars */ 
	CreatMode,		/* Default mode for creat'ing files */ 
	CaseIgnore,		/* Case ignore search */ 
	MarksShouldFloat;	/* Adjust marks on insertion/deletion */ 
 
#ifdef PROCS 
extern int	PMaxLines;	/* Max lines allowed in process buffer */ 
#endif 
 
extern int 
	exp,		/* Argument count */ 
	exp_p;		/* Argument count is supplied */ 
 
extern int 
	io,		/* File descriptor for reading and writing files */ 
	errormsg,	/* Last message was an error message 
			   so don't erase the error before it 
			   has been read. */ 
	this_cmd,	/* ... */ 
	last_cmd,	/* Last command ... to implement appending 
			   to kill buffer */ 
	RecDepth,	/* Recursion depth */ 
	InputPending,	/* Non-zero if there is input waiting to 
			   be processed. */ 
 	killptr,	/* Index into killbuf */ 
	CanScroll,	/* Can this terminal scroll? */ 
	Crashing,	/* We are in the middle of crashing */ 
	Asking;		/* Are we on read a string from the terminal? */ 
 
extern char	Minibuf[]; 
 
extern char	**argvp; 
 
#define curline		curbuf->b_dot 
#define curchar		curbuf->b_char 
#define curmark		curbuf->b_markring[curbuf->b_themark] 
 
extern char	linebuf[]; 
extern Line	*killbuf[];	/* Array of pointers to killed stuff */ 
 
extern char	mesgbuf[]; 
 
struct screenline { 
	char	*s_line, 
		*s_length; 
}; 
 
struct macro { 
	int	Type;		/* In this case a macro */ 
	char	*Name;		/* Name is always second ... */ 
	int	MacLength,	/* Length of macro so we can use ^@ */ 
		MacBuflen,	/* Memory allocated for it */ 
		Offset,		/* Index into body for defining and running */ 
		Flags,		/* Defining/running this macro? */ 
		Ntimes;		/* Number of times to run this macro */ 
	char	*Body;		/* Actual body of the macro */ 
	struct macro 
		*m_nextm; 
}; 
 
struct variable { 
	int	Type;		/* In this case a variable */ 
	char	*Name;		/* Name is always second */ 
	int	*Value; 
	char	*Base; 
}; 
 
struct funct { 
	int	Type; 
	char	*Name; 
	int	(*f_func)(); 
}; 
 
extern int 
	(*Getchar)(); 
 
typedef struct data_obj { 
	int	Type; 
	char	*Name; 
} data_obj;	/* A pointer to funct, macro, or variable */ 
 
extern data_obj 
	*mainmap[],	/* Various key maps.  There is one main */ 
	*pref1map[],	/* map and three prefix maps ... */ 
	*pref2map[], 
	*miscmap[], 
	*LastFunc;	/* Last function invoked */ 
 
extern int 
	LastKeyStruck; 
 
extern struct funct	commands[]; 
extern struct macro	*macros; 
extern struct variable	variables[]; 
 
extern struct macro 
	*macstack[], 
	KeyMacro; 
 
extern int 
	stackp, 
 
	CapLine,	/* Cursor line and cursor column */ 
	CapCol, 
 
	UpdModLine,	/* Whether we want to update the mode line */ 
	UpdMesg;	/* Update the message line */ 
	 
extern long 
	lseek(); 
 
extern disk_line 
	putline(); 
 
extern data_obj 
	*findcom(); 
 
extern Line 
	*next_line(), 
	*prev_line(), 
	*nbufline(), 
	*reg_delete(), 
	*lastline(), 
	*listput(); 
 
extern char 
	*tilde(), 
	*copystr(), 
	*getbexist(),		/* GET Buffer EXISTing */ 
	*FuncName(), 
	*filename(), 
	*getblock(), 
	*sprintf(), 
	*strcpy(), 
	*IOerr(), 
	*bufmod(), 
	*index(), 
	*ask(), 
	*ask_file(), 
	*RunEdit(), 
	*getline(), 
	*getblock(), 
	*malloc(), 
	*emalloc(), 
	*mktemp(), 
	*place(), 
	*realloc(), 
	*getright(), 
	*getcptr(), 
	*rindex(), 
	*getenv(), 
	*tgoto(), 
	*PathRelative(), 
	*sprint(), 
	*StrIndex(); 
 
extern Bufpos 
	*dosearch(), 
	*DoYank(), 
	*m_paren(); 
 
extern Mark 
	*CurMark(), 
	*MakeMark(); 
 
extern Window 
	*windlook(), 
	*windbp(), 
	*div_wind(); 
 
extern data_obj 
	**IsPrefix(); 
 
extern Buffer 
	*do_find(), 
	*do_select(), 
	*mak_buf(), 
	*buf_exists(), 
	*file_exists(); 
 
extern int 
	Cmode(), 
	PascalMode(), 
	EuclidMode(), 
	AsmMode(), 
	FundMode(), 
	ScribeMode(), 
	CluMode(), 
	TexMode(), 
	TextMode(); 
