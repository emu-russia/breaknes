// Parser of complex expressions using syntax tree

// Parses a stream of tokens into a syntax tree. The nodes of the tree can be either expressions or operations.
// Expressions are identifiers, numbers or strings.
// Operations are special commands that control the "growth" of the tree.
// The branches of a tree can grow recursively, in the form of subtrees.

#pragma once

/// <summary>
/// Operations for composite expressions. Used in the eval_expr method.
/// </summary>
enum class OPS
{
	NOP = 0,
	LPAREN, RPAREN,   // ( )
	PLUS, MINUS,	// + -
	NOT, NEG,     // ! ~
	MUL, DIV, MOD,     // * / %
	SHL, SHR, ROTL, ROTR,   // << >> <<< >>>
	GREATER, GREATER_EQ, LESS, LESS_EQ,     // > >= < <=
	LOGICAL_EQ, LOGICAL_NOTEQ,	// == !=
	AND, OR, XOR, // & | ^
};

struct token_t {
	int		type;			// Token type, overused definition EVAL_xxx; EVAL_LABEL in this case means that the token is an identifier
	OPS		op;				// Type of operation (see OPS)
	long	number;			// Converted number (EVAL_NUMBER)
	char	string[0x100];	// String representation for EVAL_LABEL and EVAL_STRING
};

struct node_t
{
	node_t* lvalue;
	node_t* rvalue;
	token_t* token;
	int     depth;
};

struct tree_t
{
	std::list<node_t*> nodes;
	node_t* curr;
	node_t* node;
	int depth;
	int prio;
	int prio_stack[1000];
	int initialized;
};

/// <summary>
/// Calculate the expression and return a number. Possible errors are displayed in the process (if there is an error, return 0).
/// </summary>
/// <param name="text">The text of the expression, for example: MyData + 32 * entry_size + 12. Other labels and defines can be identifiers</param>
/// <returns></returns>
long eval_expr(char* text);
