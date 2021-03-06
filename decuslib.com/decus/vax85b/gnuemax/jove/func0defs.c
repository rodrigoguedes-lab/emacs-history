/* /usr/src/local/jove/func_defs.c, Thu Sep 20 00:12:41 1984, ed: hsu */ 
 
#include "jove.h" 
 
/* There are two lists of commands, one for the editor and one for the 
   setmaps command.  I didn't know of any reasonable way to make all this 
   work, and as a result, and change made to the first list MUST be made 
   to the second one as well. */ 
 
#ifndef TXT_TO_C 
extern int 
	EscPrefix(), 
	CtlxPrefix(), 
	MiscPrefix(), 
	AdjWindows(), 
	AppReg(), 
	Apropos(), 
	BackChar(), 
	Bparen(), 
	BackWord(), 
	BackwardPara(), 
	Bof(), 
	Bol(), 
	Bos(), 
	Bow(), 
	BindAKey(), 
#ifdef MACROS 
	BindMac(), 
#endif 
	BufPos(), 
#ifdef CASEREG 
	CasRegLower(), 
	CasRegUpper(), 
#endif 
	CapWord(), 
	CenterLine(), 
#ifdef COMMENT 
	InsComment(), 
	InsertNewComment(), 
#endif 
	LowWord(), 
	UppWord(), 
#ifdef CHDIR 
	Chdir(), 
	prCWD(), 
#endif 
	prCTIME(), 
	DateEdit(), 
#ifdef DIRLIST 
	DirList(), 
#endif 
	ChrToOct(), 
	ClAndRedraw(), 
	MakeErrors(), 
	CopyRegion(), 
	BufSelect(), 
	DelBlnkLines(), 
#ifdef COMMENT 
	DeleteComment(), 
#endif 
#ifdef XFILE 
	DeleteFile(), 
	RenameFile(), 
#endif 
	DelNChar(), 
	DelNWord(), 
	OneWindow(), 
	DelPChar(), 
	DelPWord(), 
	DelReg(), 
	DelWtSpace(), 
	DelCurWindow(), 
	KeyDesc(), 
	Digit(), 
	DescBindings(), 
	DescCom(), 
	Eof(), 
	Eol(), 
	Eos(), 
	Eow(), 
	BufErase(), 
	PtToMark(), 
	Extend(), 
#ifdef MACROS 
	ExecMacro(), 
	RunMacro(), 
#endif 
	Leave(), 
	RegToShell(), 
	FindFile(), 
	WindFind(), 
	FindTag(), 
	ToIndent(), 
	ForChar(), 
	Fparen(), 
	ForWord(), 
	ForwardPara(), 
	FourTime(), 
	GoLine(), 
	GrowWindow(), 
#ifdef INCSEARCH 
	IncFSearch(), 
	IncRSearch(), 
#endif 
	InsFile(), 
	Justify(), 
	BufKill(), 
	KillEOL(), 
	KillEos(), 
	BufList(), 
	NotModified(), 
	MarkBuffer(), 
#ifdef MACROS 
	NameMac(), 
#endif 
	Newline(), 
	OpenLine(), 
	LineAI(), 
	IndentNewLine(), 
	NextError(), 
	PrevError(), 
	NextLine(), 
	NextPage(), 
	NextWindow(), 
	Recur(), 
	PopMark(), 
	PageNWind(), 
	PagePWind(), 
	UPageNWind(), 
	UPagePWind(), 
	Tab(), 
	DoParen(), 
	CParse(), 
	LintParse(), 
	XParse(), 
#ifdef SPELL 
	SpelWords(), 
#endif 
#ifdef JOB_CONTROL 
	PauseJove(), 
#endif 
	PrevLine(), 
	PrevPage(), 
	PrevWindow(), 
	Push(), 
	QRepSearch(), 
	QuotChar(), 
	ReadFile(), 
#ifdef MACROS 
	ReadMacs(), 
#endif 
	RedrawDisplay(), 
	ReInitTTY(), 
	ReNamBuf(), 
	RepSearch(), 
	RERepSearch(), 
	REQRepSearch(), 
	Beep(), 
	DownScroll(), 
	UpScroll(), 
	FillColumn(), 
	FillPrefix(), 
	ForSearch(), 
	RevSearch(), 
	REForSearch(), 
	RERevSearch(), 
	SelfInsert(), 
	InsertC(), 
	SetVar(), 
	SetIntrc(), 
	SetMark(), 
	ShellCom(), 
	ShToBuf(), 
	ShrWindow(), 
	Source(), 
#ifdef SPELL 
	SpelBuffer(), 
#endif 
	SplitWind(), 
#ifdef MACROS 
	Remember(), 
	Forget(), 
#endif 
	StrLength(), 
	PauseJove(), 
	XonXoffMode(), 
	TransChar(), 
	UnBound(), 
	Undo(), 
	SaveFile(), 
	WtModBuf(), 
	WriteFile(), 
#ifdef MACROS 
	WriteMacs(), 
#endif 
	WrtReg(), 
	Yank(), 
	YankPop(), 
	PrVar(), 
	SetQchars(), 
	WNumLines(); 
 
#ifdef PROCS 
extern int 
#ifdef SENDMAIL 
	SendMail(), 
	AbortSendmail(), 
	ExitSendmail(), 
#endif 
#ifdef RECVMAIL 
	ReadMail(), 
#endif 
	IShell(), 
	ProcInt(), 
	ProcQuit(), 
	ProcStop(), 
	ProcCont(), 
	ProcKill(), 
	ProcSend(), 
	ProcNewline(), 
	ProcList(), 
	ProcPrompt(), 
	Iprocess(), 
	RegToProc(), 
	KillUnixProcess(); 
#endif 
 
#define WIRED_FUNC(c)	c 
 
#else TXT_TO_C 
 
#define WIRED_FUNC(c)	0 
 
#endif TXT_TO_C 
 
extern int 
	AutoFill(), 
	OvrWrite(), 
	AutoSave(),	/* Buffer specific */ 
 
	AutoIndent(),	/* Not buffer specific */ 
	SenseCase(), 
	Magic(), 
	ShowMtch(); 
 
extern int 
	Cmode(),	/* Major mode */ 
	PascalMode(), 
	EuclidMode(), 
	CluMode(), 
	ScribeMode(), 
	TexMode(), 
	AsmMode(), 
	FundMode(), 
	TextMode(); 
 
struct funct	commands[] = { 
	FUNCTION, "Prefix-1", WIRED_FUNC(EscPrefix), 
	FUNCTION, "Prefix-2", WIRED_FUNC(CtlxPrefix), 
	FUNCTION, "Prefix-3", WIRED_FUNC(MiscPrefix), 
#ifdef SENDMAIL 
	FUNCTION, "abort-mail", WIRED_FUNC(AbortSendmail), 
#endif 
	FUNCTION, "append-region", WIRED_FUNC(AppReg), 
	FUNCTION, "apropos", WIRED_FUNC(Apropos), 
	FUNCTION, "asm-mode", WIRED_FUNC(AsmMode), 
	FUNCTION, "auto-fill-mode", WIRED_FUNC(AutoFill), 
	FUNCTION, "auto-indent-mode", WIRED_FUNC(AutoIndent), 
	FUNCTION, "auto-save-mode", WIRED_FUNC(AutoSave), 
	FUNCTION, "backward-character", WIRED_FUNC(BackChar), 
	FUNCTION, "backward-paragraph", WIRED_FUNC(BackwardPara), 
	FUNCTION, "backward-paren", WIRED_FUNC(Bparen), 
	FUNCTION, "backward-word", WIRED_FUNC(BackWord), 
	FUNCTION, "beginning-of-file", WIRED_FUNC(Bof), 
	FUNCTION, "beginning-of-line", WIRED_FUNC(Bol), 
	FUNCTION, "beginning-of-sentence", WIRED_FUNC(Bos), 
	FUNCTION, "beginning-of-window", WIRED_FUNC(Bow), 
	FUNCTION, "bind-to-key", WIRED_FUNC(BindAKey), 
	FUNCTION, "bind-macro-to-key", WIRED_FUNC(BindMac), 
	FUNCTION, "buffer-position", WIRED_FUNC(BufPos), 
	FUNCTION, "c-mode", WIRED_FUNC(Cmode), 
	FUNCTION, "case-region-lower", WIRED_FUNC(CasRegLower), 
	FUNCTION, "case-region-upper", WIRED_FUNC(CasRegUpper), 
	FUNCTION, "case-word-capitalize", WIRED_FUNC(CapWord), 
	FUNCTION, "case-word-lower", WIRED_FUNC(LowWord), 
	FUNCTION, "case-word-upper", WIRED_FUNC(UppWord), 
	FUNCTION, "center-line", WIRED_FUNC(CenterLine), 
	FUNCTION, "character-to-octal-insert", WIRED_FUNC(ChrToOct), 
#ifdef CHDIR 
	FUNCTION, "cd", WIRED_FUNC(Chdir), 
#endif 
	FUNCTION, "clear-and-redraw", WIRED_FUNC(ClAndRedraw), 
	FUNCTION, "clu-mode", WIRED_FUNC(CluMode), 
	FUNCTION, "compile-it", WIRED_FUNC(MakeErrors), 
#ifdef PROCS 
	FUNCTION, "continue-process", WIRED_FUNC(ProcCont), 
#endif 
	FUNCTION, "copy-region", WIRED_FUNC(CopyRegion), 
	FUNCTION, "current-time", WIRED_FUNC(prCTIME), 
	FUNCTION, "date-edit", WIRED_FUNC(DateEdit), 
	FUNCTION, "delete-blank-lines", WIRED_FUNC(DelBlnkLines), 
	FUNCTION, "delete-buffer", WIRED_FUNC(BufKill), 
	FUNCTION, "delete-comment", WIRED_FUNC(DeleteComment), 
#ifdef XFILE 
	FUNCTION, "delete-file", WIRED_FUNC(DeleteFile), 
#endif 
	FUNCTION, "delete-next-character", WIRED_FUNC(DelNChar), 
	FUNCTION, "delete-other-windows", WIRED_FUNC(OneWindow), 
	FUNCTION, "delete-prev-hacking-indentation", WIRED_FUNC(DelPChar), 
	FUNCTION, "delete-white-space", WIRED_FUNC(DelWtSpace), 
	FUNCTION, "delete-current-window", WIRED_FUNC(DelCurWindow), 
	FUNCTION, "describe-bindings", WIRED_FUNC(DescBindings), 
	FUNCTION, "describe-command", WIRED_FUNC(DescCom), 
	FUNCTION, "describe-key", WIRED_FUNC(KeyDesc), 
	FUNCTION, "digit", WIRED_FUNC(Digit), 
	FUNCTION, "end-of-file", WIRED_FUNC(Eof), 
	FUNCTION, "end-of-line", WIRED_FUNC(Eol), 
	FUNCTION, "end-of-sentence", WIRED_FUNC(Eos), 
	FUNCTION, "end-of-window", WIRED_FUNC(Eow), 
	FUNCTION, "erase-buffer", WIRED_FUNC(BufErase), 
	FUNCTION, "euclid-mode", WIRED_FUNC(EuclidMode), 
	FUNCTION, "exchange-point-and-mark", WIRED_FUNC(PtToMark), 
	FUNCTION, "execute-extended-command", WIRED_FUNC(Extend), 
	FUNCTION, "execute-keyboard-macro", WIRED_FUNC(ExecMacro), 
	FUNCTION, "execute-macro", WIRED_FUNC(RunMacro), 
	FUNCTION, "exit-jove", WIRED_FUNC(Leave), 
	FUNCTION, "filter-region", WIRED_FUNC(RegToShell), 
	FUNCTION, "find-file", WIRED_FUNC(FindFile), 
	FUNCTION, "fundamental-mode", WIRED_FUNC(FundMode), 
	FUNCTION, "window-find", WIRED_FUNC(WindFind), 
	FUNCTION, "find-tag", WIRED_FUNC(FindTag), 
	FUNCTION, "first-non-blank", WIRED_FUNC(ToIndent), 
	FUNCTION, "forward-character", WIRED_FUNC(ForChar), 
	FUNCTION, "forward-paragraph", WIRED_FUNC(ForwardPara), 
	FUNCTION, "forward-paren", WIRED_FUNC(Fparen), 
	FUNCTION, "forward-word", WIRED_FUNC(ForWord), 
	FUNCTION, "universal-arg", WIRED_FUNC(FourTime), 
	FUNCTION, "goto-line", WIRED_FUNC(GoLine), 
	FUNCTION, "grow-window", WIRED_FUNC(GrowWindow), 
	FUNCTION, "handle-paren", WIRED_FUNC(DoParen), 
	FUNCTION, "handle-tab", WIRED_FUNC(Tab), 
	FUNCTION, "i-search-forward", WIRED_FUNC(IncFSearch), 
	FUNCTION, "i-search-reverse", WIRED_FUNC(IncRSearch), 
	FUNCTION, "indent-new-line", WIRED_FUNC(IndentNewLine), 
	FUNCTION, "insert-comment", WIRED_FUNC(InsComment), 
	FUNCTION, "insert-file", WIRED_FUNC(InsFile), 
	FUNCTION, "insert-new-comment", WIRED_FUNC(InsertNewComment), 
	FUNCTION, "insert-typed-character", WIRED_FUNC(InsertC), 
	FUNCTION, "interrupt-character", WIRED_FUNC(SetIntrc), 
#ifdef PROCS 
	FUNCTION, "interrupt-process", WIRED_FUNC(ProcInt), 
	FUNCTION, "i-shell", WIRED_FUNC(IShell), 
	FUNCTION, "i-shell-command", WIRED_FUNC(Iprocess), 
#endif 
	FUNCTION, "justify-paragraph", WIRED_FUNC(Justify), 
	FUNCTION, "kill-next-word", WIRED_FUNC(DelNWord), 
	FUNCTION, "kill-previous-word", WIRED_FUNC(DelPWord), 
	FUNCTION, "kill-region", WIRED_FUNC(DelReg), 
	FUNCTION, "kill-to-end-of-line", WIRED_FUNC(KillEOL), 
	FUNCTION, "kill-next-sentence", WIRED_FUNC(KillEos), 
#ifdef DIRLIST 
	FUNCTION, "list-files", WIRED_FUNC(DirList), 
#endif 
#ifdef PROCS 
	FUNCTION, "kill-process", WIRED_FUNC(ProcKill), 
	FUNCTION, "kill-unix-process", WIRED_FUNC(KillUnixProcess), 
#endif 
	FUNCTION, "list-buffers", WIRED_FUNC(BufList), 
#ifdef PROCS 
	FUNCTION, "list-processes", WIRED_FUNC(ProcList), 
#endif 
	FUNCTION, "make-buffer-unmodified", WIRED_FUNC(NotModified), 
	FUNCTION, "mark-buffer", WIRED_FUNC(MarkBuffer), 
	FUNCTION, "name-keyboard-macro", WIRED_FUNC(NameMac), 
	FUNCTION, "newline", WIRED_FUNC(Newline), 
	FUNCTION, "newline-and-backup", WIRED_FUNC(OpenLine), 
	FUNCTION, "newline-and-indent", WIRED_FUNC(LineAI), 
	FUNCTION, "next-error", WIRED_FUNC(NextError), 
	FUNCTION, "next-line", WIRED_FUNC(NextLine), 
	FUNCTION, "next-page", WIRED_FUNC(NextPage), 
	FUNCTION, "next-window", WIRED_FUNC(NextWindow), 
	FUNCTION, "number-lines-in-window", WIRED_FUNC(WNumLines), 
	FUNCTION, "over-write-mode", WIRED_FUNC(OvrWrite), 
	FUNCTION, "page-next-window", WIRED_FUNC(PageNWind), 
	FUNCTION, "page-next-wind-up", WIRED_FUNC(UPageNWind), 
	FUNCTION, "page-prev-window", WIRED_FUNC(PagePWind), 
	FUNCTION, "page-prev-wind-up", WIRED_FUNC(UPagePWind), 
	FUNCTION, "parse-C-errors", WIRED_FUNC(CParse), 
	FUNCTION, "parse-LINT-errors", WIRED_FUNC(LintParse), 
	FUNCTION, "parse-spelling-errors", WIRED_FUNC(SpelWords), 
	FUNCTION, "parse-some-errors", WIRED_FUNC(XParse), 
	FUNCTION, "pascal-mode", WIRED_FUNC(PascalMode), 
#ifdef JOB_CONTROL, 
	FUNCTION, "pause-jove", WIRED_FUNC(PauseJove), 
#endif 
	FUNCTION, "pop-mark", WIRED_FUNC(PopMark), 
	FUNCTION, "previous-error", WIRED_FUNC(PrevError), 
	FUNCTION, "previous-line", WIRED_FUNC(PrevLine), 
	FUNCTION, "previous-page", WIRED_FUNC(PrevPage), 
	FUNCTION, "previous-window", WIRED_FUNC(PrevWindow), 
	FUNCTION, "print", WIRED_FUNC(PrVar), 
#ifdef PROCS 
	FUNCTION, "process-newline", WIRED_FUNC(ProcNewline), 
#endif 
#ifdef CHDIR 
	FUNCTION, "pwd", WIRED_FUNC(prCWD), 
#endif 
	FUNCTION, "push", WIRED_FUNC(Push), 
	FUNCTION, "query-replace-string", WIRED_FUNC(QRepSearch), 
#ifdef PROCS 
	FUNCTION, "quit-process", WIRED_FUNC(ProcQuit), 
#endif 
	FUNCTION, "quote-character", WIRED_FUNC(QuotChar), 
	FUNCTION, "re-search-forward", WIRED_FUNC(REForSearch), 
	FUNCTION, "re-search-reverse", WIRED_FUNC(RERevSearch), 
	FUNCTION, "re-query-replace-string", WIRED_FUNC(REQRepSearch), 
	FUNCTION, "re-replace-string", WIRED_FUNC(RERepSearch), 
	FUNCTION, "visit-file", WIRED_FUNC(ReadFile), 
	FUNCTION, "read-macros-from-file", WIRED_FUNC(ReadMacs), 
#ifdef RECVMAIL 
	FUNCTION, "read-mail", WIRED_FUNC(ReadMail), 
#endif 
	FUNCTION, "redistribute-windows", WIRED_FUNC(AdjWindows), 
	FUNCTION, "redraw-display", WIRED_FUNC(RedrawDisplay), 
	FUNCTION, "recursive-edit", WIRED_FUNC(Recur), 
#ifdef PROCS 
	FUNCTION, "region-to-process", WIRED_FUNC(RegToProc), 
#endif 
	FUNCTION, "reinitialize-terminal", WIRED_FUNC(ReInitTTY), 
	FUNCTION, "rename-buffer", WIRED_FUNC(ReNamBuf), 
#ifdef XFILE 
	FUNCTION, "rename-file", WIRED_FUNC(RenameFile), 
#endif 
	FUNCTION, "replace-string", WIRED_FUNC(RepSearch), 
	FUNCTION, "ring-the-bell", WIRED_FUNC(Beep), 
	FUNCTION, "scribe-mode", WIRED_FUNC(ScribeMode), 
	FUNCTION, "scroll-down", WIRED_FUNC(DownScroll), 
	FUNCTION, "scroll-up", WIRED_FUNC(UpScroll), 
	FUNCTION, "search-forward", WIRED_FUNC(ForSearch), 
	FUNCTION, "search-reverse", WIRED_FUNC(RevSearch), 
	FUNCTION, "select-buffer", WIRED_FUNC(BufSelect), 
	FUNCTION, "self-insert", WIRED_FUNC(SelfInsert), 
#ifdef SENDMAIL 
	FUNCTION, "send-mail", WIRED_FUNC(SendMail), 
#endif 
#ifdef PROCS 
	FUNCTION, "send-process", WIRED_FUNC(ProcSend), 
#endif 
	FUNCTION, "set", WIRED_FUNC(SetVar), 
	FUNCTION, "set-fill-column", WIRED_FUNC(FillColumn), 
	FUNCTION, "set-fill-prefix", WIRED_FUNC(FillPrefix), 
	FUNCTION, "set-mark", WIRED_FUNC(SetMark), 
#ifdef PROCS 
	FUNCTION, "set-process-prompt", WIRED_FUNC(ProcPrompt), 
#endif 
	FUNCTION, "quote-characters", WIRED_FUNC(SetQchars), 
	FUNCTION, "shell-command", WIRED_FUNC(ShellCom), 
	FUNCTION, "shell-command-to-buffer", WIRED_FUNC(ShToBuf), 
	FUNCTION, "show-match-mode", WIRED_FUNC(ShowMtch), 
	FUNCTION, "shrink-window", WIRED_FUNC(ShrWindow), 
	FUNCTION, "source", WIRED_FUNC(Source), 
	FUNCTION, "spell-buffer", WIRED_FUNC(SpelBuffer), 
	FUNCTION, "split-current-window", WIRED_FUNC(SplitWind), 
	FUNCTION, "start-remembering", WIRED_FUNC(Remember), 
#ifdef PROCS 
	FUNCTION, "stop-process", WIRED_FUNC(ProcStop), 
#endif 
	FUNCTION, "stop-remembering", WIRED_FUNC(Forget), 
	FUNCTION, "string-length", WIRED_FUNC(StrLength), 
#ifdef JOB_CONTROL 
	FUNCTION, "suspend-jove", WIRED_FUNC(PauseJove), 
#endif 
	FUNCTION, "tex-mode", WIRED_FUNC(TexMode), 
	FUNCTION, "text-mode", WIRED_FUNC(TextMode), 
	FUNCTION, "toggle-xon-xoff-mode", WIRED_FUNC(XonXoffMode), 
	FUNCTION, "transpose-characters", WIRED_FUNC(TransChar), 
	FUNCTION, "unbound", WIRED_FUNC(UnBound), 
	FUNCTION, "undo", WIRED_FUNC(Undo), 
	FUNCTION, "write-current-file", WIRED_FUNC(SaveFile), 
	FUNCTION, "write-macros-to-file", WIRED_FUNC(WriteMacs), 
	FUNCTION, "write-modified-files", WIRED_FUNC(WtModBuf), 
	FUNCTION, "write-named-file", WIRED_FUNC(WriteFile), 
	FUNCTION, "write-region", WIRED_FUNC(WrtReg), 
	FUNCTION, "yank", WIRED_FUNC(Yank), 
	FUNCTION, "yank-pop", WIRED_FUNC(YankPop), 
	FUNCTION, 0, 0 
}; 
 
#ifndef TXT_TO_C 
data_obj * 
FindCom(prompt, flags) 
char	*prompt; 
{ 
	static char	*strings[(sizeof commands) / sizeof (struct funct)]; 
	static int	beenhere = 0; 
	register char	**strs = strings; 
	register int	com; 
	register struct funct	*f = commands; 
	extern int	InJoverc; 
 
	if (beenhere == 0) { 
		beenhere = 1; 
		for (; f->Name; f++) 
			*strs++ = f->Name; 
		*strs = 0; 
	} 
 
	if ((com = complete(strings, prompt, flags)) < 0) 
		return InJoverc ? (data_obj *) -1 : 0; 
	f = &commands[com]; 
	return (data_obj *) f; 
} 
#endif 
 
