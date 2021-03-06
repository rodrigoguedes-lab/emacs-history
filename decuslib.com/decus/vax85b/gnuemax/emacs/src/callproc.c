/* Synchronous subprocess invocation for GNU Emacs. 
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
#include <signal.h> 
#include <sys/file.h> 
#include <sys/ioctl.h> 
#include <sgtty.h> 
#include <sys/wait.h> 
#include <sys/time.h> 
#include <sys/resource.h> 
#include "config.h" 
#include "lisp.h" 
#include "commands.h" 
#include "buffer.h" 
#include "paths.h" 
 
#define max(a, b) ((a) > (b) ? (a) : (b)) 
 
Lisp_Object Vexec_path, Vexec_directory; 
 
Lisp_Object Vshell_file_name; 
 
Lisp_Object 
call_process_cleanup (fdpid) 
     Lisp_Object fdpid; 
{ 
  Lisp_Object fd, pid; 
  fd = Fcar (fdpid); 
  pid = Fcdr (fdpid); 
  close (XFASTINT (fd)); 
  kill (XFASTINT (pid), SIGKILL); 
  return Qnil; 
} 
 
DEFUN ("call-process", Fcall_process, Scall_process, 1, MANY, 0, 
  "Call PROGRAM in separate process.\n\ 
Program's input comes from file INFILE (nil means /dev/null).\n\ 
Insert output in BUFFER before dot; t means current buffer;\n\ 
 nil for BUFFER means discard it; 0 means discard and don't wait.\n\ 
Fourth arg DISPLAY non-nil means redisplay buffer as output is inserted.\n\ 
Remaining arguments are strings passed as command arguments to PROGRAM.\n\ 
This function waits for PROGRAM to terminate;\n\ 
if you quit, the process is killed.") 
  (nargs, args) 
     int nargs; 
     Lisp_Object *args; 
{ 
  Lisp_Object infile, display, buffer, tem, tem1; 
  int fd[2]; 
  int filefd; 
  int pid; 
  register int nread; 
  char buf[1024]; 
  int count = specpdl_ptr - specpdl; 
  register int i; 
  register unsigned char **new_argv 
    = (unsigned char **) alloca ((max (2, nargs - 2)) * sizeof (char *)); 
  struct buffer *old = bf_cur; 
 
  tem = args[0]; 
  CHECK_STRING (tem, 0); 
 
  if (nargs <= 1 || NULL (args[1])) 
    args[1] = build_string ("/dev/null"); 
  else 
    tem = args[1], 
    args[1] = Fexpand_file_name (tem, bf_cur->directory); 
  tem = args[1]; 
  CHECK_STRING (tem, 1); 
 
  if (nargs <= 2 || NULL (args[2])) 
    buffer = Qnil; 
  else if (EQ (args[2], Qt)) 
    buffer = Qt; 
  else if (XFASTINT (args[2]) == 0) 
    buffer = args[2]; 
  else 
    { 
      tem = args[2]; 
      buffer = Fget_buffer (tem); 
      CHECK_BUFFER (buffer, 2); 
    } 
 
  display = nargs >= 3 ? args[3] : Qnil; 
 
  for (i = 4; i < nargs; i++) 
    { 
      tem = args[i]; 
      CHECK_STRING (tem, i); 
      new_argv[i - 3] = XSTRING (args[i])->data; 
    } 
  /* Program name is first command arg */ 
  new_argv[0] = XSTRING (args[0])->data; 
  new_argv[i - 3] = 0; 
 
  filefd = open (XSTRING (args[1])->data, O_RDONLY, 0); 
  if (filefd < 0) 
    { 
      tem = args[1]; 
      report_file_error ("Opening process input file", Fcons (tem, Qnil)); 
    } 
  /* If program file name is not absolute, search our path for it */ 
  if (new_argv[0][0] != '/') 
    { 
      tem = Qnil; 
      tem1 = args[0]; 
      close (openp (Vexec_path, tem1, "", &tem, 1)); 
      if (NULL (tem)) 
	report_file_error ("Opening program to exec", Fcons (tem1, Qnil)); 
      new_argv[0] = XSTRING (tem)->data; 
    } 
 
  if (XTYPE (buffer) == Lisp_Int) 
    fd[1] = open ("/dev/null", 0), fd[0] = -1; 
  else 
    { 
      pipe (fd); 
      ioctl (fd[0], FIOCLEX, 0); 
    } 
 
  pid = vfork(); 
  if (!pid) 
    child_setup (filefd, fd[1], fd[1], new_argv); 
 
  close (filefd); 
  close (fd[1]); 
 
  if (pid < 0) 
    { 
      close (fd[0]); 
      report_file_error ("Doing vfork", Qnil); 
    } 
 
  if (XTYPE (buffer) == Lisp_Int) 
    { 
#ifndef subprocesses 
      wait3 (0, WNOHANG | WUNTRACED, 0); 
#endif subprocesses 
      return Qnil; 
    } 
 
  record_unwind_protect (call_process_cleanup, 
			 Fcons (make_number (fd[0]), make_number (pid))); 
 
 
  if (XTYPE (buffer) == Lisp_Buffer) 
    Fset_buffer (buffer); 
 
  immediate_quit = 1; 
  while ((nread = read (fd[0], buf, sizeof buf)) > 0) 
    { 
      immediate_quit = 0; 
      QUIT; 
      if (!NULL (buffer)) 
	InsCStr (buf, nread); 
      if (!NULL (display) && INTERACTIVE) 
	DoDsp (1); 
      immediate_quit = 1; 
    } 
 
  /* Wait for it to terminate, unless it already has.  */ 
  while (1) 
    { 
#ifdef subprocesses 
      sigsetmask (1<<SIGCHLD); 
      if (0 > kill (pid, 0)) 
	{ 
	  sigsetmask (0); 
	  break; 
	} 
      sigpause (0); 
#else 
      if (0 > kill (pid, 0)) 
	break; 
      wait (); 
#endif 
    } 
 
  immediate_quit = 0; 
  QUIT; 
 
  SetBfp (old); 
 
  unbind_to (count); 
 
  return Qnil; 
} 
 
child_setup (in, out, err, new_argv) 
     int in, out, err; 
     char **new_argv; 
{ 
  int pid = getpid(); 
  register int i; 
  unsigned char *temp; 
  struct sgttyb s; 
 
  ioctl (out, TIOCGETP, &s); 
  s.sg_flags &= ~(ECHO | CRMOD | ANYP | ALLDELAY | RAW | LCASE | CBREAK | TANDEM); 
  ioctl (out, TIOCSETN, &s); 
 
  setpriority(PRIO_PROCESS, getpid(), 0); 
 
  if (XTYPE (bf_cur->directory) == Lisp_String) 
    { 
      temp = (unsigned char *) alloca (XSTRING (bf_cur->directory)->size + 2); 
      bcopy (XSTRING (bf_cur->directory)->data, temp, 
	     XSTRING (bf_cur->directory)->size); 
      i = XSTRING (bf_cur->directory)->size; 
      if (temp[i - 1] != '/') temp[i++] = '/'; 
      temp[i] = 0; 
      chdir (temp); 
    } 
 
  close (0); 
  close (1); 
  close (2); 
 
  dup2 (in, 0); 
  dup2 (out, 1); 
  dup2 (err, 2); 
 
  setpgrp (pid, pid); 
  ioctl (0, TIOCSPGRP, &pid); 
 
  for (i = 0; i < NSIG; i++) 
    signal (i, SIG_DFL); 
 
#ifdef vipc 
  something missing here; 
#endif vipc 
 
  execvp (new_argv[0], new_argv); 
  write (1, "Couldn't exec the program ", 26); 
  write (1, new_argv[0], strlen (new_argv[0])); 
  _exit (1); 
} 
 
DEFUN ("call-process-region", Fcall_process_region, Scall_process_region, 
  3, MANY, 0, 
  "Send text from START to END to a process running PROGRAM.\n\ 
Delete it if DELETE is non-nil.\n\ 
Put output in BUFFER, before dot.  nil => discard it, t => current buffer.\n\ 
Sixth arg DISPLAY non-nil means redisplay buffer as output is inserted.\n\ 
Remaining args are passed to PROGRAM at startup as command args.\n\ 
This function normally waits for the process to terminate;\n\ 
if you quit, the process is killed.") 
  (nargs, args) 
     int nargs; 
     Lisp_Object *args; 
{ 
  Lisp_Object filename_string, start, end; 
  char tempfile[20]; 
 
  strcpy (tempfile, "/tmp/emacsXXXXXX"); 
  mktemp (tempfile); 
 
  filename_string = build_string (tempfile); 
  start = args[0]; 
  end = args[1]; 
  Fwrite_region (start, end, filename_string, Qnil, Qlambda); 
 
  if (!NULL (args[3])) 
    Fdelete_region (start, end); 
 
  args[3] = filename_string; 
  Fcall_process (nargs - 2, args + 2); 
  unlink (tempfile); 
  return Qnil; 
} 
 
init_callproc () 
{ 
  char *sh; 
 
  Vexec_path = nconc2 (decode_env_path ("PATH", ""), 
		       decode_env_path ("%$&*#", PATH_EXEC)); 
  Vexec_directory = build_string (PATH_EXEC); 
 
  sh = (char *) getenv ("SHELL"); 
  Vshell_file_name = build_string (sh ? sh : "/bin/sh"); 
} 
 
syms_of_callproc () 
{ 
  DefLispVar ("shell-file-name", &Vshell_file_name, 
    "*File name to load inferior shells from.\n\ 
Initialized from the SHELL environment variable."); 
 
  DefLispVar ("exec-path", &Vexec_path, 
    "*List of directories to search programs to run in subprocesses.\n\ 
Each element is a string (directory name) or nil (try default directory)."); 
 
  DefLispVar ("exec-directory", &Vexec_directory, 
    "Directory that holds programs that come with GNU Emacs,\n\ 
intended for Emacs to invoke."); 
 
  defsubr (&Scall_process); 
  defsubr (&Scall_process_region); 
} 
