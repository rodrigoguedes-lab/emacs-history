/* Minibuffer input and completion. 
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
#include "config.h" 
#include "lisp.h" 
#include "commands.h" 
#include "buffer.h" 
#include "window.h" 
#include "syntax.h" 
#include "disp_extern.h" 
 
#define min(a, b) ((a) < (b) ? (a) : (b)) 
 
/* List of buffers for use as minibuffers. 
  The first element of the list is used for the outermost minibuffer invocation, 
  the next element is used for a recursive minibuffer invocation, etc. 
  The list is extended at the end as deeped minibuffer recursions are encountered. */ 
Lisp_Object Vminibuffer_list; 
 
struct minibuf_save_data 
  { 
    struct buffer *buffer; 
    Lisp_Object window; 
    char *prompt; 
    Lisp_Object hscroll; 
  }; 
 
int minibuf_save_vector_size; 
struct minibuf_save_data *minibuf_save_vector; 
 
int auto_help;		/* Nonzero means display completion help for invalid input */ 
int remove_help_window; /* Nonzero means remove completion help from display 
			   when the minibuffer is exited. */ 
 
 
/* Fread_minibuffer leaves the input, as a string, here */ 
Lisp_Object last_minibuf_string; 
 
/* Actual minibuffer invocation. */ 
 
void read_minibuf_string_unwind (); 
Lisp_Object get_minibuffer (); 
Lisp_Object read_minibuf (); 
 
Lisp_Object 
read_minibuf_string (map, prefix, prompt) 
     Lisp_Object map; 
     Lisp_Object prefix; 
     Lisp_Object prompt; 
{ 
  return read_minibuf (map, prefix, prompt, 0); 
} 
 
 
Lisp_Object 
read_minibuf (map, prefix, prompt, expflag) 
     Lisp_Object map; 
     Lisp_Object prefix; 
     Lisp_Object prompt; 
     int expflag; 
{ 
  char lbuf[200]; 
  Lisp_Object val; 
  int count = specpdl_ptr - specpdl; 
  int abortflag; 
 
  if (MinibufDepth == minibuf_save_vector_size) 
    minibuf_save_vector = 
     (struct minibuf_save_data *) realloc (minibuf_save_vector, 
		      (minibuf_save_vector_size *= 2) * sizeof (struct minibuf_save_data));  
  minibuf_save_vector[MinibufDepth].prompt = minibuf_prompt; 
 
  record_unwind_protect (save_window_restore, save_window_save ()); 
 
  val = bf_cur->directory; 
  Fset_buffer (get_minibuffer (MinibufDepth++)); 
  bf_cur->directory = val; 
 
  Fshow_buffer (minibuf_window, Fcurrent_buffer ()); 
  Fselect_window (minibuf_window); 
  XFASTINT (XWINDOW (minibuf_window)->hscroll) = 0; 
 
  Fwiden (); 
  Ferase_buffer (); 
 
  record_unwind_protect (read_minibuf_string_unwind, Qnil); 
 
  if (!NULL (prefix)) 
    Finsert (1, &prefix); 
 
  strncpy (lbuf, XSTRING (prompt)->data, sizeof lbuf - 1); 
  lbuf[sizeof lbuf - 1] = 0; 
  minibuf_prompt = lbuf; 
  minibuf_message = 0; 
 
  bf_cur->keymap = map; 
  Frecursive_edit (); 
 
  /* If cursor is on the minibuffer line, 
     show the user we have exited by putting it in column 0.  */ 
  if (cursY - 1 >= XFASTINT (XWINDOW (minibuf_window)->top)) 
    { 
      cursX = 1; 
      UpdateScreen (1, 1); 
    } 
 
  /* Make minibuffer contents into a string */ 
  val = make_string (&CharAt (1), bf_s1 + bf_s2); 
  bcopy (bf_p2 + bf_s1 + 1, 
	 XSTRING (val)->data + bf_s1, 
	 bf_s2); 
 
  last_minibuf_string = val; 
 
  /* If Lisp form desired instead of string, read buffer contents */ 
  if (expflag) 
    { 
      SetDot (1); 
      val = Fread (Fcurrent_buffer ()); 
    } 
 
  unbind_to (count); 
  return val; 
} 
 
/* Return a buffer to be used as the minibuffer at depth `depth'. 
 depth = 0 is the lowest allowed argument, and that is the value 
 used for nonrecursive minibuffer invocations */ 
 
Lisp_Object 
get_minibuffer (depth) 
     int depth; 
{ 
  Lisp_Object tail, num, buf; 
  char name[14]; 
  extern Lisp_Object nconc2 (); 
 
  XFASTINT (num) = depth; 
  tail = Fnthcdr (num, Vminibuffer_list); 
  if (NULL (tail)) 
    { 
      tail = Fcons (Qnil, Qnil); 
      Vminibuffer_list = nconc2 (Vminibuffer_list, tail); 
    } 
  buf = Fcar (tail); 
  if (NULL (buf) || NULL (XBUFFER (buf)->name)) 
    { 
      sprintf (name, " *Minibuf-%d*", depth); 
      buf = Fget_buffer_create (build_string (name)); 
      XCONS (tail)->car = buf; 
    } 
 
  return buf; 
} 
 
/* This function is called on exiting minibuffer, whether normally or not, 
 and it restores the current window, buffer, etc. */ 
 
void 
read_minibuf_string_unwind () 
{ 
  Ferase_buffer (); 
 
  /* If this was a recursive minibuffer, 
     tie the minibuffer window back to the outer level minibuffer buffer */ 
  MinibufDepth--; 
  /* Make sure minibuffer window is erased, not ignored */ 
  windows_or_buffers_changed++; 
  XFASTINT (XWINDOW (minibuf_window)->last_modified) = 0; 
 
  /* Restore prompt from outer minibuffer */ 
  minibuf_prompt = minibuf_save_vector[MinibufDepth].prompt; 
} 
 
DEFUN ("eval-minibuffer", Feval_minibuffer, Seval_minibuffer, 1, 1, 0, 
  "Return value of Lisp expression read using the minibuffer.\n\ 
Return Prompt with PROMPT.") 
  (prompt) 
     Lisp_Object prompt; 
{ 
  return Feval (Fread_minibuffer (prompt)); 
} 
 
DEFUN ("read-minibuffer", Fread_minibuffer, Sread_minibuffer, 1, 1, 0, 
  "Return Lisp object read using the minibuffer, prompting with PROMPT.") 
  (prompt) 
     Lisp_Object prompt; 
{ 
  return read_minibuf (MinibufLocalMap, Qnil, prompt, 1); 
} 
 
/* Functions that use the minibuffer to read various things. */ 
 
DEFUN ("read-string", Fread_string, Sread_string, 1, 1, 0, 
  "Read a string from the minibuffer, prompting with string PROMPT.") 
  (prompt) 
     Lisp_Object prompt; 
{ 
  CHECK_STRING (prompt, 0); 
  return read_minibuf_string (MinibufLocalMap, Qnil, prompt); 
} 
 
DEFUN ("read-input", Fread_input, Sread_input, 2, 2, 0, 
  "Read string from minibuffer, prompting with string PROMPT.\n\ 
Start off with second arg string INIT as minibuffer contents.") 
  (prompt, init) 
     Lisp_Object prompt, init; 
{ 
  CHECK_STRING (prompt, 0); 
  CHECK_STRING (init, 1); 
 
  return read_minibuf_string (MinibufLocalMap, init, prompt); 
} 
 
DEFUN ("read-no-blanks-input", Fread_no_blanks_input, Sread_no_blanks_input, 2, 2, 0, 
  "Args PROMPT and INIT, strings.  Read a string from the terminal, not allowing blanks.\n\ 
Prompt with PROMPT, and provide INIT as an initial value of the input string.") 
  (prompt, init) 
     Lisp_Object prompt, init; 
{ 
  CHECK_STRING (prompt, 0); 
  CHECK_STRING (init, 1); 
 
  return read_minibuf_string (MinibufLocalNSMap, init, prompt); 
} 
 
DEFUN ("read-command", Fread_command, Sread_command, 1, 1, 0, 
  "One arg PROMPT, a string.  Read the name of a command and return as a symbol.\n\ 
Prompts with PROMPT.") 
  (prompt) 
     Lisp_Object prompt; 
{ 
  return Fintern (Fcompleting_read (prompt, Vobarray, Qcommandp, Qt, Qnil), 
		  Qnil); 
} 
 
DEFUN ("read-function", Fread_function, Sread_function, 1, 1, 0, 
  "One arg PROMPT, a string.  Read the name of a function and return as a symbol.\n\ 
Prompts with PROMPT.") 
  (prompt) 
     Lisp_Object prompt; 
{ 
  return Fintern (Fcompleting_read (prompt, Vobarray, Qfboundp, Qt, Qnil), 
		  Qnil); 
} 
 
DEFUN ("read-variable", Fread_variable, Sread_variable, 1, 1, 0, 
  "One arg PROMPT, a string.  Read the name of a variable and return as a symbol.\n\ 
Prompts with PROMPT.") 
  (prompt) 
     Lisp_Object prompt; 
{ 
  return Fintern (Fcompleting_read (prompt, Vobarray, Qboundp, Qt, Qnil), 
		  Qnil); 
} 
 
DEFUN ("read-buffer", Fread_buffer, Sread_buffer, 1, 3, 0, 
  "One arg PROMPT, a string.  Read the name of a buffer and return as a string.\n\ 
Prompts with PROMPT.\n\ 
Optional second arg is value to return if user enters an empty line.\n\ 
If optional third arg REQUIRE-MATCH is non-nil, only existing buffer names are allowed.") 
  (prompt, def, require_match) 
     Lisp_Object prompt, def, require_match; 
{ 
  Lisp_Object tem; 
  Lisp_Object args[3]; 
  struct gcpro gcpro1; 
 
  if (XTYPE (def) == Lisp_Buffer) 
    def = XBUFFER (def)->name; 
  if (!NULL (def)) 
    { 
      args[0] = build_string ("%s(default %s) "); 
      args[1] = prompt; 
      args[2] = def; 
      prompt = Fformat (3, args); 
    } 
  GCPRO1 (def); 
  tem = Fcompleting_read (prompt, Vbuffer_alist, Qnil, require_match, Qnil); 
  UNGCPRO; 
  if (XSTRING (tem)->size) 
    return tem; 
  return def; 
} 
 
DEFUN ("try-completion", Ftry_completion, Stry_completion, 2, 3, 0, 
  "Return common substring of all completions of STRING in ALIST.\n\ 
Each car of each element of ALIST is tested to see if it begins with STRING.\n\ 
All that match are compared together; the longest initial sequence\n\ 
common to all matches is returned as a string.\n\ 
If there is no match at all, nil is returned.\n\ 
For an exact match, t is returned.\n\ 
\n\ 
ALIST can be an obarray instead of an alist.\n\ 
Then the print names of all symbols in the obarray are the possible matches.\n\ 
\n\ 
If optional third argument PREDICATE is non-nil,\n\ 
it is used to test each possible match.\n\ 
The match is a candidate only if PREDICATE returns non-nil.\n\ 
The argument given to PREDICATE is the alist element or the symbol from the obarray.") 
  (string, alist, pred) 
     Lisp_Object string, alist, pred; 
{ 
  Lisp_Object bestmatch, tail, elt, eltstring; 
  int bestmatchsize; 
  int compare, matchsize; 
  int list = LISTP (alist); 
  int index, obsize; 
  int matchcount = 0; 
  Lisp_Object bucket, zero, end, tem; 
  unsigned char *p1, *p2; 
 
  CHECK_STRING (string, 0); 
  if (!list && XTYPE (alist) != Lisp_Vector) 
    return call3 (alist, string, pred, Qnil); 
 
  bestmatch = Qnil; 
 
  if (list) 
    tail = alist; 
  else 
    { 
      index = 0; 
      obsize = XVECTOR (alist)->size; 
      bucket = XVECTOR (alist)->contents[index]; 
    } 
 
  while (1) 
    { 
      /* Get the next element of the alist or obarray. */ 
      /* Exit the loop if the elements are all used up. */ 
      /* elt gets the alist element or symbol. 
	 eltstring gets the name to check as a completion. */ 
 
      if (list) 
	{ 
	  if (NULL (tail)) 
	    break; 
	  elt = Fcar (tail); 
	  eltstring = Fcar (elt); 
	  tail = Fcdr (tail); 
	} 
      else 
	{ 
	  if (XSYMBOL (bucket)) 
	    { 
	      elt = bucket; 
	      eltstring = Fsymbol_name (elt); 
	      XSETSYMBOL (bucket, XSYMBOL (bucket)->next); 
	    } 
	  else if (++index >= obsize) 
	    break; 
	  else 
	    { 
	      bucket = XVECTOR (alist)->contents[index]; 
	      continue; 
	    } 
	} 
 
      /* Is this element a possible completion? */ 
 
      if (XTYPE (eltstring) == Lisp_String && 
	  XSTRING (string)->size <= XSTRING (eltstring)->size && 
	  !strncmp (XSTRING (eltstring)->data, XSTRING (string)->data, XSTRING (string)->size)) 
	{ 
	  /* Yes. */ 
	  /* Ignore this element if there is a predicate and the predicate doesn't like it. */ 
 
	  if (!NULL (pred)) 
	    { 
	      if (EQ (pred, Qcommandp)) 
		tem = Fcommandp (elt); 
	      else 
		{ 
		  tem = call1 (pred, elt); 
		} 
	      if (NULL (tem)) continue; 
	    } 
 
	  /* Update computation of how much all possible completions match */ 
 
	  matchcount++; 
	  if (NULL (bestmatch)) 
	    bestmatch = eltstring, bestmatchsize = XSTRING (eltstring)->size; 
	  else 
	    { 
	      compare = min (bestmatchsize, XSTRING (eltstring)->size); 
	      p1 = XSTRING (bestmatch)->data; 
	      p2 = XSTRING (eltstring)->data; 
	      for (matchsize = 0; matchsize < compare; matchsize++) 
		if (p1[matchsize] != p2[matchsize]) break; 
	      bestmatchsize = min (matchsize, bestmatchsize); 
	    } 
	} 
    } 
 
  if (NULL (bestmatch)) 
    return Qnil;		/* No completions found */ 
  if (matchcount == 1 && bestmatchsize == XSTRING (string)->size) 
    return Qt; 
 
  XFASTINT (zero) = 0;		/* Else extract the part in which */ 
  XFASTINT (end) = bestmatchsize;	     /* all completions agree */ 
  return Fsubstring (bestmatch, zero, end); 
} 
 
DEFUN ("all-completions", Fall_completions, Sall_completions, 2, 3, 0, 
  "Search for partial matches to STRING in ALIST.\n\ 
Each car of each element of ALIST is tested to see if it begins with STRING.\n\ 
The value is a list of all the strings from ALIST that match.\n\ 
ALIST can be an obarray instead of an alist.\n\ 
Then the print names of all symbols in the obarray are the possible matches.\n\ 
\n\ 
If optional third argument PREDICATE is non-nil,\n\ 
it is used to test each possible match.\n\ 
The match is a candidate only if PREDICATE returns non-nil.\n\ 
The argument given to PREDICATE is the alist element or the symbol from the obarray.") 
  (string, alist, pred) 
     Lisp_Object string, alist, pred; 
{ 
  Lisp_Object tail, elt, eltstring; 
  Lisp_Object allmatches; 
  int list = LISTP (alist); 
  int index, obsize; 
  Lisp_Object bucket, tem; 
 
  CHECK_STRING (string, 0); 
  if (!list && XTYPE (alist) != Lisp_Vector) 
    { 
      return call3 (alist, string, pred, Qt); 
    } 
  allmatches = Qnil; 
 
  if (list) 
    tail = alist; 
  else 
    { 
      index = 0; 
      obsize = XVECTOR (alist)->size; 
      bucket = XVECTOR (alist)->contents[index]; 
    } 
 
  while (1) 
    { 
      /* Get the next element of the alist or obarray. */ 
      /* Exit the loop if the elements are all used up. */ 
      /* elt gets the alist element or symbol. 
	 eltstring gets the name to check as a completion. */ 
 
      if (list) 
	{ 
	  if (NULL (tail)) 
	    break; 
	  elt = Fcar (tail); 
	  eltstring = Fcar (elt); 
	  tail = Fcdr (tail); 
	} 
      else 
	{ 
	  if (XSYMBOL (bucket)) 
	    { 
	      elt = bucket; 
	      eltstring = Fsymbol_name (elt); 
	      XSETSYMBOL (bucket, XSYMBOL (bucket)->next); 
	    } 
	  else if (++index >= obsize) 
	    break; 
	  else 
	    { 
	      bucket = XVECTOR (alist)->contents[index]; 
	      continue; 
	    } 
	} 
 
      /* Is this element a possible completion? */ 
 
      if (XTYPE (eltstring) == Lisp_String && 
	  XSTRING (string)->size <= XSTRING (eltstring)->size && 
	  XSTRING (eltstring)->data[0] != ' ' && 
	  !strncmp (XSTRING (eltstring)->data, XSTRING (string)->data, XSTRING (string)->size)) 
	{ 
	  /* Yes. */ 
	  /* Ignore this element if there is a predicate and the predicate doesn't like it. */ 
 
	  if (!NULL (pred)) 
	    { 
	      if (EQ (pred, Qcommandp)) 
		tem = Fcommandp (elt); 
	      else 
		tem = call1 (pred, elt); 
	      if (NULL (tem)) continue; 
	    } 
	  /* Ok => put it on the list. */ 
	  allmatches = Fcons (eltstring, allmatches); 
	} 
    } 
 
  return Fnreverse (allmatches); 
} 
 
Lisp_Object Vminibuffer_completion_table, Qminibuffer_completion_table; 
Lisp_Object Vminibuffer_completion_predicate, Qminibuffer_completion_predicate; 
Lisp_Object Vminibuffer_completion_confirm, Qminibuffer_completion_confirm; 
 
DEFUN ("completing-read", Fcompleting_read, Scompleting_read, 2, 5, 0, 
  "Read a string in the minibuffer, with completion.\n\ 
Args are PROMPT, TABLE, PREDICATE, REQUIRE-MATCH and INITIAL-INPUT.\n\ 
PROMPT is a string to prompt with; normally it ends in a colon and a space.\n\ 
TABLE is an alist whose elements' cars are strings, or an obarray (see try-completion).\n\ 
PREDICATE limits completion to a subset of TABLE; see try-completion for details.\n\ 
If REQUIRE-MATCH is non-nil, the user is not allowed to exit unless\n\ 
 the input is (or completes to) an element of TABLE.\n\ 
 If it is also not t, ^M does not exit if it does non-null completion.\n\ 
If INITIAL-INPUT is non-nil, insert it in the minibuffer initially.") 
  (prompt, table, pred, require_match, init) 
     Lisp_Object prompt, table, pred, require_match, init; 
{ 
  Lisp_Object val; 
  int count = specpdl_ptr - specpdl; 
  specbind (Qminibuffer_completion_table, table); 
  specbind (Qminibuffer_completion_predicate, pred); 
  specbind (Qminibuffer_completion_confirm, 
	    EQ (require_match, Qt) ? Qnil : Qt); 
  val = read_minibuf_string (NULL (require_match) ? MinibufLocalCompletionMap 
						  : MinibufLocalMustMatchMap, 
			     init, prompt); 
  unbind_to (count); 
  return val; 
} 
 
Lisp_Object 
temp_minibuf_message_unwind () 
{ 
  del_range (dot, NumCharacters + 1); 
  return Qnil; 
} 
 
temp_minibuf_message (m) 
     char *m; 
{ 
  int osize = dot; 
  int count = specpdl_ptr - specpdl; 
  record_unwind_protect (temp_minibuf_message_unwind, Qnil); 
 
  InsStr (m); 
  SetDot (osize); 
  Fsit_for (make_number (2)); 
  unbind_to (count); 
} 
 
Lisp_Object Fminibuffer_completion_help (); 
 
DEFUN ("minibuffer-complete", Fminibuffer_complete, Sminibuffer_complete, 0, 0, "", 
  "Complete the minibuffer contents as far as possible.") 
  () 
{ 
  Lisp_Object completion, beg; 
  int osize; 
 
  completion = Ftry_completion (Fbuffer_string (), Vminibuffer_completion_table, 
				Vminibuffer_completion_predicate); 
  if (NULL (completion)) 
    { 
      Ding (); 
      temp_minibuf_message (" [No match]"); 
      return Qnil; 
    } 
  if (EQ (completion, Qt))	/* Exact, unique match */ 
    return Qt; 
 
  SetDot (NumCharacters + 1); 
  if (XSTRING (completion)->size > NumCharacters) 
    { 
      XFASTINT (beg) = NumCharacters; 
      completion = Fsubstring (completion, beg, Qnil); 
      Finsert (1, &completion); 
    } 
  else if (auto_help) 
    Fminibuffer_completion_help (); 
  else 
    temp_minibuf_message (" [Next char not unique]"); 
 
  return Qt; 
} 
 
DEFUN ("minibuffer-complete-word", Fminibuffer_complete_word, Sminibuffer_complete_word, 
  0, 0, "", 
  "Complete the minibuffer contents at most a single word.") 
  () 
{ 
  Lisp_Object completion, beg, end, tem; 
  int i; 
 
  completion = Ftry_completion (Fbuffer_string (), Vminibuffer_completion_table, 
				Vminibuffer_completion_predicate); 
  if (NULL (completion)) 
    { 
      Ding (); 
      temp_minibuf_message (" [No match]"); 
      return Qnil; 
    } 
  if (EQ (completion, Qt)) 
    return Qnil; 
 
  i = NumCharacters; 
 
  if (i == XSTRING (completion)->size) 
    { 
      tem = Ftry_completion (concat2 (Fbuffer_string (), build_string ("-")), 
			     Vminibuffer_completion_table, 
			     Vminibuffer_completion_predicate); 
      if (XTYPE (tem) == Lisp_String) 
	completion = tem; 
    }       
 
  for (; i < XSTRING (completion)->size; i++) 
    if (SYNTAX (XSTRING (completion)->data[i]) != Sword) break; 
  if (i < XSTRING (completion)->size) 
    i = i + 1; 
 
  if (i == NumCharacters) 
    { 
      if (auto_help) 
	Fminibuffer_completion_help (); 
      return Qnil; 
    } 
  XFASTINT (beg) = NumCharacters; 
  XFASTINT (end) = i; 
  completion = Fsubstring (completion, beg, end); 
  SetDot (NumCharacters + 1); 
  Finsert (1, &completion); 
  return Qt; 
} 
 
DEFUN ("minibuffer-complete-and-exit", Fminibuffer_complete_and_exit, 
        Sminibuffer_complete_and_exit, 0, 0, "", 
  "Complete the minibuffer contents, and maybe exit.\n\ 
Exit if the name is valid with no completion needed.\n\ 
If name was completed to a valid match,\n\ 
a repetition of this command will exit.") 
  () 
{ 
  Lisp_Object completion, beg; 
  Lisp_Object tem; 
  int len = NumCharacters; 
 
  /* Allow user to specify null string */ 
  if (NumCharacters == 0) 
    Fthrow (Qexit, Qnil); 
 
  completion = Ftry_completion (Fbuffer_string (), Vminibuffer_completion_table, 
				Vminibuffer_completion_predicate); 
  if (NULL (completion)) 
    { 
      Ding (); 
      temp_minibuf_message (" [No match]"); 
      return Qnil; 
    } 
  if (EQ (completion, Qt)) 
    Fthrow (Qexit, Qnil); 
 
  SetDot (NumCharacters + 1); 
  if (XSTRING (completion)->size > NumCharacters) 
    { 
      XFASTINT (beg) = NumCharacters; 
      completion = Fsubstring (completion, beg, Qnil); 
      Finsert (1, &completion); 
    } 
 
  /* It did find a match.  Do we match some possibility exactly now? */ 
  if (LISTP (Vminibuffer_completion_table)) 
    tem = Fassoc (Fbuffer_string (), Vminibuffer_completion_table); 
  else if (XTYPE (Vminibuffer_completion_table) == Lisp_Vector) 
    tem = Fintern_soft (Fbuffer_string (), Vminibuffer_completion_table); 
  else 
    tem = call3 (Vminibuffer_completion_table, 
		 Fbuffer_string (), 
		 Vminibuffer_completion_predicate, 
		 Qlambda); 
 
  if (NULL (tem)) 
    { 
      if (auto_help) 
	Fminibuffer_completion_help (); 
      return Qnil; 
    } 
  /* We have an exact match now. */ 
  /* If nontrivial completion was done, maybe avoid exiting minibuffer */ 
  if (len < NumCharacters && !NULL (Vminibuffer_completion_confirm)) 
    return Qnil; 
 
  Fthrow (Qexit, Qnil); 
} 
 
minibuffer_completion_help_1 (completions) 
     Lisp_Object completions; 
{ 
  Lisp_Object tail; 
  int i; 
  struct buffer *old = bf_cur; 
  SetBfp (XBUFFER (Vstandard_output)); 
 
  InsStr ("Possible completions are:"); 
 
  for (tail = completions, i = 0; !NULL (tail); tail = Fcdr (tail), i++) 
    { 
      if (i & 1) 
	ToCol (35); 
      else 
        Fterpri (Qnil); 
      Fprinc (Fcar (tail), Qnil); 
    } 
  SetBfp (old); 
} 
 
DEFUN ("minibuffer-completion-help", Fminibuffer_completion_help, Sminibuffer_completion_help, 
  0, 0, "", 
  "Display a list of possible completions of the current minibuffer contents.") 
  () 
{ 
  Lisp_Object completions; 
  message ("Making completion list..."); 
  completions = Fall_completions (Fbuffer_string (), Vminibuffer_completion_table, 
				  Vminibuffer_completion_predicate); 
  if (NULL (completions)) 
    Ding (); 
  else 
    internal_with_output_to_temp_buffer (" *Completions*", 
					 minibuffer_completion_help_1, 
					 Fsort (completions, Qstring_lessp)); 
  minibuf_message = 0; 
  return Qnil; 
} 
 
DEFUN ("self-insert-and-exit", Fself_insert_and_exit, Sself_insert_and_exit, 0, 0, "", 
  "Terminate minibuffer input.") 
  () 
{ 
  SelfInsert (LastKeyStruck); 
  Fthrow (Qexit, Qnil); 
} 
 
DEFUN ("exit-minibuffer", Fexit_minibuffer, Sexit_minibuffer, 0, 0, "", 
  "Terminate this minibuffer argument.") 
  () 
{ 
  Fthrow (Qexit, Qnil); 
} 
 
init_minibuf_once () 
{ 
  Vminibuffer_list = Qnil; 
  staticpro (&Vminibuffer_list); 
} 
 
syms_of_minibuf () 
{ 
  MinibufDepth = 0; 
  minibuf_prompt = 0; 
  minibuf_save_vector_size = 5; 
  minibuf_save_vector = (struct minibuf_save_data *) malloc (5 * sizeof (struct minibuf_save_data)); 
 
  Qminibuffer_completion_table = intern ("minibuffer-completion-table"); 
  staticpro (&Qminibuffer_completion_table); 
 
  Qminibuffer_completion_confirm = intern ("minibuffer-completion-confirm"); 
  staticpro (&Qminibuffer_completion_confirm); 
 
  Qminibuffer_completion_predicate = intern ("minibuffer-completion-predicate"); 
  staticpro (&Qminibuffer_completion_predicate); 
 
  staticpro (&last_minibuf_string); 
  last_minibuf_string = Qnil; 
 
  DefBoolVar ("completion-auto-help", &auto_help, 
    "*Non-nil means automatically provide help for invalid completion input."); 
  auto_help = 1; 
 
  DefBoolVar ("remove-help-window", &remove_help_window, 
    "*Non-nil means remove list-of-completions from display when exiting minibuffer."); 
  remove_help_window = 1; 
 
  DefLispVar ("minibuffer-completion-table", &Vminibuffer_completion_table, 
    "Alist or obarray used for completion in the minibuffer."); 
  Vminibuffer_completion_table = Qnil; 
 
  DefLispVar ("minibuffer-completion-predicate", &Vminibuffer_completion_predicate, 
    "Holds PREDICATE argument to completing-read."); 
  Vminibuffer_completion_predicate = Qnil; 
 
  DefLispVar ("minibuffer-completion-confirm", &Vminibuffer_completion_confirm, 
    "Non-nil => demand confirmation of completion before exiting minibuffer."); 
  Vminibuffer_completion_confirm = Qnil; 
 
  defsubr (&Seval_minibuffer); 
  defsubr (&Sread_minibuffer); 
  defsubr (&Sread_string); 
  defsubr (&Sread_command); 
  defsubr (&Sread_variable); 
  defsubr (&Sread_buffer); 
  defsubr (&Sread_input); 
  defsubr (&Sread_no_blanks_input); 
 
  defsubr (&Stry_completion); 
  defsubr (&Sall_completions); 
  defsubr (&Scompleting_read); 
  defsubr (&Sminibuffer_complete); 
  defsubr (&Sminibuffer_complete_word); 
  defsubr (&Sminibuffer_complete_and_exit); 
  defsubr (&Sminibuffer_completion_help); 
 
  defsubr (&Sself_insert_and_exit); 
  defsubr (&Sexit_minibuffer); 
} 
 
keys_of_minibuf () 
{ 
  ndefkey (MinibufLocalMap, Ctl ('g'), "abort-recursive-edit"); 
  ndefkey (MinibufLocalMap, Ctl ('m'), "exit-minibuffer"); 
  ndefkey (MinibufLocalMap, Ctl ('j'), "exit-minibuffer"); 
 
  ndefkey (MinibufLocalNSMap, Ctl ('g'), "abort-recursive-edit"); 
  ndefkey (MinibufLocalNSMap, Ctl ('m'), "exit-minibuffer"); 
  ndefkey (MinibufLocalNSMap, Ctl ('j'), "exit-minibuffer"); 
 
  ndefkey (MinibufLocalNSMap, ' ', "exit-minibuffer"); 
  ndefkey (MinibufLocalNSMap, '\t', "exit-minibuffer"); 
  ndefkey (MinibufLocalNSMap, '?', "self-insert-and-exit"); 
 
  ndefkey (MinibufLocalCompletionMap, Ctl ('g'), "abort-recursive-edit"); 
  ndefkey (MinibufLocalCompletionMap, Ctl ('m'), "exit-minibuffer"); 
  ndefkey (MinibufLocalCompletionMap, Ctl ('j'), "exit-minibuffer"); 
  ndefkey (MinibufLocalCompletionMap, 033, "minibuffer-complete"); 
  ndefkey (MinibufLocalCompletionMap, ' ', "minibuffer-complete-word"); 
  ndefkey (MinibufLocalCompletionMap, '?', "minibuffer-completion-help"); 
  ndefkey (MinibufLocalCompletionMap, 034, "ESC-prefix"); 
 
  ndefkey (MinibufLocalMustMatchMap, Ctl ('g'), "abort-recursive-edit"); 
  ndefkey (MinibufLocalMustMatchMap, Ctl ('m'), "minibuffer-complete-and-exit"); 
  ndefkey (MinibufLocalMustMatchMap, Ctl ('j'), "minibuffer-complete-and-exit"); 
  ndefkey (MinibufLocalMustMatchMap, 033, "minibuffer-complete"); 
  ndefkey (MinibufLocalMustMatchMap, ' ', "minibuffer-complete-word"); 
  ndefkey (MinibufLocalMustMatchMap, '?', "minibuffer-completion-help"); 
  ndefkey (MinibufLocalMustMatchMap, 034, "ESC-prefix"); 
} 
