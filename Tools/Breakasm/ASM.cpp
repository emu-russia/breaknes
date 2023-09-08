// 6502 assembler.
// Note for experienced developers: the code was written when I was still a student and didn't know about Dragon Books or regular expressions.
// That's why the code looks naive, but that's even good. New developers will learn it faster and take the ideas into circulation.
#include "pch.h"

/*
	Assembler syntax:

	[LABEL:]  COMMAND  [OPERAND1, OPERAND2, OPERAND3]       ; Comments

	Commands can be any 6502 instruction or one of reserved directives:
		ORG, INCLUDE, DEFINE, BYTE, WORD, END, PROCESSOR

	Register names and CPU instructions cannot be used as label names.

	You cannot DEFINE, if such label is already defined.
	Redefinition of labels is NOT allowed.
	Redefinition of DEFINEs just replace previous definition.
*/

static uint8_t *PRG;
long org;        // current emit offset
long stop;
long errors;

// Previously, instead of list, we used a ordinary array of large size (limited) + a variable that stores the number of records.
// Now it is essentially the same, but instead of an array it is a list of virtually unlimited size (as long as memory is available)

static  std::list<label_s *> labels;    // name labels
static  std::list<patch_s *> patches;   // patch history
static  std::list<define_s *> defines;  // definitions
static  std::list<node_t*> tree;	// syntax tree

static  std::list<std::string> source_name_stack;	// Stack for the name of the current source file (for INCLUDE)
static  std::list<int> linenum_stack;		// Stack for the line number of the current source file (for INCLUDE)

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

static label_s * label_lookup (char *name)
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
			printf ("ERROR(%s,%i): Reserved keyword used as label \'%s\'\n", get_source_name().c_str(), get_linenum(), temp_name);
			errors++;
		}
		else if (orig != UNDEF && label->orig != UNDEF ) {
			printf ("ERROR(%s,%i): label \'%s\' already defined in %s, line %i\n", get_source_name().c_str(), get_linenum(), temp_name, label->source, label->line);
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
			printf ("ERROR(%s,%i): Undefined label \'%s\' (%08X)\n", patch->source, patch->line, patch->label->name, orig );
			errors++;
		}
		else { 
			if ( patch->branch != 0) {
				org = patch->orig;
				rel = orig - org - 1;
				if (rel > 127 || rel < -128) {
					printf("ERROR(%s,%i): Branch relative offset to %s out of range\n", patch->source, patch->line, patch->label->name);
					errors++;
				}
				else emit(rel & 0xff);
			}
			else {
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
		if (patch->branch) printf(" (REL)\n");
		else printf(" (ABS)\n");
	}
}

// Defines management.
//

static define_s * define_lookup (char *name)
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
		printf ("ERROR(%s,%i): Already defined as label in %s, line %i\n", get_source_name().c_str(), get_linenum(), label->source, label->line );
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

int eval (char *text, eval_t *result)
{
	char buf[1024]{}, * p = buf, c, quot = 0, * ptr;
	int type = EVAL_WTF, i, len;
	define_s * def;
	label_s * label;
	int composite_expr = 0;

	// Indirect test
	result->indirect = 0;
	len = (int)strlen (text);
	for (i=0; i<len; i++) {
		c = text[i];
		if ( (c == '(' || c == ')') && quot == 0 ) {
			text[i] = ' ';
			result->indirect = 1;
		}
		else if ( c == '\"' || c == '\'' ) {
			if ( c == quot ) quot = 0;
			else quot = c;
		}
	}
	for (i=len-1; i>=0; i--) {
		if ( text[i] <= ' ' ) text[i] = 0;
		else break;
	}
	ptr = text;
	while (*ptr <= ' ' && *ptr) ptr++;      // Skip whitespaces
	text = ptr;

	if ( text[0] == '#' || isdigit(text[0]) ) {      // Number
		if (!isdigit(text[0])) text++;
		while (1) {
			c = *text++;
			if (c == 0) break;
			if (c == '$') {
				*p++ = '0';
				*p++ = 'x';
			}
			else *p++ = c;
		}
		*p++ = 0;
		type = EVAL_NUMBER;
	}
	else if ( text[0] == '$' ) {    // Address
		text++;
		*p++ = '0';
		*p++ = 'x';
		while (1) {
			c = *text++;
			if (c == 0) break;
			else *p++ = c;
		}
		*p++ = 0;
		type = EVAL_ADDRESS;
	}
	else if ( text[0] == '\'' || text[0] == '\"' ) {    // String
		quot = text[0];
		text++;
		while (1) {
			c = *text++;
			if (c == 0 || c == quot) break;
			else *p++ = c;
		}
		*p++ = 0;
		type = EVAL_STRING;
	}
	else {          // Label azAZ09_  + composite expressions are also saved as a "label"
		while (1) {
			c = *text++;
			if (c == 0) break;
			if (c >= 'a' && c <= 'z') *p++ = c;
			else if (c >= 'A' && c <= 'Z') *p++ = c;
			else if (c >= '0' && c <= '9') *p++ = c;
			else if (c == '_' ) *p++ = c;
			else {
				// If we encounter some symbol NOT of the alphabet, we will assume that the label is a composite expression
				if (c > ' ')
					composite_expr = 1;
				*p++ = c;
			}
		}
		*p++ = 0;
		type = EVAL_LABEL;
	}

	switch (type) {
		case EVAL_NUMBER:
			result->number = strtoul ( buf, NULL, 0);
			break;
		case EVAL_ADDRESS:
			result->address = strtoul ( buf, NULL, 0);
			break;
		case EVAL_LABEL:
			def = define_lookup (buf);
			if (def) return eval (def->replace, result);
			else {
				label = label_lookup (buf);
				if (label) result->label = label;
				else {
					result->label = add_label(buf, UNDEF);
					result->label->composite = composite_expr;
				}
			}
			break;
		case EVAL_STRING:
			strncpy (result->string, buf, 255);
			break;
	}

	return result->type = type;
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

// The process of lexical analysis is classical: from a stream of characters (chars) a stream of tokens (token_t) is made. Then the list of obtained tokens is processed by the expression parser.

static char get_char(char** pp)
{
	char* ptr = *pp;
	char ch = *ptr++;
	*pp = ptr;
	return ch;
}

static void put_back_char(char** pp)
{
	char* ptr = *pp;
	*pp = --ptr;
}

static token_t* create_op_token(int optype)
{
	token_t* token = new token_t;
	token->type = EVAL_OP;
	token->op = optype;
	return token;
}

static token_t* next_token(char** pp)
{
	token_t* token = nullptr;
	char buf[0x100]{}, *ptr;
	int base = 10;
	bool quot;

	while (1) {

		char ch = get_char(pp);
		if (ch == 0)
			break;
		if (ch <= ' ')
			continue;

		if (ch == '#' || ch == '$' || isdigit(ch)) {	// Number
			ptr = buf;
			if (ch == '$')
				base = 16;
			else
				base = 10;
			if (isdigit(ch))
				*ptr++ = ch;
			while (1) {
				ch = get_char(pp);
				if (ch == '#') {
				}
				else if (ch == '$') {
					base = 16;
				}
				else if (isxdigit(ch)) {
					*ptr++ = ch;
				}
				else {
					put_back_char(pp);
					break;
				}
			}
			*ptr++ = 0;
			//printf("number: %s\n", buf);
			token = new token_t;
			token->type = EVAL_NUMBER;
			token->number = strtoul(buf, nullptr, base);
			return token;
		}
		else if (ch == '_' || isalpha(ch)) {	// Identifier [_a-zA-Z][_a-zA-Z0-9]
			ptr = buf;
			*ptr++ = ch;
			while (1) {
				ch = get_char(pp);
				if (ch == '_' || isalpha(ch) || isdigit(ch)) {
					*ptr++ = ch;
				}
				else {
					put_back_char(pp);
					break;
				}
			}
			*ptr++ = 0;
			//printf("ident: %s\n", buf);
			token = new token_t;
			token->type = EVAL_LABEL;
			strcpy(token->string, buf);
			return token;
		}
		else if (ch == '\"' || ch == '\'') {	// "String" 'String'
			ptr = buf;
			quot = ch == '\'';
			while (1) {
				ch = get_char(pp);
				if (ch == '\'' || ch == '\"') {
					if (quot) {
						if (ch == '\'')
							break;
						else
							*ptr++ = ch;
					}
					else {
						if (ch == '\"')
							break;
						else
							*ptr++ = ch;
					}
				}
				else if (ch == 0) {
					break;
				}
				else {
					*ptr++ = ch;
				}
			}
			*ptr++ = 0;
			printf("string: %s\n", buf);
			token = new token_t;
			token->type = EVAL_STRING;
			strcpy(token->string, buf);
			return token;
		}

		// Operations
		else if (ch == '(') {
			token = create_op_token(LPAREN);
			break;
		}
		else if (ch == ')') {
			token = create_op_token(RPAREN);
			break;
		}
		else if (ch == '+') {
			token = create_op_token(PLUS);
			break;
		}
		else if (ch == '-') {
			token = create_op_token(MINUS);
			break;
		}
		else if (ch == '!') {	// ! !=
			char ch2 = get_char(pp);
			if (ch2 == '=') {
				token = create_op_token(LOGICAL_NOTEQ);
				break;
			}
			else {
				put_back_char(pp);
				token = create_op_token(NOT);
				break;
			}
		}
		else if (ch == '~') {
			token = create_op_token(NEG);
			break;
		}
		else if (ch == '*') {
			token = create_op_token(MUL);
			break;
		}
		else if (ch == '/') {
			token = create_op_token(DIV);
			break;
		}
		else if (ch == '%') {
			token = create_op_token(MOD);
			break;
		}
		else if (ch == '&') {
			token = create_op_token(AND);
			break;
		}
		else if (ch == '|') {
			token = create_op_token(OR);
			break;
		}
		else if (ch == '^') {
			token = create_op_token(XOR);
			break;
		}

		else if (ch == '>') {	// > >= >> >>>
			char ch2 = get_char(pp);
			if (ch2 == '=') {
				token = create_op_token(GREATER_EQ);
				break;
			}
			else if (ch2 == '>') {
				char ch3 = get_char(pp);
				if (ch3 == '>') {
					token = create_op_token(ROTR);
					break;
				}
				else {
					put_back_char(pp);
					token = create_op_token(SHR);
					break;
				}
			}
			else {
				put_back_char(pp);
				token = create_op_token(GREATER);
				break;
			}
		}
		else if (ch == '<') {	// < <= << <<<
			char ch2 = get_char(pp);
			if (ch2 == '=') {
				token = create_op_token(LESS_EQ);
				break;
			}
			else if (ch2 == '<') {
				char ch3 = get_char(pp);
				if (ch3 == '<') {
					token = create_op_token(ROTL);
					break;
				}
				else {
					put_back_char(pp);
					token = create_op_token(SHL);
					break;
				}
			}
			else {
				put_back_char(pp);
				token = create_op_token(LESS);
				break;
			}
		}
		else if (ch == '=') {	// ==

			char ch2 = get_char(pp);
			if (ch2 == '=') {
				token = create_op_token(LOGICAL_EQ);
				break;
			}
			else {
				put_back_char(pp);
			}
		}

		printf("ERROR(%s,%i): unknown character in string: %c\n", get_source_name().c_str(), get_linenum(), ch);
		errors++;
		break;
	}

	return token;
}

static void tokenize(char* text, std::list<token_t*>& tokens)
{
	token_t* next = nullptr;
	char* pp = text;
	do {
		next = next_token(&pp);
		if (next != nullptr)
			tokens.push_back(next);
	} while (next != nullptr);
}

static const char* opstr(int type)
{
	const char* str[] = {
		"NOP", "(", ")", "+", "-", "!", "~", "*", "/", "%",
		"<<", ">>", "<<<", ">>>", ">", ">=", "<", "<=", 
		"==", "!=", "&", "|", "^",
	};
	return str[type];
}

static void dump_tokens(std::list<token_t*>& tokens)
{
	for (auto it = tokens.begin(); it != tokens.end(); ++it) {

		token_t* token = *it;
		switch (token->type)
		{
			case EVAL_NUMBER:
				printf("EVAL_NUMBER: %d (0x%08X)\n", token->number, token->number);
				break;
			case EVAL_LABEL:
				printf("EVAL_IDENT: %s\n", token->string);
				break;
			case EVAL_STRING:
				printf("EVAL_STRING: %s\n", token->string);
				break;
			case EVAL_OP:
				printf("EVAL_OP: %s\n", opstr(token->op));
				break;
		}
	}
}

// priorities of operations (1: low -> high)
static int opprio[] = {
	1,			// nop
	12, 12,		// ( )
	9,			// +
	9,			// -
	11, 11,     // ! ~
	10, 10, 10,	// * / %
	8, 8, 8, 8,	// << >> <<< >>>
	7, 7, 7, 7,	// > >= < <=
	7, 7,		// == !=
	6, 4, 5,	// & | ^
};

static int isunary (int op)
{
    switch (op)
    {
        case NOT:
        case NEG:
            return 1;
    }
    return 0;
}

static int isbinary (int op)
{
    switch (op)
    {
        case PLUS:
        case MINUS:
        case MUL: case DIV: case MOD:
        case SHL: case SHR: case ROTL: case ROTR:
        case GREATER: case GREATER_EQ: case LESS: case LESS_EQ:
        case LOGICAL_EQ: case LOGICAL_NOTEQ:
        case AND: case OR: case XOR:
            return 1;
    }
    return 0;
}

// adding a new branch to the syntax tree
static node_t* addnode(token_t* token, int depth)
{
	node_t* node = new node_t;
	node->lvalue = node->rvalue = NULL;
	node->depth = depth;
	node->token = token;
	tree.push_back(node);
	return node;
}

// executing the tree (semantic analysis)
static node_t * evaluate (node_t * expr, long *lvalue)
{
    node_t * curr;
    long rvalue = 0, mvalue;
    token_t * token;
    int uop = NOP, op = NOP;
    int debug = 0;

    if (debug) printf ( "[" );

    // lvalue = { [uop] <ident|expr> [op] }
    curr = expr;
    while (curr) {

        if (curr->depth < expr->depth) break;

        // optional unary operation
        token = curr->token;
        if ( token->type == EVAL_OP && isunary(token->op) ) {
			if (curr->rvalue == NULL) {
				printf("Missing identifier\n");
				errors++;
			}
            else {
                curr = curr->rvalue;
                uop = token->op;
            }
        }
        else uop = NOP;

        if (uop != NOP && debug) printf ( "%s ", opstr(uop) );

        // mandatory identifier or nested expression
        token = curr->token;
        if ( token->type == EVAL_LABEL || token->type == EVAL_NUMBER || curr->depth > expr->depth ) {

			mvalue = 0;

            if ( curr->depth > expr->depth ) {  // nested expression
                if (debug) printf ( "SUBEVAL " );
                curr = evaluate (curr, &mvalue);
                //printf ( "SUB LVALUE : %i\n", mvalue.num.value );
            }
            else if ( token->type == EVAL_LABEL) {
                curr = curr->rvalue;

				define_s* def = define_lookup(token->string);
				if (def) {
					eval_t define_eval{};
					eval(def->replace, &define_eval);
					if (define_eval.type == EVAL_NUMBER) {
						mvalue = define_eval.number;
					}
					else if (define_eval.type == EVAL_ADDRESS) {
						mvalue = define_eval.address;
					}
					else if (define_eval.type == EVAL_LABEL) {
						mvalue = define_eval.label->orig;
					}
				}
				else {
					label_s* label = label_lookup(token->string);
					if (label) {
						mvalue = label->orig;
					}
					else {
						printf("Undefined identifier: %s", token->string);
					}
				}
                if (debug) printf ( "SYMBOL: %s", token->string );
            }
            else if ( token->type == EVAL_NUMBER) {
                curr = curr->rvalue;
				mvalue = token->number;
                if (debug) printf ( "NUMBER(%i) ", mvalue );
            }

            // perform unary operation on MVALUE
            if (uop != NOP) {
                switch (uop)
                {
                    case NOT:
                        mvalue = !mvalue;
                        break;
                    case NEG:
                        mvalue = ~mvalue;
                        break;
                }
            }

            // perform binary operation RVALUE = RVALUE op MVALUE
            if (op != NOP) {
                switch (op)
                {
                    case MINUS:
                        rvalue -= mvalue;
                        break;
                    case PLUS:
                        rvalue += mvalue;
                        break;
                    case MUL:
                        rvalue *= mvalue;
                        break;
                    case DIV:
                        if (mvalue) rvalue /= mvalue;
                        break;
                    case MOD:
                        if (mvalue) rvalue %= mvalue;
                        break;
                }
            }
            else rvalue = mvalue;
        }
		else {
			printf ("Identifier required\n");
			errors++;
		}

        // optional binary operation
        if (curr && curr->depth == expr->depth ) {
            token = curr->token;
            if ( token->type == EVAL_OP && isbinary(token->op) ) {
                curr = curr->rvalue;
                op = token->op;
            }
            else op = NOP;

            if (op != NOP && debug) printf ( "%s ", opstr(op) );
        }
    }

    if (debug) printf ( "]" );

	*lvalue = rvalue;
    return curr;
}

// grow a syntax tree
static void grow ( node_t **expr, token_t * token)
{
    static node_t * curr, * node;
    static int depth, prio, prio_stack[1000];

    node = addnode ( token, depth );

    if (*expr == NULL)
    {
		curr = node;
        *expr = curr;
        depth = prio = 0;
        memset ( prio_stack, 0, sizeof(prio_stack) );
    }
    else {

		if ( token->type == EVAL_OP) {
			if ( token->op == LPAREN) {
				prio++;
				depth++;
				return;
			}
			else if ( token->op == RPAREN ) {
				prio--;
				if (depth > 0) depth--;
				else printf ( "Unmatched parenthesis" );
				return;
			}
			else if ( opprio[token->op] > prio_stack[prio] ) {  // priority increase. for binary operations, the priority of the previous token is also increased.
				if ( isbinary (token->op) ) curr->depth++;
				node->depth++;
				prio_stack[prio] = opprio[token->op];
				depth++;
			}
			else if ( opprio[token->op] < prio_stack[prio] ) { // de-prioritizing
				prio_stack[prio] = opprio[token->op];
				node->depth--;
				depth--;
			}
		}
		// we just ignore the unclosed parentheses

		curr->rvalue = node;
		node->lvalue = curr;
		curr = node;
    }
}

/// <summary>
/// Calculate the expression and return a number. Possible errors are displayed in the process (if there is an error, return 0).
/// </summary>
/// <param name="text">The text of the expression, for example: MyData + 32 * entry_size + 12. Other labels and defines can be identifiers</param>
/// <returns></returns>
long eval_expr(char* text)
{
	// Obtain token stream

	std::list<token_t*> tokens;
	tokenize(text, tokens);

	// Grow syntax tree

	node_t* root = nullptr;
	for (auto it = tokens.begin(); it != tokens.end(); ++it) {
		grow(&root, *it);
	}

	// Execute the tree

	long result = 0;
	evaluate(root, &result);

#ifdef _DEBUG
	//printf("Source expression: %s, result: %d (0x%08X)\n", text, result, result);
	//dump_tokens(tokens);
	//dump_labels();
	//dump_defines();
	//dump_patches();
#endif

	// Clean
	while (!tokens.empty()) {
		token_t* token = tokens.back();
		tokens.pop_back();
		delete token;
	}

	while (!tree.empty()) {
		node_t* node = tree.back();
		tree.pop_back();
		delete node;
	}
	
	return result;
}

/// <summary>
/// Handle special labels that are actually complex expressions.
/// </summary>
static void do_expr_labels()
{
	for (auto it = labels.begin(); it != labels.end(); ++it) {
		label_s* label = *it;
		if (label->composite) {
			label->orig = eval_expr(label->name);
		}
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

void emit (uint8_t b)
{
	PRG[org++] = b;
}

static oplink optab[] = {

	// CPU Instructions
	{ "BRK", opBRK }, { "RTI", opRTI }, { "RTS", opRTS },

	{ "PHP", opPHP }, { "CLC", opCLC }, { "PLP", opPLP }, { "SEC", opSEC },
	{ "PHA", opPHA }, { "CLI", opCLI }, { "PLA", opPLA }, { "SEI", opSEI },
	{ "DEY", opDEY }, { "TYA", opTYA }, { "TAY", opTAY }, { "CLV", opCLV },
	{ "INY", opINY }, { "CLD", opCLD }, { "INX", opINX }, { "SED", opSED },

	{ "TXA", opTXA }, { "TXS", opTXS }, { "TAX", opTAX }, { "TSX", opTSX },
	{ "DEX", opDEX }, { "NOP", opNOP },

	{ "BPL", opBRA }, { "BMI", opBRA }, { "BVC", opBRA }, { "BVS", opBRA },
	{ "BCC", opBRA }, { "BCS", opBRA }, { "BNE", opBRA }, { "BEQ", opBRA },

	{ "JSR", opJMP }, { "JMP", opJMP }, 

	{ "ORA", opALU1 }, { "AND", opALU1 }, { "EOR", opALU1 }, { "ADC", opALU1 }, 
	{ "CMP", opALU1 }, { "SBC", opALU1 }, 
	{ "CPX", opCPXY }, { "CPY", opCPXY }, 
	{ "INC", opINCDEC }, { "DEC", opINCDEC }, 
	{ "BIT", opBIT },
	{ "ASL", opSHIFT }, { "LSR", opSHIFT }, { "ROL", opSHIFT }, { "ROR", opSHIFT }, 

	{ "LDA", opLDST },
	{ "LDX", opLDSTX },
	{ "LDY", opLDSTY },
	{ "STA", opLDST },
	{ "STX", opLDSTX },
	{ "STY", opLDSTY },

	// Directives
	{ "INCLUDE", opINCLUDE },
	{ "DEFINE", opDEFINE },
	{ "BYTE", opBYTE },
	{ "WORD", opWORD },
	{ "ORG", opORG },
	{ "END", opEND },
	{ "PROCESSOR", opDUMMY },

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

int assemble (char *text, char* source_name, uint8_t *prg)
{
	line l;
	oplink *opl;
	PRG = prg;
	org = 0;
	stop = 0;
	errors = 0;

	cleanup ();

	source_name_stack.push_back(source_name);

	// Add keywords
	linenum_stack.push_back(0);
	add_label ("A", KEYWORD);
	add_label ("X", KEYWORD);
	add_label ("Y", KEYWORD);
	opl = optab;
	while (1) {
		if ( opl->name == NULL ) break;
		else add_label (opl->name, KEYWORD);
		opl++;
	}
	nextline();

	while (1) {
		if (*text == 0) break;
		parse_line (&text, l);

		//printf ( "%s: \'%s\' \'%s\'\n", l.label, l.cmd, l.op );

		// Add label
		if ( strlen (l.label) > 1 ) {
			add_label ( l.label, org );
		}

		// Execute command
		if ( strlen (l.cmd) > 1 ) {
			opl = optab;
			while (1) {
				if ( opl->name == NULL ) {
					printf ("ERROR(%s,%i): Unknown command %s\n", get_source_name().c_str(), get_linenum(), l.cmd);
					break;
				}
				else if ( !_stricmp (opl->name, l.cmd) ) {
					_strupr (l.cmd);
					opl->handler (l.cmd, l.op);
					break;
				}
				opl++;
			}
			if (stop) break;
		}
		nextline();
	}

	// Patch jump/branch offsets.
	do_expr_labels();
	do_patch ();

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
	line l;
	oplink* opl;

	source_name_stack.push_back(source_name);
	linenum_stack.push_back(1);

	while (1) {
		if (*text == 0) break;
		parse_line(&text, l);

		//printf ( "%s: \'%s\' \'%s\'\n", l.label, l.cmd, l.op );

		// Add label
		if (strlen(l.label) > 1) {
			add_label(l.label, org);
		}

		// Execute command
		if (strlen(l.cmd) > 1) {
			opl = optab;
			while (1) {
				if (opl->name == NULL) {
					printf("ERROR(%s,%i): Unknown command %s\n", get_source_name().c_str(), get_linenum(), l.cmd);
					break;
				}
				else if (!_stricmp(opl->name, l.cmd)) {
					_strupr(l.cmd);
					opl->handler(l.cmd, l.op);
					break;
				}
				opl++;
			}
			if (stop) break;
		}
		nextline();
	}

	source_name_stack.pop_back();
	linenum_stack.pop_back();
}
