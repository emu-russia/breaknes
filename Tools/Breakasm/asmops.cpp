/*
	Behaviour:
	----------

	Implied: just emit

	ORG: just set origin in range 0...0xFFFF

	INCLUDE: calls assemble_include from the main module.
	The current source file name and line numbers are stored on the stack (INCLUDE can be nested)

	DEFINE: add new define if not exist or replace existing define

	ABS: hint directive. Tells the assembler that the NEXT instruction with a Zero Page / Absolute
	ambiguity must use the absolute (or absolute indexed) opcode. See testall.asm for examples.

	BYTE, WORD:
		- split on parameters, separated by comma
		- determine type of parameter: define -> immediate, string, label
		- emit one by one

	Others are handled by a single unified table-driven handler (op_std):
		- split operands on parameters
		- evaluate each parameter with the expression engine (eval_expr)
		- emit opcode variations according to parameter types and the addressing-mode table

	Jumps and branches are emitted with 0 offset, after first pass
	(since not all labels are yet defined).
	The assembler performs multiple passes until all labels are resolved, then
	the final pass "patches" jump/branch offsets.
*/

// Typical errors

#include "pch.h"

static void NotEnoughParameters (char *cmd)
{
	ERR ( "ERROR(%s,%i): %s not enough parameters\n", get_source_name().c_str(), get_linenum(), cmd);
	errors++;
}

static void WrongParameters (char *cmd, char *op)
{
	ERR ( "ERROR(%s,%i): %s wrong parameters: %s\n", get_source_name().c_str(), get_linenum(), cmd, op);
	errors++;
}

// ****************************************************************
// Unified instruction table
// ****************************************************************

// Describes all addressing modes of a 6502 instruction.
// A zero opcode means that the given addressing mode is not available for the instruction.
struct instr_s {
	const char* name;
	uint8_t op_imp;		// implied / accumulator (ASL A)
	uint8_t op_imm;		// #immediate
	uint8_t op_zpg;		// zero page
	uint8_t op_zpgx;	// zero page, X
	uint8_t op_zpgy;	// zero page, Y
	uint8_t op_indx;	// (indirect, X)	[syntax: X, addr]
	uint8_t op_indy;	// (indirect), Y	[syntax: addr, Y with addr < $100]
	uint8_t op_abs;		// absolute
	uint8_t op_absx;	// absolute, X
	uint8_t op_absy;	// absolute, Y
	uint8_t op_ind;		// (indirect)
};

static const instr_s instab[] = {

	// Load / Store
	{ "LDA", 0,   0xA9, 0xA5, 0xB5, 0,   0xA1, 0xB1, 0xAD, 0xBD, 0xB9, 0 },
	{ "STA", 0,   0,    0x85, 0x95, 0,   0x81, 0x91, 0x8D, 0x9D, 0x99, 0 },
	{ "LDX", 0,   0xA2, 0xA6, 0,    0xB6, 0,    0,    0xAE, 0,    0xBE, 0 },
	{ "STX", 0,   0,    0x86, 0,    0x96, 0,    0,    0x8E, 0,    0,    0 },
	{ "LDY", 0,   0xA0, 0xA4, 0xB4, 0,   0,    0,    0xAC, 0xBC, 0,    0 },
	{ "STY", 0,   0,    0x84, 0x94, 0,   0,    0,    0x8C, 0,    0,    0 },

	// ALU
	{ "ORA", 0,   0x09, 0x05, 0x15, 0,   0x01, 0x11, 0x0D, 0x1D, 0x19, 0 },
	{ "AND", 0,   0x29, 0x25, 0x35, 0,   0x21, 0x31, 0x2D, 0x3D, 0x39, 0 },
	{ "EOR", 0,   0x49, 0x45, 0x55, 0,   0x41, 0x51, 0x4D, 0x5D, 0x59, 0 },
	{ "ADC", 0,   0x69, 0x65, 0x75, 0,   0x61, 0x71, 0x6D, 0x7D, 0x79, 0 },
	{ "CMP", 0,   0xC9, 0xC5, 0xD5, 0,   0xC1, 0xD1, 0xCD, 0xDD, 0xD9, 0 },
	{ "SBC", 0,   0xE9, 0xE5, 0xF5, 0,   0xE1, 0xF1, 0xED, 0xFD, 0xF9, 0 },

	// Shifts / rotates
	{ "ASL", 0x0A, 0,   0x06, 0x16, 0,   0,    0,    0x0E, 0x1E, 0,    0 },
	{ "ROL", 0x2A, 0,   0x26, 0x36, 0,   0,    0,    0x2E, 0x3E, 0,    0 },
	{ "LSR", 0x4A, 0,   0x46, 0x56, 0,   0,    0,    0x4E, 0x5E, 0,    0 },
	{ "ROR", 0x6A, 0,   0x66, 0x76, 0,   0,    0,    0x6E, 0x7E, 0,    0 },

	// Other memory instructions
	{ "BIT", 0,   0,    0x24, 0,    0,   0,    0,    0x2C, 0,    0,    0 },
	{ "INC", 0,   0,    0xE6, 0xF6, 0,   0,    0,    0xEE, 0xFE, 0,    0 },
	{ "DEC", 0,   0,    0xC6, 0xD6, 0,   0,    0,    0xCE, 0xDE, 0,    0 },
	{ "CPX", 0,   0xE0, 0xE4, 0,    0,   0,    0,    0xEC, 0,    0,    0 },
	{ "CPY", 0,   0xC0, 0xC4, 0,    0,   0,    0,    0xCC, 0,    0,    0 },

	// Jumps
	{ "JMP", 0,   0,    0,    0,    0,   0,    0,    0x4C, 0,    0,    0x6C },
	{ "JSR", 0,   0,    0,    0,    0,   0,    0,    0x20, 0,    0,    0 },

	// Implied
	{ "BRK", 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "RTI", 0x40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "RTS", 0x60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "PHP", 0x08, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "CLC", 0x18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "PLP", 0x28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "SEC", 0x38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "PHA", 0x48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "CLI", 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "PLA", 0x68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "SEI", 0x78, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "DEY", 0x88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "TYA", 0x98, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "TAY", 0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "CLV", 0xB8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "INY", 0xC8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "CLD", 0xD8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "INX", 0xE8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "SED", 0xF8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "TXA", 0x8A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "TXS", 0x9A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "TAX", 0xAA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "TSX", 0xBA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "DEX", 0xCA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "NOP", 0xEA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

	{ NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static const instr_s* find_instr(const char* cmd)
{
	for (const instr_s* ins = instab; ins->name; ins++) {
		if (!_stricmp(ins->name, cmd)) return ins;
	}
	return NULL;
}

// True if the instruction has only the implied addressing mode (BRK is 0x00, so the opcode
// itself cannot be used to detect the mode presence).
static int is_implied_only(const instr_s* ins)
{
	return ins->op_imm == 0 && ins->op_zpg == 0 && ins->op_zpgx == 0 && ins->op_zpgy == 0 &&
		ins->op_indx == 0 && ins->op_indy == 0 && ins->op_abs == 0 && ins->op_absx == 0 &&
		ins->op_absy == 0 && ins->op_ind == 0;
}

// ****************************************************************
// Expression helpers
// ****************************************************************

// True if the evaluated operand is the register (X or Y keyword).
static int is_reg(eval_t* v, const char* reg)
{
	return v->type == EVAL_LABEL && v->label->orig == KEYWORD && !_stricmp(v->label->name, reg);
}

// Get the numeric address value of an evaluated operand.
// Returns false if the value is not known yet.
// When the value is taken from the previous assembly pass (a forward reference),
// *provisional is set to true: the operand must be emitted with a patch so that
// the final value of the current pass is written into the PRG.
static bool addr_value(eval_t* v, int type, long* out, bool* provisional)
{
	*provisional = false;
	if (type == EVAL_ADDRESS) {
		*out = v->address;
		return true;
	}
	if (type == EVAL_NUMBER) {
		// A plain number in a memory context is treated as an address (legacy behavior).
		*out = v->number;
		return true;
	}
	if (type == EVAL_LABEL) {
		if (v->label->orig == KEYWORD)
			return false;   // a register name (X/Y) is not an address value
		if (v->label->composite) {
			// Composite expression: evaluate it right now with the expression engine.
			bool resolved = false;
			long val = eval_expr(v->label->name, false, true, &resolved);
			if (resolved) {
				*out = val;
				return true;
			}
			// Fallback to the value of the previous pass (if any).
			if (prev_label_value(v->label->name, out)) {
				*provisional = true;
				return true;
			}
			return false;
		}
		if (v->label->orig != UNDEF) {
			*out = v->label->orig;
			return true;
		}
		// Forward reference: use the value of the previous pass to choose the
		// addressing mode, then patch the operand with the final value.
		if (prev_label_value(v->label->name, out)) {
			*provisional = true;
			return true;
		}
		return false;
	}
	return false;
}

// Emit the operand bytes for a memory access, patching them if the value is provisional.
static void emit_operand(long addr, bool provisional, label_s* label, int num_bytes, char* cmd, char* ops)
{
	if (provisional) {
		add_patch(label, org, num_bytes == 1 ? 2 : 0);
	}
	for (int i = 0; i < num_bytes; i++) {
		if (provisional)
			emit(0);
		else
			emit((addr >> (8 * i)) & 0xff);
	}
}

// Emit a memory operand with a known address value (zero page / absolute choice).
static void emit_addr(const instr_s* ins, long addr, int indirect, bool provisional, label_s* label, char* cmd, char* ops)
{
	if (indirect) {
		if (!ins->op_ind) {
			WrongParameters(cmd, ops);
			return;
		}
		emit(ins->op_ind);
		emit_operand(addr, provisional, label, 2, cmd, ops);
		return;
	}
	if (addr < 0x100 && !force_abs && ins->op_zpg) {   // Zero page
		emit(ins->op_zpg);
		emit_operand(addr, provisional, label, 1, cmd, ops);
	}
	else if (ins->op_abs) {                             // Absolute
		emit(ins->op_abs);
		emit_operand(addr, provisional, label, 2, cmd, ops);
	}
	else {
		WrongParameters(cmd, ops);
	}
}

// Emit an absolute (or indirect) operand for an unresolved label - it will be patched later.
static void emit_patch_addr(const instr_s* ins, label_s* label, int indirect, char* cmd, char* ops)
{
	uint8_t opcode = indirect ? ins->op_ind : ins->op_abs;
	if (!opcode) {
		WrongParameters(cmd, ops);
		return;
	}
	emit(opcode);
	add_patch(label, org, 0);
	emit(0);
	emit(0);
}

// addr, X
static void emit_addr_x(const instr_s* ins, long addr, bool provisional, label_s* label, char* cmd, char* ops)
{
	if (addr < 0x100 && !force_abs && ins->op_zpgx) {   // Zero page, X
		emit(ins->op_zpgx);
		emit_operand(addr, provisional, label, 1, cmd, ops);
	}
	else if (ins->op_absx) {                            // Absolute, X
		emit(ins->op_absx);
		emit_operand(addr, provisional, label, 2, cmd, ops);
	}
	else {
		WrongParameters(cmd, ops);
	}
}

static void emit_patch_x(const instr_s* ins, label_s* label, char* cmd, char* ops)
{
	if (!ins->op_absx) {
		WrongParameters(cmd, ops);
		return;
	}
	emit(ins->op_absx);
	add_patch(label, org, 0);
	emit(0);
	emit(0);
}

// addr, Y
static void emit_addr_y(const instr_s* ins, long addr, bool provisional, label_s* label, char* cmd, char* ops)
{
	if (addr < 0x100 && !force_abs) {                   // (indirect), Y or zero page, Y
		if (ins->op_indy) {
			emit(ins->op_indy);
			emit_operand(addr, provisional, label, 1, cmd, ops);
			return;
		}
		if (ins->op_zpgy) {
			emit(ins->op_zpgy);
			emit_operand(addr, provisional, label, 1, cmd, ops);
			return;
		}
	}
	if (ins->op_absy) {                                 // Absolute, Y
		emit(ins->op_absy);
		emit_operand(addr, provisional, label, 2, cmd, ops);
	}
	else {
		WrongParameters(cmd, ops);
	}
}

static void emit_patch_y(const instr_s* ins, label_s* label, char* cmd, char* ops)
{
	if (!ins->op_absy) {
		WrongParameters(cmd, ops);
		return;
	}
	emit(ins->op_absy);
	add_patch(label, org, 0);
	emit(0);
	emit(0);
}

// X, addr  -> (indirect, X)
static void emit_indx(const instr_s* ins, long addr, bool provisional, label_s* label, char* cmd, char* ops)
{
	if (!ins->op_indx) {
		WrongParameters(cmd, ops);
		return;
	}
	emit(ins->op_indx);
	emit_operand(addr, provisional, label, 1, cmd, ops);
}

// X, label -> (indirect, X) with a one-byte zero page patch
static void emit_patch_indx(const instr_s* ins, label_s* label, char* cmd, char* ops)
{
	if (!ins->op_indx) {
		WrongParameters(cmd, ops);
		return;
	}
	emit(ins->op_indx);
	add_patch(label, org, 2);
	emit(0);
}

// ****************************************************************
// Unified instruction handler
// ****************************************************************

void op_std(char* cmd, char* ops)
{
	const instr_s* ins = find_instr(cmd);
	if (!ins) {
		WrongParameters(cmd, ops);
		return;
	}

	eval_t val[2];
	int type[2] = { EVAL_WTF, EVAL_WTF };
	bool provisional;

	split_param(ops);

	// Implied / accumulator
	if (param_num == 0) {
		if (ins->op_imp || is_implied_only(ins)) emit(ins->op_imp);
		else NotEnoughParameters(cmd);
		return;
	}

	type[0] = eval(params[0].string, &val[0]);

	// One operand
	if (param_num == 1) {
		switch (type[0]) {
			case EVAL_NUMBER:   // #immediate
				if (ins->op_imm) {
					emit(ins->op_imm);
					if (val[0].label) {      // forward-referenced immediate: patch later
						add_patch(val[0].label, org, 2);
						emit(0);
					}
					else emit(val[0].number & 0xff);
				}
				else WrongParameters(cmd, ops);
				return;
			case EVAL_STRING:
				WrongParameters(cmd, ops);
				return;
			case EVAL_ADDRESS:  // zero page / absolute / (indirect)
				emit_addr(ins, val[0].address, val[0].indirect, false, NULL, cmd, ops);
				return;
			case EVAL_LABEL:
				if (val[0].label->orig == KEYWORD) {    // accumulator form: ASL A
					if (!_stricmp(val[0].label->name, "A") && ins->op_imp)
						emit(ins->op_imp);
					else WrongParameters(cmd, ops);
				}
				else {
					long addr;
					if (addr_value(&val[0], type[0], &addr, &provisional))
						emit_addr(ins, addr, val[0].indirect, provisional, val[0].label, cmd, ops);
					else
						emit_patch_addr(ins, val[0].label, val[0].indirect, cmd, ops);
				}
				return;
		}
		WrongParameters(cmd, ops);
		return;
	}

	// Two operands
	if (param_num == 2) {
		type[1] = eval(params[1].string, &val[1]);

		// X, addr  -> (indirect, X)
		if (is_reg(&val[0], "X")) {
			long addr;
			if (addr_value(&val[1], type[1], &addr, &provisional))
				emit_indx(ins, addr, provisional, val[1].label, cmd, ops);
			else if (type[1] == EVAL_LABEL && val[1].label->orig != KEYWORD)
				emit_patch_indx(ins, val[1].label, cmd, ops);
			else WrongParameters(cmd, ops);
			return;
		}

		// addr, X
		if (is_reg(&val[1], "X")) {
			long addr;
			if (addr_value(&val[0], type[0], &addr, &provisional))
				emit_addr_x(ins, addr, provisional, val[0].label, cmd, ops);
			else if (type[0] == EVAL_LABEL && val[0].label->orig != KEYWORD)
				emit_patch_x(ins, val[0].label, cmd, ops);
			else WrongParameters(cmd, ops);
			return;
		}

		// addr, Y
		if (is_reg(&val[1], "Y")) {
			long addr;
			if (addr_value(&val[0], type[0], &addr, &provisional))
				emit_addr_y(ins, addr, provisional, val[0].label, cmd, ops);
			else if (type[0] == EVAL_LABEL && val[0].label->orig != KEYWORD)
				emit_patch_y(ins, val[0].label, cmd, ops);
			else WrongParameters(cmd, ops);
			return;
		}

		WrongParameters(cmd, ops);
		return;
	}

	NotEnoughParameters(cmd);
}

// Branches
// **************************************************************

void opBRA (char *cmd, char *ops)
{
	label_s * label;
	int type;
	eval_t val;
	split_param (ops);

	if (param_num >= 1) {
		type = eval ( params[0].string, &val );
		if (type == EVAL_LABEL) {
			if ( !_stricmp (cmd, "BPL") ) emit (0x10);
			if ( !_stricmp (cmd, "BMI") ) emit (0x30);
			if ( !_stricmp (cmd, "BVC") ) emit (0x50);
			if ( !_stricmp (cmd, "BVS") ) emit (0x70);
			if ( !_stricmp (cmd, "BCC") ) emit (0x90);
			if ( !_stricmp (cmd, "BCS") ) emit (0xB0);
			if ( !_stricmp (cmd, "BNE") ) emit (0xD0);
			if ( !_stricmp (cmd, "BEQ") ) emit (0xF0);
			label = add_label (val.label->name, UNDEF);
			add_patch (label, org, 1 );
			emit (0);
		}
		else WrongParameters (cmd, ops);
	}
	else NotEnoughParameters (cmd);
}

// ABS directive
// **************************************************************

void opABS(char* cmd, char* ops)
{
	force_abs = 1;
}

// INCLUDE
// **************************************************************

void opINCLUDE(char* cmd, char* ops)
{
	while (*ops <= ' ' && *ops) ops++;
	
	// Construct a file name enclosed in double or single quotes

	char source_name[0x100]{}, *ptr = source_name;
	while (*ops) {
		if (*ops == '\"' || *ops == '\'') {
			ops++;
			continue;
		}
		*ptr++ = *ops++;
	}
	*ptr++ = 0;

	FILE* f;
	f = fopen(source_name, "rt");
	if (!f) {
		ERR("ERROR(%s,%i): Failed to load the nested source file: %s\n", get_source_name().c_str(), get_linenum(), source_name);
		errors++;
		return;
	}

	// One more byte of memory is allocated to complete the text with the null character (END).

	fseek(f, 0, SEEK_END);
	long size = ftell(f) + 1;
	fseek(f, 0, SEEK_SET);

	char* text = new char[size];
	memset(text, 0, size);

	size_t readSize = fread(text, 1, size, f);
	fclose(f);
	if (readSize >= size)
	{
		delete[] text;
		ERR("ERROR(%s,%i): Error loading the source file.\n", get_source_name().c_str(), get_linenum());
		errors++;
		return;
	}
	
	assemble_include(text, source_name);
	delete[] text;
}

// DEFINE
// **************************************************************

void opDEFINE (char *cmd, char *ops)
{
	char name[256], *p = name;
	while (*ops > ' ' && *ops ) *p++ = *ops++;
	*p++ = 0;
	while (*ops <= ' ' && *ops ) ops++;
	add_define (name, ops);
}

// BYTE, WORD
// **************************************************************

void opBYTE (char *cmd, char *ops)
{
	int i, type, len, c;
	eval_t val;
	split_param (ops);

	for (i=0; i<param_num; i++) {
		type = eval ( params[i].string, &val );
		if ( type == EVAL_LABEL ) {
			ERR ( "ERROR(%s,%i): Label cannot be used here\n", get_source_name().c_str(), get_linenum());
			errors++;
		}
		else if ( type == EVAL_NUMBER ) {
			if ( val.label ) {		// forward-referenced immediate (#label): patch later
				add_patch (val.label, org, 2 );
				emit (0);
			}
			else emit ( val.number & 0xff );
		}
		else if ( type == EVAL_ADDRESS ) {
			emit ( val.address & 0xff );
		}
		else if ( type == EVAL_STRING ) {
			len = (int)strlen ( val.string );
			for ( c=0; c<len; c++) emit ( val.string[c] );
		}
	}
}

void opWORD (char *cmd, char *ops)
{
	label_s *label;
	int i, type;
	eval_t val;
	split_param (ops);

	for (i=0; i<param_num; i++) {
		type = eval ( params[i].string, &val );
		if ( type == EVAL_STRING ) {
			ERR ( "ERROR(%s,%i): String cannot be used here\n", get_source_name().c_str(), get_linenum());
			errors++;
		}
		else if ( type == EVAL_NUMBER ) {
			if ( val.label ) {		// forward-referenced immediate (#label): patch later
				add_patch (val.label, org, 0 );
				emit (0); emit (0);
			}
			else {
				emit ( val.number & 0xff );
				emit ( (val.number >> 8) & 0xff );
			}
		}
		else if ( type == EVAL_ADDRESS ) {
			emit ( val.address & 0xff );
			emit ( (val.address >> 8) & 0xff );
		}
		else if ( type == EVAL_LABEL ) {
			label = add_label (val.label->name, UNDEF);
			add_patch (label, org, 0 );
			emit (0); emit (0);
		}
		else WrongParameters(cmd, ops);
	}
}


// Misc.
// **************************************************************

void opORG (char *cmd, char *ops)
{
	int type;
	eval_t val;
	long addr;
	bool provisional;

	split_param (ops);

	if (param_num == 1) {
		type = eval ( params[0].string, &val );
		if ( type == EVAL_ADDRESS ) org = val.address & 0xffff;
		else if ( type == EVAL_NUMBER ) org = val.number & 0xffff;
		else if ( type == EVAL_LABEL && addr_value(&val, type, &addr, &provisional) ) org = addr & 0xffff;
		else WrongParameters (cmd, ops);
	}
	else NotEnoughParameters (cmd);
}

void opEND (char *cmd, char *ops)
{
	stop = 1;
}

void opDUMMY (char *cmd, char *ops) {}
