/* Lisp functions pertaining to editing. 
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
 
#include <pwd.h> 
#include <stdio.h> 
#include <strings.h> 
#undef NULL 
#include "lisp.h" 
#include "buffer.h" 
#include "window.h" 
 
#define min(a, b) ((a) < (b) ? (a) : (b)) 
 
Lisp_Object ml_apply (); 
 
/* Some static data, and a function to initialize it for each run */ 
 
static struct passwd *pw;	/* password entry for the current user */ 
static char LoginId[12];	/* login ID of current user */ 
static char FullName[50];	/* full name of current user */ 
 
static char ConvertedSystemName[40]; 
static char *user_name; 
 
void 
init_editfns () 
{ 
  register char *p; 
 
  /* Don't bother with this on initial start when just dumping out */ 
  if (!NULL (Vpurify_flag)) 
    return; 
 
  pw = (struct passwd *) getpwuid (getuid ()); 
  strncpy (LoginId, pw->pw_name, sizeof LoginId); 
 
  user_name = (char *) getenv ("USER"); 
  if (!user_name) 
    user_name = LoginId; 
 
  if (strcmp (user_name, LoginId)) 
    pw = (struct passwd *) getpwnam (user_name); 
  strncpy (FullName, USER_FULL_NAME, sizeof FullName); 
 
  p = SYSTEM_NAME; 
  if (p == 0 || *p == 0) 
    p = "Bogus System Name"; 
  strncpy (ConvertedSystemName, p, sizeof ConvertedSystemName); 
  p = ConvertedSystemName; 
  while (*p) 
    { 
      if (*p < ' ') 
	*p = 0; 
      else 
	if (*p == ' ') 
	  *p = '-'; 
      p++; 
    } 
} 
 
DEFUN ("char-to-string", Fchar_to_string, Schar_to_string, 1, 1, 0, 
  "Convert arg CHAR to a string containing that character.") 
  (n) 
     Lisp_Object n; 
{ 
  char c; 
  CHECK_NUMBER (n, 0); 
 
  c = XINT (n); 
  return make_string (&c, 1); 
} 
 
DEFUN ("string-to-char", Fstring_to_char, Sstring_to_char, 1, 1, 0, 
  "Convert arg STRING to a character, the first character of that string.") 
  (str) 
     Lisp_Object str; 
{ 
  Lisp_Object val; 
  CHECK_STRING (str, 0); 
 
  if (XSTRING (str)->size) 
    XFASTINT (val) = ((unsigned char *) XSTRING (str)->data)[0]; 
  else 
    XFASTINT (val) = 0; 
  return val; 
} 
 
static Lisp_Object 
buildmark (val) 
{ 
  Lisp_Object mark; 
  mark = Fmake_marker (); 
  Fset_marker (mark, make_number (val), Qnil); 
  return mark; 
} 
 
DEFSIMPLE ("dot", Fdot, Sdot, 
  "Return value of dot, as an integer.\n\ 
Beginning of buffer is position 1.", 
	   Lisp_Int, XSETINT, dot) 
 
DEFUN ("dot-marker", Fdot_marker, Sdot_marker, 0, 0, 0, 
   "Return value of dot, as a marker object.") 
  () 
{ 
  return buildmark (dot); 
} 
 
DEFUN ("goto-char", Fgoto_char, Sgoto_char, 1, 1, "p", 
  "One arg, a number.  Set dot to that number.\n\ 
Beginning of buffer is position 1.") 
  (n) 
     Lisp_Object n; 
{ 
  int charno; 
  CHECK_NUMBER_COERCE_MARKER (n, 0); 
  charno = XINT (n); 
  if (charno < FirstCharacter) charno = FirstCharacter; 
  if (charno > NumCharacters) charno = NumCharacters + 1; 
  SetDot (charno); 
  return n; 
} 
 
DEFUN ("mark", Fmark, Smark, 0, 0, 0, 
  "Return this buffer's mark value as integer, or nil if no mark.") 
  () 
{ 
  Lisp_Object val; 
  if (!NULL (bf_cur->mark)) 
    return Fmarker_position (bf_cur->mark); 
  return Qnil; 
} 
 
DEFUN ("mark-marker", Fmark_marker, Smark_marker, 0, 0, 0, 
  "Return this buffer's mark, as a marker object, or nil if no mark.\n\ 
Watch out!  Moving this marker changes the buffer's mark.") 
  () 
{ 
  return bf_cur->mark; 
} 
 
DEFUN ("set-mark", Fset_mark, Sset_mark, 1, 1, "", 
  "Set this buffer's mark to POS;\n\ 
Argument is character position, or nil to clear out the mark.") 
  (pos) 
     Lisp_Object pos; 
{ 
  if (NULL (pos)) 
    { 
      bf_cur->mark = Qnil; 
      return Qnil; 
    } 
  CHECK_NUMBER_COERCE_MARKER (pos, 0); 
 
  if (NULL (bf_cur->mark)) 
    bf_cur->mark = Fmake_marker (); 
 
  Fset_marker (bf_cur->mark, pos, Qnil); 
  return pos; 
} 
 
Lisp_Object 
save_excursion_save () 
{ 
  Lisp_Object olddot, oldmark; 
  int visible = XBUFFER (XWINDOW (selected_window)->buffer) == bf_cur; 
 
  olddot = Fdot_marker (); 
 
  if (!NULL (bf_cur->mark)) 
    oldmark = Fcopy_marker (bf_cur->mark); 
  else 
    oldmark = Qnil; 
 
  return Fcons (olddot, Fcons (oldmark, visible ? Qt : Qnil)); 
} 
 
save_excursion_restore (info) 
     Lisp_Object info; 
{ 
  Lisp_Object tem; 
 
  tem = Fmarker_buffer (Fcar (info)); 
  /* If buffer being returned to is now deleted, avoid error */ 
  /* Otherwise could get error here while unwinding to top level 
     and crash */ 
  if (NULL (XBUFFER (tem)->name)) 
    return; 
  Fset_buffer (tem); 
  Fgoto_char (Fcar (info)); 
  unchain_marker (Fcar (info)); 
  tem = Fcar (Fcdr (info)); 
  Fset_mark (tem); 
  if (!NULL (tem)) 
    unchain_marker (tem); 
  tem = Fcdr (Fcdr (info)); 
  if (!NULL (tem) && bf_cur != XBUFFER (XWINDOW (selected_window)->buffer)) 
    Fswitch_to_buffer (Fcurrent_buffer ()); 
} 
 
DEFUN ("save-excursion", Fsave_excursion, Ssave_excursion, 0, UNEVALLED, 0, 
  "Save dot (and mark), execute BODY, then restore dot and mark.\n\ 
Executes BODY just like PROGN.  Dot and mark values are restored\n\ 
even in case of abnormal exit (throw or error).") 
  (args) 
     Lisp_Object args; 
{ 
  Lisp_Object val; 
  int count = specpdl_ptr - specpdl; 
 
  record_unwind_protect (save_excursion_restore, save_excursion_save ()); 
			  
  val = Fprogn (args); 
  unbind_to (count); 
  return val; 
} 
 
extern baud_rate; 
 
DEFSIMPLE ("baud-rate", Fbaud_rate, Sbaud_rate, 
	   "Return the output baud rate of the terminal.", 
	   Lisp_Int, XSETINT, baud_rate) 
DEFSIMPLE ("buffer-size", Fbufsize, Sbufsize, 
	   "Return the number of characters in the current buffer.", 
	   Lisp_Int, XSETINT, bf_s1 + bf_s2) 
 
DEFSIMPLE ("dot-min", Fdot_min, Sdot_min, 
	   "Return the minimum permissible value of dot in the current buffer.\n\ 
This is 1, unless a clipping restriction is in effect.", 
	   Lisp_Int, XSETINT, FirstCharacter) 
 
DEFUN ("dot-min-marker", Fdot_min_marker, Sdot_min_marker, 0, 0, 0, 
  "Return a marker to the beginning of the currently visible part of the buffer.\n\ 
This is the beginning, unless a clipping restriction is in effect.") 
  () 
{ 
  return buildmark (FirstCharacter); 
} 
 
DEFSIMPLE ("dot-max", Fdot_max, Sdot_max, 
	   "Return the minimum permissible value of dot in the current buffer.\n\ 
This is 1 plus the value of (buffer-size),\n\ 
unless a clipping restriction is in effect, in which case it is less.", 
	   Lisp_Int, XSETINT, NumCharacters+1) 
 
DEFUN ("dot-max-marker", Fdot_max_marker, Sdot_max_marker, 0, 0, 0, 
  "Return a marker to the end of the currently visible part of the buffer.\n\ 
This is the actual end, unless a clipping restriction is in effect.") 
  () 
{ 
  return buildmark (NumCharacters+1); 
} 
DEFSIMPLE ("following-char", Ffollchar, Sfollchar, 
	   "Return the character following dot, as a number.", 
	   Lisp_Int, XSETINT, dot>NumCharacters ? 0 : CharAt(dot)) 
DEFSIMPLE ("preceding-char", Fprevchar, Sprevchar, 
	   "Return the character preceding dot, as a number.", 
	   Lisp_Int, XSETINT, dot<=FirstCharacter ? 0 : CharAt(dot-1)) 
 
DEFPRED ("bobp", Fbobp, Sbobp, "Return T if dot is at the beginning of the buffer.", 
	 dot<=FirstCharacter) 
DEFPRED ("eobp", Feobp, Seobp, "Return T if dot is at the end of the buffer.", 
	 dot>NumCharacters) 
DEFPRED ("bolp", Fbolp, Sbolp, "Return T if dot is at the beginning of a line.", 
	 dot<=FirstCharacter || CharAt(dot-1)=='\n') 
DEFPRED ("eolp", Feolp, Seolp, "Return T if dot is at the end of a line.", 
	 dot>NumCharacters || CharAt(dot)=='\n') 
 
DEFUN ("char-after", Fchar_after, Schar_after, 1, 1, 0, 
  "One arg, POS, a number.  Return the character in the current buffer at position POS.\n\ 
If POS is out of range, the value is NIL.") 
  (pos) 
     Lisp_Object pos; 
{ 
  Lisp_Object val; 
  CHECK_NUMBER_COERCE_MARKER (pos, 0); 
  if (XINT (pos) < FirstCharacter || XINT (pos) > NumCharacters) return Qnil; 
 
  XFASTINT (val) = CharAt(XINT (pos)); 
  return val; 
} 
 
DEFUN ("user-login-name", Fuser_login_name, Suser_login_name, 0, 0, "", 
  "Return the name under which user logged in, as a string.") 
  () 
{ 
  return build_string (user_name); 
} 
 
DEFUN ("user-full-name", Fuser_full_name, Suser_full_name, 0, 0, "", 
  "Return the full name of the user logged in, as a string.") 
  () 
{ 
  return build_string (FullName); 
} 
 
DEFUN ("system-name", Fsystem_name, Ssystem_name, 0, 0, "", 
  "Return the name of the machine you are running on, as a string.") 
  () 
{ 
  return build_string (ConvertedSystemName); 
} 
 
DEFUN ("current-time-string", Fcurrent_time_string, Scurrent_time_string, 0, 0, "", 
  "Return the current time, as a human-readable string.") 
  () 
{ 
  long now = time ( (long *) 0); 
  char *tem = (char *) ctime (&now); 
  tem [24] = 0; 
  return build_string (tem); 
} 
 
DEFUN ("insert", Finsert, Sinsert, 0, MANY, 0, 
  "Any number of args, strings or chars.  Insert them after dot, moving dot forward.") 
  (nargs, args) 
     int nargs; 
     Lisp_Object *args; 
{ 
  int argnum; 
  Lisp_Object tem; 
  char str[1]; 
 
  for (argnum = 0; argnum < nargs; argnum++) 
    { 
      tem = args[argnum]; 
      if (XTYPE (tem) == Lisp_Int) 
	{ 
	  str[0] = XINT (tem); 
	  InsCStr (str, 1); 
	} 
      else if (XTYPE (tem) == Lisp_String) 
	{ 
	  InsCStr (XSTRING (tem)->data, XSTRING (tem)->size); 
	} 
      else 
	wrong_type_argument (Qchar_or_string_p, tem, argnum); 
    } 
  return Qnil; 
} 
 
/* Return a string with the contents of the current region */ 
 
DEFUN ("buffer-substring", Fbuffer_substring, Sbuffer_substring, 2, 2, 0, 
  "Return the contents of part of the current buffer as a string.\n\ 
The two arguments specify the start and end, as character numbers.") 
  (b, e) 
     Lisp_Object b, e; 
{ 
  int beg, end; 
 
  validate_region (&b, &e); 
  beg = XINT (b); 
  end = XINT (e); 
 
  if (beg <= bf_s1 && end > bf_s1) 
      GapTo (beg); 
  return make_string (&CharAt (beg), end - beg); 
} 
 
DEFUN ("buffer-string", Fbuffer_string, Sbuffer_string, 0, 0, 0, 
  "Return the contents of the current buffer as a string.") 
  () 
{ 
  if (FirstCharacter <= bf_s1 && NumCharacters + 1 > bf_s1) 
      GapTo (FirstCharacter); 
  return make_string (&CharAt (FirstCharacter), NumCharacters + 1 - FirstCharacter); 
} 
 
DEFUN ("region-to-string", Fregion_to_string, Sregion_to_string, 0, 0, 0, 
  "Return the contents of the region as a string") 
  () 
{ 
  int foo; 
  Lisp_Object b, e; 
 
  if (NULL (bf_cur->mark)) 
    error ("The mark is not set now"); 
  foo = marker_position (bf_cur->mark); 
 
  XFASTINT (b) = dot < foo ? dot : foo; 
  XFASTINT (e) = dot > foo ? dot : foo; 
  return Fbuffer_substring (b, e); 
} 
 
DEFUN ("insert-buffer-substring", Finsert_buffer_substring, Sinsert_buffer_substring, 
  1, 3, 0, 
  "Insert before dot a substring of the contents buffer BUFFER.\n\ 
BUFFER may be a buffer or a buffer name.\n\ 
Arguments START and END are character numbers specifying the substring.\n\ 
They default to the beginning and the end of BUFFER.") 
  (buf, b, e) 
     Lisp_Object buf, b, e; 
{ 
    int beg, end, exch; 
 
  buf = Fget_buffer (buf); 
  if (XBUFFER (buf) == bf_cur) 
    error ("Cannot insert buffer into itself"); 
 
  if (NULL (b)) 
    beg = XBUFFER (buf)->text.head_clip - 1; 
  else 
    { 
      CHECK_NUMBER_COERCE_MARKER (b, 0); 
      beg = XINT (b) - 1; 
    } 
  if (NULL (e)) 
    end = XBUFFER (buf)->text.size1 + XBUFFER (buf)->text.size2 
			- XBUFFER (buf)->text.tail_clip; 
  else 
    { 
      CHECK_NUMBER_COERCE_MARKER (e, 1); 
      end = XINT (e) - 1; 
    } 
 
  if (beg > end) 
    exch = beg, beg = end, end = exch; 
 
  if (!(XBUFFER (buf)->text.head_clip - 1 <= beg 
	&& beg <= end 
        && end <= XBUFFER (buf)->text.size1 + XBUFFER (buf)->text.size2 
			- XBUFFER (buf)->text.tail_clip)) 
    Fsignal (Qargs_out_of_range, Fcons (b, Fcons (e, Qnil))); 
 
  if (beg < XBUFFER (buf)->text.size1) 
    { 
      InsCStr (XBUFFER (buf)->text.p1 + 1 + beg, min (end, XBUFFER (buf)->text.size1) - beg); 
      beg = min (end, XBUFFER (buf)->text.size1); 
    } 
  if (beg < end) 
    InsCStr (XBUFFER (buf)->text.p2 + 1 + beg, end - beg); 
 
  return Qnil; 
} 
 
DEFUN ("subst-char-in-region", Fsubst_char_in_region, 
  Ssubst_char_in_region, 4, 4, 0, 
  "From START to END, replace FROMCHAR with TOCHAR each time it occurs.") 
  (start, end, fromchar, tochar) 
     Lisp_Object start, end, fromchar, tochar; 
{ 
  register int pos, stop, look; 
 
  validate_region (&start, &end); 
  CHECK_NUMBER (fromchar, 2); 
  CHECK_NUMBER (tochar, 3); 
 
  pos = XINT (start); 
  stop = XINT (end); 
  modify_region (pos, stop); 
 
  look = XINT (fromchar); 
 
  while (pos < stop) 
    { 
      if (CharAt (pos) == look) 
	{ 
	  RecordChange (pos, 1); 
	  CharAt (pos) = XINT (tochar); 
	} 
      pos++; 
    } 
 
  return Qnil; 
} 
 
DEFUN ("delete-region", Fdelete_region, Sdelete_region, 2, 2, "r", 
  "Delete the text between dot and mark.\n\ 
When called from a program, expects two arguments,\n\ 
character numbers specifying the stretch to be deleted.") 
  (b, e) 
     Lisp_Object b, e; 
{ 
  validate_region (&b, &e); 
  del_range (XINT (b), XINT (e)); 
  return Qnil; 
} 
 
DEFUN ("widen", Fwiden, Swiden, 0, 0, "", 
  "Remove restrictions from current buffer, allowing full text to be seen and edited.") 
  () 
{ 
  bf_cur->text.head_clip = bf_head_clip = 1; 
  bf_cur->text.tail_clip = bf_tail_clip = 0; 
  clip_changed = 1; 
  return Qnil; 
} 
 
DEFUN ("narrow-to-region", Fnarrow_to_region, Snarrow_to_region, 2, 2, "r", 
  "Restrict editing in current buffer to text between present values of dot and mark.\n\ 
Called interactively, takes two arguments, character numbers that specify\n\ 
the stretch to restrict to.") 
  (b, e) 
     Lisp_Object b, e; 
{ 
  int i; 
 
  CHECK_NUMBER_COERCE_MARKER (b, 0); 
  CHECK_NUMBER_COERCE_MARKER (e, 1); 
 
  if (XINT (b) > XINT (e)) 
    i = XINT (b), XINT (b) = XINT (e), XINT (e) = i; 
 
  if (!(1 <= XINT (b) && XINT (b) <= XINT (e) 
        && XINT (e) <= bf_s1 + bf_s2 + 1)) 
    Fsignal (Qargs_out_of_range, Fcons (b, Fcons (e, Qnil))); 
 
  bf_cur->text.head_clip = bf_head_clip = XFASTINT (b); 
  bf_cur->text.tail_clip = bf_tail_clip = bf_s1 + bf_s2 + 1 - XFASTINT (e); 
  if (dot < XFASTINT (b)) 
    SetDot (XFASTINT (b)); 
  if (dot > XFASTINT (e)) 
    SetDot (XFASTINT (e)); 
  clip_changed = 1; 
  return Qnil; 
} 
 
Lisp_Object 
save_restriction_save () 
{ 
  Lisp_Object ml, mh; 
  /* Note: I tried using markers here, but it does not win 
     because insertion at the end of the saved region 
     does not advance mh and is considered "outside" the saved region. */ 
  XFASTINT (ml) = bf_head_clip; 
  XFASTINT (mh) = bf_tail_clip; 
 
  return Fcons (Fcurrent_buffer (), Fcons (ml, mh)); 
} 
 
save_restriction_restore (data) 
     Lisp_Object data; 
{ 
  register struct buffer *old = bf_cur; 
  register int newhead, newtail; 
 
  Fset_buffer (XCONS (data)->car); 
 
  data = XCONS (data)->cdr; 
 
  newhead = XINT (XCONS (data)->car); 
  newtail = XINT (XCONS (data)->cdr); 
  if (newhead + newtail > bf_s1 + bf_s2 + 1) 
    { 
      newhead = 1; 
      newtail = 0; 
    } 
  bf_cur->text.head_clip = bf_head_clip = newhead; 
  bf_cur->text.tail_clip = bf_tail_clip = newtail; 
  clip_changed = 1; 
 
  /* If dot is outside the new visible range, move it inside. */ 
  if (dot < FirstCharacter) 
    SetDot (FirstCharacter); 
  if (dot > NumCharacters+1) 
    SetDot (NumCharacters+1); 
 
  SetBfp (old); 
} 
 
DEFUN ("save-restriction", Fsave_restriction, Ssave_restriction, 0, UNEVALLED, 0, 
  "Execute the body, undoing at the end any changes to current buffer's restrictions.\n\ 
Changes to restrictions are made by narrow-to-region or by widen.\n\ 
Thus, the restrictions are the same after this function as they were before it.\n\ 
The value returned is that returned by the last form in the body.\n\ 
\n\ 
This function can be confused if, within the body, you widen\n\ 
and then make changes outside the area within the saved restrictions.") 
  (body) 
     Lisp_Object body; 
{ 
  Lisp_Object val; 
  int count = specpdl_ptr - specpdl; 
 
  record_unwind_protect (save_restriction_restore, save_restriction_save ()); 
  val = Fprogn (body); 
  unbind_to (count); 
  return val; 
} 
 
DEFUN ("message", Fmessage, Smessage, 1, MANY, 0, 
  "Print a one-line message at the bottom of the screen.\n\ 
The first argument is a control string.\n\ 
It may contain %s or %d or %c to print successive following arguments.\n\ 
%s means print an argument as a string, %d means print as number in decimal,\n\ 
%c means print a number as a single character.\n\ 
The argument used by %s must be a string or a symbol;\n\ 
the argument used by %d or %c must be a number.") 
  (nargs, args) 
     int nargs; 
     Lisp_Object *args; 
{ 
  Lisp_Object val; 
  static char buf[100]; 
 
  val = Fformat (nargs, args); 
  strncpy (buf, XSTRING (val)->data, 99); 
  buf[99] = 0; 
 
  message1 (buf); 
  return Qnil; 
} 
 
DEFUN ("format", Fformat, Sformat, 1, MANY, 0, 
  "Format a string out of a control-string and arguments.\n\ 
The first argument is a control string.\n\ 
It, and subsequent arguments substituted into it, become the value, which is a string.\n\ 
It may contain %s or %d or %c to substitute successive following arguments.\n\ 
%s means print an argument as a string, %d means print as number in decimal,\n\ 
%c means print a number as a single character.\n\ 
The argument used by %s must be a string or a symbol;\n\ 
the argument used by %d or %c must be a number.") 
  (nargs, args) 
     int nargs; 
     Lisp_Object *args; 
{ 
  int i; 
  char buf[100]; 
  unsigned char **strings = (unsigned char **) alloca (nargs * sizeof (char *)); 
  struct _iobuf _strbuf; 
 
  for (i = 0; i < nargs; i++) 
    { 
      if (XTYPE (args[i]) == Lisp_String) 
        strings[i] = XSTRING (args[i])->data; 
      else if (XTYPE (args[i]) == Lisp_Symbol) 
        strings[i] = XSYMBOL (args[i])->name->data; 
      else if (XTYPE (args[i]) == Lisp_Int) 
        strings[i] = (unsigned char *) XINT (args[i]); 
      else 
        strings[i] = (unsigned char *) "??"; 
    } 
 
  _strbuf._flag = _IOSTRG; 
  _strbuf._ptr = buf; 
  _strbuf._cnt = sizeof buf; 
  _doprnt(strings[0], strings + 1, &_strbuf); 
  putc('\0', &_strbuf); 
 
  return build_string (buf); 
} 
 
Lisp_Object format1 (string1) 
     char *string1; 
{ 
  char buf[100]; 
  struct _iobuf _strbuf; 
 
  _strbuf._flag = _IOSTRG; 
  _strbuf._ptr = buf; 
  _strbuf._cnt = sizeof buf; 
  _doprnt(string1, &string1 + 1, &_strbuf); 
  putc('\0', &_strbuf); 
 
  return build_string (buf); 
} 
 
/* Compare two chars according to case-fold   APW 1/81 */ 
DEFUN ("char-equal", Fchar_equal, Schar_equal, 2, 2, 0, 
  "T if args (both characters (numbers)) match.  May ignore case.\n\ 
Case is ignored if the current buffer specifies to do so.") 
  (c1, c2) 
     Lisp_Object c1, c2; 
{ 
  extern char *downcase_table;  /* From search.c */ 
 
  CHECK_NUMBER (c1, 0); 
  CHECK_NUMBER (c2, 1); 
 
  if (!NULL (bf_cur->case_fold_search) 
      ? downcase_table[XINT (c1)] == downcase_table[XINT (c2)] 
      : XINT (c1) == XINT (c2)) 
    return Qt; 
  return Qnil; 
} 
 
DEFUN ("getenv", Fgetenv, Sgetenv, 1, 1, "sEnvironment variable: ", 
  "One arg VAR, a string. Return the value of environment variable VAR, as a string.") 
  (str) 
     Lisp_Object str; 
{ 
  char *val; 
  CHECK_STRING (str, 0); 
  val = (char *) getenv (XSTRING (str)->data); 
  if (!val) 
    return Qnil; 
  return build_string (val); 
} 
 
void 
syms_of_editfns () 
{ 
  defsubr (&Schar_equal); 
  defsubr (&Sgoto_char); 
  defsubr (&Sstring_to_char); 
  defsubr (&Schar_to_string); 
  defsubr (&Sregion_to_string); 
  defsubr (&Sbuffer_substring); 
  defsubr (&Sbuffer_string); 
  defsubr (&Sbaud_rate); 
 
  defsubr (&Sdot_marker); 
  defsubr (&Smark_marker); 
  defsubr (&Sdot); 
  defsubr (&Smark); 
  defsubr (&Sset_mark); 
  defsubr (&Ssave_excursion); 
 
  defsubr (&Sbufsize); 
  defsubr (&Sdot_max); 
  defsubr (&Sdot_min); 
 
  defsubr (&Sbobp); 
  defsubr (&Seobp); 
  defsubr (&Sbolp); 
  defsubr (&Seolp); 
  defsubr (&Sfollchar); 
  defsubr (&Sprevchar); 
  defsubr (&Schar_after); 
  defsubr (&Sinsert); 
 
  defsubr (&Suser_login_name); 
  defsubr (&Suser_full_name); 
  defsubr (&Scurrent_time_string); 
  defsubr (&Sgetenv); 
  defsubr (&Ssystem_name); 
  defsubr (&Smessage); 
  defsubr (&Sformat); 
 
  defsubr (&Sinsert_buffer_substring); 
  defsubr (&Ssubst_char_in_region); 
  defsubr (&Sdelete_region); 
  defsubr (&Swiden); 
  defsubr (&Snarrow_to_region); 
  defsubr (&Ssave_restriction); 
} 
