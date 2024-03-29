#pragma once

#define BREAKASM_VERSION "1.3"

#define UNDEF   0xbabadaba  // undefined offset
#define KEYWORD 0xd0d0d0d0  // keyword

struct oplink {
	const char* name;
	void    (*handler) (char* cmd, char* ops);
};

struct line {
	char    label[256];
	char    cmd[256];
	char    op[1024];
};

struct label_s {
	char    name[128];
	long    orig;
	char	source[0x100];	// Name of the source file where the label was declared
	int     line;		// Line number in the source file
	int		composite;	// 1: A composite expression that uses the eval_expr method to parse it
};

struct patch_s {
	label_s* label;
	long    orig;
	int     branch;     // 1: relative branch, 0: absolute jmp
	char	source[0x100];	// Name of the source file where the patch field was encountered
	int     line;		// Line number in the source file
};

struct define_s {
	char    name[256];
	char    replace[256];
};


#define EVAL_WTF        0
#define EVAL_NUMBER     1       // #$12
#define EVAL_ADDRESS    2       // $aabb
#define EVAL_LABEL      3       // BEGIN + including complex expressions that are saved as Label, but the expression is evaluated on the second pass
#define EVAL_STRING     4       // "Hello", 'Hello'

struct eval_t {
	int     type;
	long    number;
	long    address;
	char    string[256];
	label_s* label;
	int     indirect;		// If the expression is enclosed in outer parentheses - such operand is considered as Indirect addressing
};

struct param_t {
	char    string[1024];
};

extern long org;        // current emit offset
extern long stop;
extern long errors;

extern param_t* params;
extern int param_num;

label_s* add_label(const char* name, long orig);
label_s* label_lookup(char* name);
void add_patch(label_s* label, long orig, int branch);
define_s* add_define(char* name, char* replace);
define_s* define_lookup(char* name);
int eval(char* text, eval_t* result);
void split_param(char* op);
void emit(uint8_t b);

int get_linenum();
std::string get_source_name();

int assemble (char *text, char *source_name, uint8_t *prg);
void assemble_include(char* text, char* source_name);
