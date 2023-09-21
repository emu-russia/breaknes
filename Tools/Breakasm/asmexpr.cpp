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
			printf("ERROR(%s,%i): unknown character in string: %c\n", get_source_name().c_str(), get_linenum(), ch);
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

// executing the tree (semantic analysis)
static node_t * evaluate (std::list<node_t*>& tree, node_t * expr, long *lvalue, bool quiet, bool& syntax_error)
{
	node_t * curr;
	long rvalue = 0, mvalue;
	token_t * token;
	OPS uop = OPS::NOP, op = OPS::NOP;
	int debug = 0;

	if (debug) printf ( "[" );

	// lvalue = { [uop] <ident|expr> [op] }
	curr = expr;
	while (curr) {

		if (curr->depth < expr->depth) break;

		// optional unary operation
		token = curr->token;
		if ( token->type == TOKEN_OP && isunary(token->op) ) {
			if (curr->rvalue == NULL) {
				syntax_error = true;
				if (!quiet) {
					printf("Missing identifier\n");
					errors++;
				}
			}
			else {
				curr = curr->rvalue;
				uop = token->op;
			}
		}
		else uop = OPS::NOP;

		if (uop != OPS::NOP && debug) printf ( "%s ", opstr(uop) );

		// mandatory identifier or nested expression
		token = curr->token;
		if ( token->type == TOKEN_IDENT || token->type == TOKEN_NUMBER || curr->depth > expr->depth ) {

			mvalue = 0;

			if ( curr->depth > expr->depth ) {  // nested expression
				if (debug) printf ( "SUBEVAL " );
				curr = evaluate (tree, curr, &mvalue, quiet, syntax_error);
				//printf ( "SUB LVALUE : %i\n", mvalue.num.value );
			}
			else if ( token->type == TOKEN_IDENT) {
				curr = curr->rvalue;

				define_s* def = define_lookup(token->string);
				if (def) {
					mvalue = eval_expr(def->replace, debug, quiet);
				}
				else {
					label_s* label = label_lookup(token->string);
					if (label && label->orig != UNDEF && !label->composite) {
						mvalue = label->orig;
					}
					else {
						syntax_error = true;
						if (!quiet) {
							printf("Undefined identifier: %s", token->string);
							errors++;
						}
					}
				}
				if (debug) printf ( "SYMBOL: %s", token->string );
			}
			else if ( token->type == TOKEN_NUMBER) {
				curr = curr->rvalue;
				mvalue = token->number;
				if (debug) printf ( "NUMBER(%i) ", mvalue );
			}

			// perform unary operation on MVALUE
			if (uop != OPS::NOP) {
				switch (uop)
				{
					case OPS::NOT:
						mvalue = !mvalue;
						break;
					case OPS::NEG:
						mvalue = ~mvalue;
						break;
				}
			}

			// perform binary operation RVALUE = RVALUE op MVALUE
			if (op != OPS::NOP) {
				switch (op)
				{
					case OPS::MINUS:
						rvalue -= mvalue;
						break;
					case OPS::PLUS:
						rvalue += mvalue;
						break;
					case OPS::MUL:
						rvalue *= mvalue;
						break;
					case OPS::DIV:
						if (mvalue) rvalue /= mvalue;
						break;
					case OPS::MOD:
						if (mvalue) rvalue %= mvalue;
						break;
				}
			}
			else rvalue = mvalue;
		}
		else {
			syntax_error = true;
			if (!quiet) {
				printf("Identifier required\n");
				errors++;
			}
		}

		// optional binary operation
		if (curr && curr->depth == expr->depth ) {
			token = curr->token;
			if ( token->type == TOKEN_OP && isbinary(token->op) ) {
				curr = curr->rvalue;
				op = token->op;
			}
			else op = OPS::NOP;

			if (op != OPS::NOP && debug) printf ( "%s ", opstr(op) );
		}
	}

	if (debug) printf ( "]" );

	*lvalue = rvalue;
	return curr;
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
						printf("Unmatched parenthesis");
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

long eval_expr(char* text, bool debug, bool quiet)
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
		evaluate(tree.nodes, root->rvalue->rvalue, &result, quiet, syntax_error);
	}
	if (syntax_error) {
		result = 0;
	}

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
