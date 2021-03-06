#include <sys/types.h>I
#include <sys/stat.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include "vipc.h" 
#include <errno.h> 
extern errno; 
 
extern char *getenv(); 
 
static int	unique_frob= UNIQUE_FROB; 
static int	type, length; 
 
/* 
 * macro which formats messages for sending to VIEW 
 */ 
#define VIEW_WRITE(s, t, l, msg) \ 
	type = t; length = l; \ 
	if (write (s, &unique_frob, CBUNIQUE) != CBUNIQUE) return -1; \ 
	if (write (s, &type, CBTYPE) != CBTYPE) return -1; \ 
	if (write (s, &length, CBLENGTH) != CBLENGTH) return -1; \ 
	if (length > 0 && (write (s, msg, length) != length)) return -1; 
 
/* 
 * create a (virtual circuit) channel for communication 
 * sbuf is the name of the buffer to connect to, or, if NULL, use IPCBUFFER 
 * view_connect returns a file descriptor (socket) for I/O 
 */ 
view_connect( sbuf ) 
char *sbuf; 
{ 
    char *vipcid; 
    int s; 
    struct sockaddr_in sin; 
    struct hostent *hp; 
    char myhostname[32]; 
 
    gethostname(myhostname, sizeof(myhostname)); 
    hp = gethostbyname(myhostname); 
    if ( hp == (struct hostent *) 0) 
	return -1; 
    bzero((char *)&sin, sizeof(sin)); 
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length); 
    sin.sin_family = AF_INET; 
 
    if (( vipcid = getenv( "VIPCID" )) == (char *) 0 ) { 
	errno = EDESTADDRREQ; 
	return -1; 
    } 
    if ((s = socket( AF_INET, SOCK_STREAM, 0, 0)) < 0 ) 
	return -1;		/* errno will be set */ 
 
    sin.sin_port = atoi( vipcid ); 
 
    if ( connect( s, &sin, sizeof sin ) < 0 ) 
	return -1;		/* may need timeout */ 
 
    view_sendsigs( s ); 
    view_namebuf( s, sbuf ); 
 
    return s; 
} 
 
 
/* 
 * set the buffer which is associated with the command channel 
 */ 
view_namebuf( s, bufname ) 
int s; 
char *bufname; 
{ 
    VIEW_WRITE( s, VIPC_NAMEBUF, strlen( bufname ), bufname ); 
    return 0; 
} 
 
/* 
 * set the pid of the process which is to receive signals from VIEW 
 */ 
view_sendsigs( s ) 
int s; 
{ 
    int pid = getpid(); 
    VIEW_WRITE( s, VIPC_SENDSIGS, sizeof pid, &pid ); 
    return 0; 
} 
 
/* 
 * execute the passed code (mlisp) as an mlisp prog node 
 */ 
view_mlisp( s, mlisp ) 
int s; 
char *mlisp; 
{ 
    VIEW_WRITE( s, VIPC_MLISP, strlen( mlisp ) + 1, mlisp ); 
    return 0; 
} 
 
/* 
 * write a string (str) into the attached buffer 
 */ 
view_tobuf( s, str ) 
int s; 
char *str; 
{ 
    VIEW_WRITE( s, VIPC_TOBUF, strlen( str ), str ); 
    return 0; 
} 
 
/* 
 * make a "hard" connection to the currently attached buffer 
 * this command cannot be undone, and all messages sent hereafter will 
 *	simply be stuffed into the buffer (with no header checking, etc.) 
 * in other words, simply use write(2) for future transmissions 
 */ 
view_iochan( s ) 
int s; 
{ 
    VIEW_WRITE( s, VIPC_IOCHAN, 0, "" ); 
    return 0; 
} 
 
/* 
 * inform VIEW that the channel is no longer needed (i.e., performs cleanup) 
 */ 
view_shutdown( s ) 
int s; 
{ 
    VIEW_WRITE( s, VIPC_SHUTDOWN, 0, "" ); 
    return 0; 
} 
 
 
/* 
 * set a variable to a (decimal) value 
 */ 
view_setvar( s, str, val ) 
char *str; 
int s, val; 
{ 
    char mesg[IPCBUFSIZ]; 
    sprintf( mesg, "(setq %s %d)", str, val ); 
    return view_mlisp( s, mesg ); 
} 
 
/* 
 * set a variable to a (string) value 
 */ 
view_setstr( s, str, t ) 
int s; 
char *str, *t; 
{ 
    char mesg[IPCBUFSIZ]; 
    sprintf( mesg, "(setq %s \"%s\")", str, t ); 
    return view_mlisp( s, mesg ); 
} 
 
/* 
 * send a message <msg> to the notifier from <who> with command <cmd> 
 */ 
view_notify( s, msg, cmd, who ) 
int s; 
char *msg, *cmd, *who; 
{ 
    char outbuf[IPCBUFSIZ]; 
 
    sprintf( outbuf, "(notify \"%s\" \"%s\" \"%s\")", msg, cmd, who ); 
    return view_mlisp( s, outbuf ); 
} 
