// 6502 assembler.
// Note for experienced developers: the code was written when I was still a student and didn't know about Dragon Books or regular expressions.
// That's why the code looks naive, but that's even good. New developers will learn it faster and take the ideas into circulation.
#include "pch.h"

/*
	Assembler syntax:

	[LABEL:]  COMMAND  [OPERAND1, OPERAND2, OPERAND3]       ; Comments

	Commands can be any 6502 instruction or one of reserved directives:
		ORG, INCLUDE, DEFINE, BYTE, WORD, END, PROCESSOR, ABS

	Register names and CPU instructions cannot be used as label names.

	You cannot DEFINE, if such label is already defined.
	Redefinition of labels is NOT allowed.
	Redefinition of DEFINEs just replace previous definition.

	All operands are evaluated with the expression engine (eval_expr):
	labels, defines and complex expressions like MyData + 32 * entry_size + 12
	are supported in every operand position.

	The assembler performs multiple passes until all identifiers, labels and defines
	are resolved (this resolves the Zero Page / Absolute ambiguity for forward
	references to labels placed below $100). The ABS directive forces the absolute
	opcode for the next instruction with such an ambiguity.
*/

static uint8_t *PRG;
long org;        // current emit offset
long stop;
long errors;

int silent = 0;      // 1: suppress error output (used for the intermediate assembly passes)
int force_abs = 0;   // 1: the ABS directive is active - the next instruction must use absolute addressing
FILE* list_file = NULL; // Listing output stream (NULL = listing disabled)

// Previously, instead of list, we used a ordinary array of large size (limited) + a variable that stores the number of records.
// Now it is essentially the same, but instead of an array it is a list of virtually unlimited size (as long as memory is available)

static  std::list<label_s *> labels;    // name labels
static  std::list<patch_s *> patches;   // patch history
static  std::list<define_s *> defines;  // definitions

static  std::list<std::string> source_name_stack;	// Stack for the name of the current source file (for INCLUDE)
static  std::list<int> linenum_stack;		// Stack for the line number of the current source file (for INCLUDE)

// Snapshot of label values from the previous pass (used to detect multi-pass convergence).
static  std::list<std::pair<std::string, long>> prev_labels;

// Listing lines are recorded during the final pass and formatted after patching,
// so that the listing shows the final patched bytes.
struct listing_line_s {
	long    org_before;     // PRG offset before the line was assembled
	long    org_after;      // PRG offset after the line was assembled (for ORG)
	char    label[256];
	char    cmd[256];
	char    op[1024];
};
static  std::list<listing_line_s*> listing_lines;

// ****************************************************************

/// <summary>
/// The linenum stack is required to support INCLUDE. Where previously just the linenum variable was used, this call is now used.
/// </summary>
/// <returns>Line number where the assembler is currently located in the current source file</returns>
int get_linenum()
{
	if (linenum_stack.empty())
		return 0;
	return linenum_stack.back();
}

void nextline()
{
	linenum_stack.back()++;
}

/// <summary>
/// Get the name of the current source file currently being worked with.
/// </summary>
/// <returns></returns>
std::string get_source_name()
{
	if (source_name_stack.empty())
		return "";
	return source_name_stack.back();
}

// Labels management.
//

label_s * label_lookup (char *name)
{
	label_s * label;
	for (auto it = labels.begin(); it != labels.end(); ++it) {
		label = *it;
		if (!_stricmp(label->name, name)) return label;
	}
	return NULL;
}

label_s *add_label (const char *name, long orig)
{
	char temp_name[0x200] = { 0 };
	strcpy(temp_name, name);

	int len = (int)strlen (temp_name), i;
	label_s * label;
	for (i=len-1; i>=0; i--) {
		if (temp_name[i] <= ' ') temp_name[i] = 0;
	}
	//printf ( "ADD LABEL(%s,%i): \'%s\' = %08X\n", get_source_name().c_str(), get_linenum(), temp_name, orig);
	label = label_lookup (temp_name);
	if ( label == NULL ) {
		label = new label_s;
		strcpy (label->name, temp_name);
		label->orig = orig;
		strcpy(label->source, get_source_name().c_str());
		label->line = get_linenum();
		label->composite = 0;
		labels.push_back(label);
	}
	else {
		if ( label->orig == KEYWORD ) {
			ERR ("ERROR(%s,%i): Reserved keyword used as label \'%s\'\n", get_source_name().c_str(), get_linenum(), temp_name);
			errors++;
		}
		else if (orig != UNDEF && label->orig != UNDEF ) {
			ERR ("ERROR(%s,%i): label \'%s\' already defined in %s, line %i\n", get_source_name().c_str(), get_linenum(), temp_name, label->source, label->line);
			errors++;
		}
		else {
			if (orig != UNDEF) {
				label->orig = orig;
				strcpy(label->source, get_source_name().c_str());
				label->line = get_linenum();
			}
		}
	}
	return label;
}

/// <summary>
/// Handle special labels that are actually complex expressions.
/// </summary>
static void do_expr_labels()
{
	for (auto it = labels.begin(); it != labels.end(); ++it) {
		label_s* label = *it;
		if (label->composite) {
			label->orig = eval_expr(label->name, false, false);
		}
	}
}

static void dump_labels (void)
{
	int i = 0;
	label_s * label;
	printf ("\nLABELS (%d):\n", (int)labels.size());
	for (auto it = labels.begin(); it != labels.end(); ++it) {
		label = *it;
		if (label->orig == KEYWORD) continue;
		printf("%i: $%08X = \'%s\' %s\n", i + 1, label->orig, label->name, label->composite ? "=> expr" : "");
		i++;
	}
}

// Patch management.
//

void add_patch (label_s *label, long orig, int branch)
{
	patch_s * patch = new patch_s;
	patch->label = label;
	patch->orig = orig;
	patch->branch = branch;
	strcpy(patch->source, get_source_name().c_str());
	patch->line = get_linenum();
	patches.push_back(patch);
}

static void do_patch (void)
{
	long orig;
	int rel;
	patch_s * patch;
	for (auto it= patches.begin(); it!= patches.end(); ++it) {
		patch = *it;
		orig = patch->label->orig;
		if ( orig == UNDEF ) {
			ERR ("ERROR(%s,%i): Undefined label \'%s\' (%08X)\n", patch->source, patch->line, patch->label->name, orig );
			errors++;
		}
		else { 
			if ( patch->branch == 1 ) {          // relative branch
				org = patch->orig;
				rel = orig - org - 1;
				if (rel > 127 || rel < -128) {
					ERR("ERROR(%s,%i): Branch relative offset to %s out of range\n", patch->source, patch->line, patch->label->name);
					errors++;
				}
				else emit(rel & 0xff);
			}
			else if ( patch->branch == 2 ) {     // zero page operand
				org = patch->orig;
				emit(orig & 0xff);
			}
			else {                               // absolute jump / word operand
				org = patch->orig;
				emit ( orig & 0xff );
				emit ( (orig >> 8) & 0xff );
			}    
		}
	}
}

static void dump_patches (void)
{
	patch_s * patch;
	printf ("\nPATCHES (%d):\n", (int)patches.size());
	for (auto it = patches.begin(); it != patches.end(); ++it) {
		patch = *it;
		printf("%s, line %i: $%08X = \'%s\' ($%08X)", patch->source, patch->line, patch->orig, patch->label->name, patch->label->orig);
		if (patch->branch == 1) printf(" (REL)\n");
		else if (patch->branch == 2) printf(" (ZPG)\n");
		else printf(" (ABS)\n");
	}
}

// Defines management.
//

define_s * define_lookup (char *name)
{
	define_s * def;
	for (auto it = defines.begin(); it != defines.end(); ++it) {
		def = *it;
		if (!_stricmp(def->name, name)) return def;
	}
	return NULL;
}

define_s *add_define (char *name, char *replace)
{
	label_s * label;
	define_s * def;

	if ( label = label_lookup (name) ) {
		ERR ("ERROR(%s,%i): Already defined as label in %s, line %i\n", get_source_name().c_str(), get_linenum(), label->source, label->line );
		errors++;
		return NULL;
	}

	def = define_lookup (name);
	if ( def ) {
		strcpy (def->replace, replace);
	}
	else {
		def = new define_s;
		strcpy (def->name, name);
		strcpy (def->replace, replace);
		defines.push_back(def);
	}
	return def;
}

static void dump_defines (void)
{
	int i = 0;
	define_s * def;
	printf ("\nDEFINES (%d):\n", (int)defines.size());
	for (auto it = defines.begin(); it != defines.end(); ++it) {
		def = *it;
		printf("%i: %s = %s\n", i + 1, def->name, def->replace);
		i++;
	}
}

// ****************************************************************
// Evaluate expression
//
// All numeric evaluation is delegated to the expression engine (eval_expr):
//   - "#..."   -> EVAL_NUMBER  (immediate)
//   - leading digit -> EVAL_NUMBER  (legacy behavior: plain numbers are immediate)
//   - "$..."   -> EVAL_ADDRESS (memory address)
//   - "..." / '...' -> EVAL_STRING
//   - identifier / complex expression -> EVAL_LABEL (if not resolved yet, the whole
//     expression is registered as a "composite" label whose value is computed
//     by do_expr_labels() at the end of the pass)

// Returns true if the whole operand is wrapped in a single pair of parentheses
// (indirect addressing: JMP (addr)). In that case the outer parentheses are
// replaced with spaces. Parentheses inside the expression are grouping only.
static bool strip_outer_parens(char* text)
{
	int len = (int)strlen(text);
	int i = 0, j = len - 1;
	while (i < len && text[i] <= ' ') i++;
	while (j >= 0 && text[j] <= ' ') j--;
	if (i >= j || text[i] != '(' || text[j] != ')')
		return false;

	// The first '(' must match the last ')'
	int depth = 0;
	for (int k = i; k <= j; k++) {
		if (text[k] == '(') depth++;
		else if (text[k] == ')') {
			depth--;
			if (depth == 0 && k != j)
				return false;   // parentheses close early: grouping, not indirect
		}
	}
	if (depth != 0)
		return false;

	text[i] = ' ';
	text[j] = ' ';
	return true;
}

int eval (char *text, eval_t *result)
{
	char buf[1024];
	char c, quot = 0;
	int i, len;
	bool resolved = false;

	// Indirect test: the whole operand wrapped in parentheses is indirect addressing (JMP (addr))
	result->indirect = strip_outer_parens(text) ? 1 : 0;
	len = (int)strlen (text);
	for (i=len-1; i>=0; i--) {
		if ( text[i] <= ' ' ) text[i] = 0;
		else break;
	}
	char* ptr = text;
	while (*ptr <= ' ' && *ptr) ptr++;      // Skip whitespaces
	text = ptr;

	// String
	if ( text[0] == '\'' || text[0] == '\"' ) {
		quot = text[0];
		text++;
		ptr = buf;
		while (1) {
			c = *text++;
			if (c == 0 || c == quot) break;
			else *ptr++ = c;
		}
		*ptr++ = 0;
		strncpy (result->string, buf, 255);
		return result->type = EVAL_STRING;
	}

	// A pure identifier [_a-zA-Z][_a-zA-Z0-9]* is a label or a define
	if (text[0] != '#' && !isdigit(text[0]) && text[0] != '$') {
		int ident = 1;
		for (i = 0; text[i]; i++) {
			c = text[i];
			if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
				ident = 0;
				break;
			}
		}
		if (ident) {
			define_s* def = define_lookup (text);
			if (def) return eval (def->replace, result);
			label_s* label = label_lookup (text);
			if (!label) label = add_label(text, UNDEF);
			result->label = label;
			return result->type = EVAL_LABEL;
		}
	}

	// Everything else is evaluated by the expression engine
	long value = eval_expr(text, false, true, &resolved);
	if (!resolved) {
		// The expression refers to not-yet-defined identifiers.
		// Register the whole expression as a composite label; its value
		// is computed by do_expr_labels() at the end of the pass.
		label_s* label = add_label(text, UNDEF);
		label->composite = 1;
		result->label = label;
		return result->type = EVAL_LABEL;
	}

	if (text[0] == '#' || isdigit(text[0])) {      // Immediate number
		result->number = value;
		return result->type = EVAL_NUMBER;
	}

	result->address = value;                       // Memory address
	return result->type = EVAL_ADDRESS;
}

static void dump_eval (eval_t *eval)
{
	switch (eval->type)
	{
		case EVAL_NUMBER:
			printf ("NUMBER: %08X\n", eval->number);
			break;
		case EVAL_ADDRESS:
			if (eval->indirect) printf ("ADDRESS: [$%08X]\n", eval->address);
			else printf ("ADDRESS: $%08X\n", eval->address);
			break;
		case EVAL_LABEL:
			if (eval->indirect) printf ("LABEL: [%s] (%08X)\n", eval->label->name, eval->label->orig);
			else printf ("LABEL: %s (%08X)\n", eval->label->name, eval->label->orig);
			break;
		case EVAL_STRING:
			printf ("STRING: %s\n", eval->string);
			break;
		default:
			printf("Undefined expression\n");
			break;
	}
}

// ****************************************************************

// Parser.

param_t *params;
int param_num;

static void add_param (char *string)
{
	param_t * param;
	while (*string <= ' ' && *string) string++; // Skip whitespaces
	if (strlen (string) == 0) return;
	params = (param_t *)realloc (params, sizeof(param_t) * (param_num+1) );
	param = &params[param_num];
	strncpy (param->string, string, 1023);
	param_num++;
}

static void dump_param (void)
{
	int i;
	param_t *param;
	for (i=0; i<param_num; i++) {
		param = &params[i];
		printf ("%i: %s\n", i, param->string);
	} 
}

void split_param (char *op)
{
	char param[1024]{};
	char c, *ptr = param, quot = 0;

	memset (param, 0, sizeof(param));

	if ( params ) {         // Cleanup
		free (params);
		params = NULL;
	}
	param_num = 0;

	while (1) {
		c = *op++;
		if ( c == 0 ) break;
		else if (c==',' && !quot) {
			*ptr++ = 0;
			add_param (param);
			ptr = param;
		}
		else if ( c == '\'' || c== '\"' ) {
			if ( quot == 0 ) quot = c;
			else if (quot == c) quot = 0;
			*ptr++ = c;
		}
		else *ptr++ = c;
	}
	*ptr++ = 0;
	add_param (param);
}

static void parse_line (char **text, line& ln)
{
	int timeout = 10000;
	char c;
	char linebuf[1000], *lp = linebuf;
	char label[1000] = {0}, cmd[1000] = {0}, op[1000] = {0}, *pp;
	int parsing_cmd = 1;
	int quot = 0, dquot = 0;

	// Get actual line characters
	while (timeout--) {
		c = **text;
		(*text)++;
		if (c == 0 || c == '\n') break;
		else if (c == ';') {
			while (**text != '\n' ) (*text)++;
		}
		else *lp++ = c;
	}
	*lp++ = 0;

	//printf ("LINE: %s\n", linebuf);

	// Parse line
	lp = linebuf;
	pp = cmd;
	while ( *lp <= ' ' && *lp) lp++;
	while (1) {
		c = *lp++;
		if (c == 0) break;
		if (c == '\'' && !dquot) quot ^= 1;
		if (c == '\"' && !quot) dquot ^= 1;
		if (c == ':' && !(quot || dquot)) {
			*pp++ = 0;              // complete parsing label
			strcpy (label, cmd);
			pp = cmd;
			parsing_cmd = 1;
			while ( *lp <= ' ' && *lp) lp++;
		}
		else if (c <= ' ' && parsing_cmd) {
			*pp++ = 0;          // complete parsing command
			pp = op;
			parsing_cmd = 0;
			while ( *lp <= ' ' && *lp) lp++;
		}
		else *pp++ = c;
	}
	*pp++ = 0;      // complete parsing operands

	strcpy (ln.label, label);
	strcpy (ln.cmd, cmd);
	strcpy (ln.op, op);
}

// ****************************************************************

int emit_mode = 1;		// 1: output byte stream; 0: just change origin

void set_emit_mode(int mode)
{
	emit_mode = mode;
}

void emit (uint8_t b)
{
	if (emit_mode) {
		PRG[org++] = b;
	}
	else {
		org++;
	}
}

static oplink optab[] = {

	// CPU Instructions (all handled by the unified table-driven handler)

	{ "BRK", op_std }, { "RTI", op_std }, { "RTS", op_std },

	{ "PHP", op_std }, { "CLC", op_std }, { "PLP", op_std }, { "SEC", op_std },
	{ "PHA", op_std }, { "CLI", op_std }, { "PLA", op_std }, { "SEI", op_std },
	{ "DEY", op_std }, { "TYA", op_std }, { "TAY", op_std }, { "CLV", op_std },
	{ "INY", op_std }, { "CLD", op_std }, { "INX", op_std }, { "SED", op_std },

	{ "TXA", op_std }, { "TXS", op_std }, { "TAX", op_std }, { "TSX", op_std },
	{ "DEX", op_std }, { "NOP", op_std },

	{ "BPL", opBRA }, { "BMI", opBRA }, { "BVC", opBRA }, { "BVS", opBRA },
	{ "BCC", opBRA }, { "BCS", opBRA }, { "BNE", opBRA }, { "BEQ", opBRA },

	{ "JSR", op_std }, { "JMP", op_std }, 

	{ "ORA", op_std }, { "AND", op_std }, { "EOR", op_std }, { "ADC", op_std }, 
	{ "CMP", op_std }, { "SBC", op_std }, 
	{ "CPX", op_std }, { "CPY", op_std }, 
	{ "INC", op_std }, { "DEC", op_std }, 
	{ "BIT", op_std },
	{ "ASL", op_std }, { "LSR", op_std }, { "ROL", op_std }, { "ROR", op_std }, 

	{ "LDA", op_std },
	{ "LDX", op_std },
	{ "LDY", op_std },
	{ "STA", op_std },
	{ "STX", op_std },
	{ "STY", op_std },

	// Directives
	{ "INCLUDE", opINCLUDE },
	{ "DEFINE", opDEFINE },
	{ "BYTE", opBYTE },
	{ "WORD", opWORD },
	{ "ORG", opORG },
	{ "END", opEND },
	{ "PROCESSOR", opDUMMY },
	{ "ABS", opABS },

	{ NULL, NULL }
};

static void cleanup (void)
{
	// Clear labels
	while (!labels.empty()) {
		label_s* item = labels.back();
		labels.pop_back();
		delete item;
	}

	// Clear patch table
	while (!patches.empty()) {
		patch_s* item = patches.back();
		patches.pop_back();
		delete item;
	}

	// Clear defines
	while (!defines.empty()) {
		define_s* item = defines.back();
		defines.pop_back();
		delete item;
	}
}

// Register the labels that cannot be used as user labels (keywords).
static void add_keywords(void)
{
	add_label ("A", KEYWORD);
	add_label ("X", KEYWORD);
	add_label ("Y", KEYWORD);
	oplink* opl = optab;
	while (1) {
		if ( opl->name == NULL ) break;
		else add_label (opl->name, KEYWORD);
		opl++;
	}
}

static void assemble_text(char* text)
{
	oplink* opl;
	line l;

	while (1) {
		if (*text == 0) break;
		parse_line(&text, l);

		// Add label
		if (strlen(l.label) > 1) {
			add_label(l.label, org);
		}

		long org_before = org;

		// Execute command
		if (strlen(l.cmd) > 1) {
			opl = optab;
			while (1) {
				if (opl->name == NULL) {
					ERR("ERROR(%s,%i): Unknown command %s\n", get_source_name().c_str(), get_linenum(), l.cmd);
					errors++;
					break;
				}
				else if (!_stricmp(opl->name, l.cmd)) {
					_strupr(l.cmd);
					opl->handler(l.cmd, l.op);
					break;
				}
				opl++;
			}
			// The ABS hint directive applies to the next CPU instruction only:
			// it is not consumed by labels or non-emitting directives.
			if (opl->handler == op_std || opl->handler == opBRA) force_abs = 0;
			if (stop) break;
		}

		// Listing (recorded now, formatted after patching)
		if (list_file && !silent) {
			listing_line_s* rec = new listing_line_s;
			rec->org_before = org_before;
			rec->org_after = org;
			strcpy(rec->label, l.label);
			strcpy(rec->cmd, l.cmd);
			strcpy(rec->op, l.op);
			listing_lines.push_back(rec);
		}

		nextline();
	}
}

// Multi-pass convergence helpers.
//

static void snapshot_labels(void)
{
	prev_labels.clear();
	for (auto it = labels.begin(); it != labels.end(); ++it) {
		prev_labels.push_back(std::make_pair(std::string((*it)->name), (*it)->orig));
	}
}

/// <summary>
/// Get the value of the label from the previous assembly pass (if any).
/// During multi-pass assembling forward-referenced labels are not defined yet
/// at the place of use, so the addressing mode is chosen by the best-known
/// value from the previous pass, and the operand is patched with the final value.
/// </summary>
bool prev_label_value(const char* name, long* out)
{
	for (auto it = prev_labels.begin(); it != prev_labels.end(); ++it) {
		if (!_stricmp(it->first.c_str(), name)) {
			*out = it->second;
			return true;
		}
	}
	return false;
}

static bool labels_changed(void)
{
	if (prev_labels.size() != labels.size())
		return true;
	auto it2 = prev_labels.begin();
	for (auto it = labels.begin(); it != labels.end(); ++it, ++it2) {
		if (it2->first != (*it)->name || it2->second != (*it)->orig)
			return true;
	}
	return false;
}

// Run a single assembly pass. Returns the pass error count.
static int run_pass(char* text, char* source_name, uint8_t* prg)
{
	PRG = prg;
	org = 0;
	stop = 0;
	errors = 0;
	force_abs = 0;

	// Zero the PRG: a previous pass may have written bytes at offsets that the
	// current pass no longer reaches (e.g. a zero page instruction replaced an
	// absolute one after the label was resolved).
	memset(prg, 0, 0x10000);

	cleanup ();

	source_name_stack.clear();
	linenum_stack.clear();
	source_name_stack.push_back(source_name);
	linenum_stack.push_back(0);
	add_keywords();
	nextline();

	assemble_text(text);

	// Patch jump/branch offsets.
	do_expr_labels();
	do_patch();

	return errors;
}

int assemble (char *text, char* source_name, uint8_t *prg)
{
	const int MAX_PASSES = 100;
	int pass;

	prev_labels.clear();

	// Multi-pass: keep assembling until all identifiers, labels and defines
	// are resolved and the label values stop changing between passes.
	// The intermediate passes run silently; the last pass reports the errors.

	for (pass = 0; pass < MAX_PASSES; pass++) {
		silent = 1;
		run_pass(text, source_name, prg);
		if (pass > 0 && !labels_changed())
			break;
		snapshot_labels();
	}

	if (pass == MAX_PASSES) {
		printf("WARNING: assembly did not converge after %d passes\n", MAX_PASSES);
	}

	// Final reporting pass (also generates the listing).
	silent = 0;
	run_pass(text, source_name, prg);

	// The listing is flushed after patching, so it shows the final bytes.
	if (list_file) {
		for (auto it = listing_lines.begin(); it != listing_lines.end(); ++it) {
			listing_line_s* rec = *it;
			if (!_stricmp(rec->cmd, "ORG")) {
				fprintf(list_file, "$%04X: org %s\n", rec->org_after, rec->op);
			}
			else {
				int num_bytes = (int)(rec->org_after - rec->org_before);
				fprintf(list_file, "$%04X: ", rec->org_before);
				for (int i = 0; i < num_bytes; i++) {
					fprintf(list_file, "%02X ", PRG[rec->org_before + i]);
				}
				for (int i = num_bytes; i < 4; i++) {
					fprintf(list_file, "   ");
				}
				if (rec->label[0]) {
					fprintf(list_file, "%s:", rec->label);
					if (rec->cmd[0]) fprintf(list_file, " %s %s", rec->cmd, rec->op);
				}
				else {
					fprintf(list_file, "%s %s", rec->cmd, rec->op);
				}
				fprintf(list_file, "\n");
			}
			delete rec;
		}
		listing_lines.clear();
	}

#ifdef _DEBUG
	dump_labels ();
	dump_defines ();
	dump_patches ();
#endif

	printf ( "%i error(s)\n", errors );
	return errors;
}

/// <summary>
/// Assemble nested source file (called from the INCLUDE directive).
/// The name of the source file will be placed on the name stack, and the line counter will count from the beginning of the file.
/// </summary>
/// <param name="text">Source file text</param>
/// <param name="source_name">Source file name</param>
void assemble_include(char* text, char* source_name)
{
	source_name_stack.push_back(source_name);
	linenum_stack.push_back(1);

	assemble_text(text);

	source_name_stack.pop_back();
	linenum_stack.pop_back();
}
