#include "pch.h"

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

static token_t* create_op_token(OPS optype)
{
	token_t* token = new token_t;
	token->type = TOKEN_OP;
	token->op = optype;
	token->number = 0;
	token->string[0] = 0;
	return token;
}

static token_t* next_token(char** pp, bool quiet)
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
			token->type = TOKEN_NUMBER;
			token->op = OPS::NOP;
			token->number = strtoul(buf, nullptr, base);
			token->string[0] = 0;
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
			token->type = TOKEN_IDENT;
			token->op = OPS::NOP;
			token->number = 0;
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
			//printf("string: %s\n", buf);
			token = new token_t;
			token->type = TOKEN_STRING;
			token->op = OPS::NOP;
			token->number = 0;
			strcpy(token->string, buf);
			return token;
		}

		// Operations
		else if (ch == '(') {
			token = create_op_token(OPS::LPAREN);
			break;
		}
		else if (ch == ')') {
			token = create_op_token(OPS::RPAREN);
			break;
		}
		else if (ch == '+') {
			token = create_op_token(OPS::PLUS);
			break;
		}
		else if (ch == '-') {
			token = create_op_token(OPS::MINUS);
			break;
		}
		else if (ch == '!') {	// ! !=
			char ch2 = get_char(pp);
			if (ch2 == '=') {
				token = create_op_token(OPS::LOGICAL_NOTEQ);
				break;
			}
			else {
				put_back_char(pp);
				token = create_op_token(OPS::NOT);
				break;
			}
		}
		else if (ch == '~') {
			token = create_op_token(OPS::NEG);
			break;
		}
		else if (ch == '*') {
			token = create_op_token(OPS::MUL);
			break;
		}
		else if (ch == '/') {
			token = create_op_token(OPS::DIV);
			break;
		}
		else if (ch == '%') {
			token = create_op_token(OPS::MOD);
			break;
		}
		else if (ch == '&') {
			token = create_op_token(OPS::AND);
			break;
		}
		else if (ch == '|') {
			token = create_op_token(OPS::OR);
			break;
		}
		else if (ch == '^') {
			token = create_op_token(OPS::XOR);
			break;
		}

		else if (ch == '>') {	// > >= >> >>>
			char ch2 = get_char(pp);
			if (ch2 == '=') {
				token = create_op_token(OPS::GREATER_EQ);
				break;
			}
			else if (ch2 == '>') {
				char ch3 = get_char(pp);
				if (ch3 == '>') {
					token = create_op_token(OPS::ROTR);
					break;
				}
				else {
					put_back_char(pp);
					token = create_op_token(OPS::SHR);
					break;
				}
			}
			else {
				put_back_char(pp);
				token = create_op_token(OPS::GREATER);
				break;
			}
		}
		else if (ch == '<') {	// < <= << <<<
			char ch2 = get_char(pp);
			if (ch2 == '=') {
				token = create_op_token(OPS::LESS_EQ);
				break;
			}
			else if (ch2 == '<') {
				char ch3 = get_char(pp);
				if (ch3 == '<') {
					token = create_op_token(OPS::ROTL);
					break;
				}
				else {
					put_back_char(pp);
					token = create_op_token(OPS::SHL);
					break;
				}
			}
			else {
				put_back_char(pp);
				token = create_op_token(OPS::LESS);
				break;
			}
		}
		else if (ch == '=') {	// = ==

			char ch2 = get_char(pp);
			if (ch2 == '=') {
				token = create_op_token(OPS::LOGICAL_EQ);
				break;
			}
			else {
				put_back_char(pp);
				token = create_op_token(OPS::EQ);
				break;
			}
		}

		if (!quiet) {
			ERR("ERROR(%s,%i): unknown character in string: %c\n", get_source_name().c_str(), get_linenum(), ch);
			errors++;
		}
		break;
	}

	return token;
}

static void tokenize(char* text, std::list<token_t*>& tokens, bool quiet)
{
	token_t* next = nullptr;
	char* pp = text;
	do {
		next = next_token(&pp, quiet);
		if (next != nullptr)
			tokens.push_back(next);
	} while (next != nullptr);
}

static const char* opstr(OPS type)
{
	const char* str[] = {
		"NOP", "(", ")", "+", "-", "!", "~", "*", "/", "%",
		"<<", ">>", "<<<", ">>>", ">", ">=", "<", "<=", 
		"==", "!=", "&", "|", "^", "=",
	};
	return str[(int)type];
}

static void dump_tokens(std::list<token_t*>& tokens)
{
	for (auto it = tokens.begin(); it != tokens.end(); ++it) {

		token_t* token = *it;
		switch (token->type)
		{
			case TOKEN_NUMBER:
				printf("TOKEN_NUMBER: %d (0x%08X)\n", token->number, token->number);
				break;
			case TOKEN_IDENT:
				printf("TOKEN_IDENT: %s\n", token->string);
				break;
			case TOKEN_STRING:
				printf("TOKEN_STRING: %s\n", token->string);
				break;
			case TOKEN_OP:
				printf("TOKEN_OP: %s\n", opstr(token->op));
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
	9,			// =
};

static int isunary (OPS op)
{
	switch (op)
	{
		case OPS::NOT:
		case OPS::NEG:
			return 1;
	}
	return 0;
}

static int isbinary (OPS op)
{
	switch (op)
	{
		case OPS::PLUS:
		case OPS::MINUS:
		case OPS::MUL: case OPS::DIV: case OPS::MOD:
		case OPS::SHL: case OPS::SHR: case OPS::ROTL: case OPS::ROTR:
		case OPS::GREATER: case OPS::GREATER_EQ: case OPS::LESS: case OPS::LESS_EQ:
		case OPS::LOGICAL_EQ: case OPS::LOGICAL_NOTEQ:
		case OPS::AND: case OPS::OR: case OPS::XOR:
			return 1;
	}
	return 0;
}

// adding a new branch to the syntax tree
static node_t* addnode(std::list<node_t*>& tree, token_t* token, int depth)
{
	node_t* node = new node_t;
	node->lvalue = node->rvalue = NULL;
	node->depth = depth;
	node->token = token;
	tree.push_back(node);
	return node;
}

// applying a binary operation to two values
static long apply_binary(OPS op, long lvalue, long mvalue)
{
	switch (op)
	{
		case OPS::MINUS:
			return lvalue - mvalue;
		case OPS::PLUS:
			return lvalue + mvalue;
		case OPS::MUL:
			return lvalue * mvalue;
		case OPS::DIV:
			return mvalue ? lvalue / mvalue : lvalue;
		case OPS::MOD:
			return mvalue ? lvalue % mvalue : lvalue;
		case OPS::SHL:
			return lvalue << mvalue;
		case OPS::SHR:
			return (unsigned long)lvalue >> mvalue;
		case OPS::ROTL:
			return (lvalue << mvalue) | ((unsigned long)lvalue >> (32 - mvalue));
		case OPS::ROTR:
			return ((unsigned long)lvalue >> mvalue) | (lvalue << (32 - mvalue));
		case OPS::GREATER:
			return lvalue > mvalue;
		case OPS::GREATER_EQ:
			return lvalue >= mvalue;
		case OPS::LESS:
			return lvalue < mvalue;
		case OPS::LESS_EQ:
			return lvalue <= mvalue;
		case OPS::LOGICAL_EQ:
			return lvalue == mvalue;
		case OPS::LOGICAL_NOTEQ:
			return lvalue != mvalue;
		case OPS::AND:
			return lvalue & mvalue;
		case OPS::OR:
			return lvalue | mvalue;
		case OPS::XOR:
			return lvalue ^ mvalue;
	}
	return lvalue;
}

// Resolve an identifier to its numeric value (define or label).
static bool resolve_ident(const char* name, long* value, bool quiet, bool& syntax_error)
{
	define_s* def = define_lookup((char*)name);
	if (def) {
		bool ok = false;
		*value = eval_expr(def->replace, false, quiet, &ok);
		if (!ok)
			syntax_error = true;
		return ok;
	}
	label_s* label = label_lookup((char*)name);
	if (label && label->orig != UNDEF && !label->composite) {
		*value = label->orig;
		return true;
	}
	syntax_error = true;
	if (!quiet) {
		ERR("Undefined identifier: %s", name);
		errors++;
	}
	return false;
}

// forward declaration (mutual recursion with evaluate)
static long evaluate(std::list<node_t*>& tree, node_t** node, int min_depth, bool quiet, bool& syntax_error);

// Evaluate a single operand: an optional run of unary operations applied to an
// identifier, a number or a higher-priority (nested) run of the syntax tree.
static long eval_operand(std::list<node_t*>& tree, node_t** node, int min_depth, bool quiet, bool& syntax_error)
{
	node_t* curr = *node;
	OPS uops[8];
	int nuop = 0;
	long value = 0;

	// optional unary operations
	while (curr && curr->token->type == TOKEN_OP && isunary(curr->token->op) && nuop < 8) {
		uops[nuop++] = curr->token->op;
		curr = curr->rvalue;
	}

	if (!curr || curr->depth < min_depth) {
		syntax_error = true;
		if (!quiet) {
			ERR("Missing identifier\n");
			errors++;
		}
		*node = NULL;
		return 0;
	}

	if (curr->depth > min_depth) {          // higher-priority run: nested expression
		node_t* sub = curr;
		value = evaluate(tree, &sub, curr->depth, quiet, syntax_error);
		*node = sub;
	}
	else if (curr->token->type == TOKEN_IDENT) {
		if (!resolve_ident(curr->token->string, &value, quiet, syntax_error))
			value = 0;
		*node = curr->rvalue;
	}
	else if (curr->token->type == TOKEN_NUMBER) {
		value = curr->token->number;
		*node = curr->rvalue;
	}
	else {
		syntax_error = true;
		if (!quiet) {
			ERR("Identifier required\n");
			errors++;
		}
		*node = curr->rvalue;
		return 0;
	}

	// apply the unary operations (right to left, as written)
	for (int i = nuop - 1; i >= 0; i--) {
		switch (uops[i])
		{
			case OPS::NOT:
				value = !value;
				break;
			case OPS::NEG:
				value = ~value;
				break;
		}
	}

	return value;
}

// Execute the syntax tree (semantic analysis).
// Evaluates a run of terms joined by binary operators at depth >= min_depth,
// starting from *node (which must point at an operand position).
static long evaluate(std::list<node_t*>& tree, node_t** node, int min_depth, bool quiet, bool& syntax_error)
{
	long lvalue = eval_operand(tree, node, min_depth, quiet, syntax_error);

	while (!syntax_error && *node) {
		node_t* curr = *node;
		if (curr->depth < min_depth) break;
		if (curr->token->type != TOKEN_OP || !isbinary(curr->token->op)) break;

		OPS op = curr->token->op;
		*node = curr->rvalue;
		long mvalue = eval_operand(tree, node, min_depth, quiet, syntax_error);
		lvalue = apply_binary(op, lvalue, mvalue);
	}

	return lvalue;
}

// grow a syntax tree
static void grow (tree_t& tree, node_t **expr, token_t * token, bool quiet)
{
	node_t *node = addnode (tree.nodes, token, tree.depth );

	if (!tree.initialized)
	{
		tree.curr = node;
		*expr = tree.curr;
		tree.depth = tree.prio = 0;
		memset (tree.prio_stack, 0, sizeof(tree.prio_stack) );
		tree.initialized = 1;
	}
	else {

		if (token->type == TOKEN_OP) {
			if (token->op == OPS::LPAREN) {
				tree.prio++;
				tree.depth++;
				return;
			}
			else if (token->op == OPS::RPAREN) {
				tree.prio--;
				if (tree.depth > 0) tree.depth--;
				else {
					if (!quiet) {
						ERR("Unmatched parenthesis");
					}
				}
				return;
			}
			else if (opprio[(int)token->op] > tree.prio_stack[tree.prio]) {  // priority increase. for binary operations, the priority of the previous token is also increased.
				if (isbinary(token->op)) tree.curr->depth++;
				node->depth++;
				tree.prio_stack[tree.prio] = opprio[(int)token->op];
				tree.depth++;
			}
			else if (opprio[(int)token->op] < tree.prio_stack[tree.prio]) { // de-prioritizing
				tree.prio_stack[tree.prio] = opprio[(int)token->op];
				node->depth--;
				tree.depth--;
			}
		}
		// we just ignore the unclosed parentheses

		tree.curr->rvalue = node;
		node->lvalue = tree.curr;
		tree.curr = node;
	}
}

static void dump_tree(tree_t& tree, node_t* root)
{
	node_t *curr = root;
	while (curr) {
		token_t *tok = curr->token;
		printf("(%i)", curr->depth);
		if (tok->type == TOKEN_OP) printf("%s ", opstr(tok->op));
		else if (tok->type == TOKEN_NUMBER) printf("%i ", tok->number);
		else printf("%s ", tok->string);
		curr = curr->rvalue;
	}
	printf("\n");
}

long eval_expr(char* text, bool debug, bool quiet, bool* resolved)
{
	char expr[0x100];
	sprintf(expr, "res=%s", text);

	// Obtain token stream

	std::list<token_t*> tokens;
	tokenize(expr, tokens, quiet);
	if (debug)
		dump_tokens(tokens);

	// Grow syntax tree

	tree_t tree{};
	node_t* root = nullptr;
	for (auto it = tokens.begin(); it != tokens.end(); ++it) {
		grow(tree, &root, *it, quiet);
	}
	if (debug)
		dump_tree(tree, root);

	// Execute the tree

	long result = 0;
	bool syntax_error = false;
	if (root) {
		node_t* start = root->rvalue->rvalue;
		result = evaluate(tree.nodes, &start, 0, quiet, syntax_error);
	}
	if (syntax_error) {
		result = 0;
	}
	if (resolved)
		*resolved = !syntax_error;

	if (debug)
		printf("Source expression: %s, result: %d (0x%08X)\n", text, result, result);

	// Clean
	while (!tokens.empty()) {
		token_t* token = tokens.back();
		tokens.pop_back();
		delete token;
	}

	while (!tree.nodes.empty()) {
		node_t* node = tree.nodes.back();
		tree.nodes.pop_back();
		delete node;
	}
	
	return result;
}
