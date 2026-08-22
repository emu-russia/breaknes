#!/usr/bin/env python3
"""
Breakasm - a simple 6502 assembler.

A Python port of the C++ assembler in this directory (asm.cpp, asmexpr.cpp,
asmops.cpp, main.cpp). It is a faithful port: the same syntax, the same
addressing-mode rules, the same error messages and the same output bytes.

Usage:
    breakasm.py [-l <file.lst>] <source.asm> <output.prg>

The PRG file is always 64 Kbytes (the size of the 6502 address space).
The -l option writes an assembly listing (address, emitted bytes, source line).

Features:
    - expression engine: labels, defines and complex expressions in every
      operand position (LDA MyData + 32 * entry_size + 12)
    - multi-pass assembling until all identifiers, labels and defines are
      resolved (this also resolves the Zero Page / Absolute ambiguity for
      forward-referenced labels below $100)
    - the ABS directive forces the absolute opcode for the next instruction
    - INCLUDE, DEFINE, BYTE, WORD, ORG, END, PROCESSOR directives
"""

import sys

BREAKASM_VERSION = "1.4"
PRG_SIZE = 0x10000

UNDEF = 0xBABADABA      # undefined offset
KEYWORD = 0xD0D0D0D0    # keyword (register or instruction name)

# eval types
EVAL_WTF, EVAL_NUMBER, EVAL_ADDRESS, EVAL_LABEL, EVAL_STRING = range(5)

# token types
TOKEN_NUMBER, TOKEN_IDENT, TOKEN_STRING, TOKEN_OP = 1, 2, 3, 4


class OPS:
    """Operations of the expression engine (order matters: see OPPRIO)."""
    NOP = 0
    LPAREN, RPAREN = 1, 2
    PLUS, MINUS = 3, 4
    NOT, NEG = 5, 6
    MUL, DIV, MOD = 7, 8, 9
    SHL, SHR, ROTL, ROTR = 10, 11, 12, 13
    GREATER, GREATER_EQ, LESS, LESS_EQ = 14, 15, 16, 17
    LOGICAL_EQ, LOGICAL_NOTEQ = 18, 19
    AND, OR, XOR = 20, 21, 22
    EQ = 23


# priorities of the operations (1: low -> high)
OPPRIO = [
    1,              # nop
    12, 12,         # ( )
    9,              # +
    9,              # -
    11, 11,         # ! ~
    10, 10, 10,     # * / %
    8, 8, 8, 8,     # << >> <<< >>>
    7, 7, 7, 7,     # > >= < <=
    7, 7,           # == !=
    6, 4, 5,        # & | ^
    9,              # =
]

UNARY_OPS = frozenset((OPS.NOT, OPS.NEG))
BINARY_OPS = frozenset((
    OPS.PLUS, OPS.MINUS, OPS.MUL, OPS.DIV, OPS.MOD,
    OPS.SHL, OPS.SHR, OPS.ROTL, OPS.ROTR,
    OPS.GREATER, OPS.GREATER_EQ, OPS.LESS, OPS.LESS_EQ,
    OPS.LOGICAL_EQ, OPS.LOGICAL_NOTEQ,
    OPS.AND, OPS.OR, OPS.XOR,
))


def is_digit(c):
    return '0' <= c <= '9'


def is_alpha(c):
    return ('a' <= c <= 'z') or ('A' <= c <= 'Z')


def is_xdigit(c):
    return ('0' <= c <= '9') or ('a' <= c <= 'f') or ('A' <= c <= 'F')


def is_ident_char(c):
    return is_alpha(c) or is_digit(c) or c == '_'


def strtoul_like(s, base):
    """Mimic C strtoul() partial parsing (no overflow handling)."""
    if not s:
        return 0
    sign = 1
    i = 0
    if s[0] in '+-':
        if s[0] == '-':
            sign = -1
        i = 1
    value = 0
    if base == 16:
        if i + 1 < len(s) and s[i] == '0' and s[i + 1] in 'xX':
            i += 2
        while i < len(s) and is_xdigit(s[i]):
            value = value * 16 + int(s[i], 16)
            i += 1
    else:
        while i < len(s) and is_digit(s[i]):
            value = value * 10 + int(s[i])
            i += 1
    return sign * value


# ---------------------------------------------------------------------------
# Expression engine (the port of asmexpr.cpp)
# ---------------------------------------------------------------------------

class Token(object):
    __slots__ = ('type', 'op', 'number', 'string')

    def __init__(self, token_type, op=OPS.NOP, number=0, string=""):
        self.type = token_type
        self.op = op
        self.number = number
        self.string = string


class Node(object):
    __slots__ = ('lvalue', 'rvalue', 'token', 'depth')

    def __init__(self, token, depth):
        self.lvalue = None
        self.rvalue = None
        self.token = token
        self.depth = depth


class Label(object):
    __slots__ = ('name', 'orig', 'source', 'line', 'composite')

    def __init__(self, name, orig, source, line):
        self.name = name
        self.orig = orig
        self.source = source
        self.line = line
        self.composite = False


class Patch(object):
    __slots__ = ('label', 'orig', 'branch', 'source', 'line')

    def __init__(self, label, orig, branch, source, line):
        self.label = label
        self.orig = orig
        self.branch = branch  # 0: absolute (2 bytes), 1: relative (1 byte), 2: zero page (1 byte)
        self.source = source
        self.line = line


class Define(object):
    __slots__ = ('name', 'replace')

    def __init__(self, name, replace):
        self.name = name
        self.replace = replace


class Eval(object):
    """Result of the operand classification (the port of eval_t)."""
    __slots__ = ('type', 'number', 'address', 'string', 'label', 'indirect')

    def __init__(self, etype, number=0, address=0, string="", label=None, indirect=False):
        self.type = etype
        self.number = number
        self.address = address
        self.string = string
        self.label = label
        self.indirect = indirect


class ListingRec(object):
    __slots__ = ('org_before', 'org_after', 'label', 'cmd', 'op')

    def __init__(self, org_before, org_after, label, cmd, op):
        self.org_before = org_before
        self.org_after = org_after
        self.label = label
        self.cmd = cmd
        self.op = op


# ---------------------------------------------------------------------------
# The unified 6502 instruction table (the port of instab in asmops.cpp).
# Tuple fields: (imp, imm, zpg, zpgx, zpgy, indx, indy, abs, absx, absy, ind)
# A zero opcode means that the addressing mode is not available.
# ---------------------------------------------------------------------------

INSTR = {
    # Load / Store
    "LDA": (0,   0xA9, 0xA5, 0xB5, 0,   0xA1, 0xB1, 0xAD, 0xBD, 0xB9, 0),
    "STA": (0,   0,    0x85, 0x95, 0,   0x81, 0x91, 0x8D, 0x9D, 0x99, 0),
    "LDX": (0,   0xA2, 0xA6, 0,    0xB6, 0,    0,    0xAE, 0,    0xBE, 0),
    "STX": (0,   0,    0x86, 0,    0x96, 0,    0,    0x8E, 0,    0,    0),
    "LDY": (0,   0xA0, 0xA4, 0xB4, 0,   0,    0,    0xAC, 0xBC, 0,    0),
    "STY": (0,   0,    0x84, 0x94, 0,   0,    0,    0x8C, 0,    0,    0),
    # ALU
    "ORA": (0,   0x09, 0x05, 0x15, 0,   0x01, 0x11, 0x0D, 0x1D, 0x19, 0),
    "AND": (0,   0x29, 0x25, 0x35, 0,   0x21, 0x31, 0x2D, 0x3D, 0x39, 0),
    "EOR": (0,   0x49, 0x45, 0x55, 0,   0x41, 0x51, 0x4D, 0x5D, 0x59, 0),
    "ADC": (0,   0x69, 0x65, 0x75, 0,   0x61, 0x71, 0x6D, 0x7D, 0x79, 0),
    "CMP": (0,   0xC9, 0xC5, 0xD5, 0,   0xC1, 0xD1, 0xCD, 0xDD, 0xD9, 0),
    "SBC": (0,   0xE9, 0xE5, 0xF5, 0,   0xE1, 0xF1, 0xED, 0xFD, 0xF9, 0),
    # Shifts / rotates
    "ASL": (0x0A, 0,   0x06, 0x16, 0,   0,    0,    0x0E, 0x1E, 0,    0),
    "ROL": (0x2A, 0,   0x26, 0x36, 0,   0,    0,    0x2E, 0x3E, 0,    0),
    "LSR": (0x4A, 0,   0x46, 0x56, 0,   0,    0,    0x4E, 0x5E, 0,    0),
    "ROR": (0x6A, 0,   0x66, 0x76, 0,   0,    0,    0x6E, 0x7E, 0,    0),
    # Other memory instructions
    "BIT": (0,   0,    0x24, 0,    0,   0,    0,    0x2C, 0,    0,    0),
    "INC": (0,   0,    0xE6, 0xF6, 0,   0,    0,    0xEE, 0xFE, 0,    0),
    "DEC": (0,   0,    0xC6, 0xD6, 0,   0,    0,    0xCE, 0xDE, 0,    0),
    "CPX": (0,   0xE0, 0xE4, 0,    0,   0,    0,    0xEC, 0,    0,    0),
    "CPY": (0,   0xC0, 0xC4, 0,    0,   0,    0,    0xCC, 0,    0,    0),
    # Jumps
    "JMP": (0,   0,    0,    0,    0,   0,    0,    0x4C, 0,    0,    0x6C),
    "JSR": (0,   0,    0,    0,    0,   0,    0,    0x20, 0,    0,    0),
    # Implied
    "BRK": (0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "RTI": (0x40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "RTS": (0x60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "PHP": (0x08, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "CLC": (0x18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "PLP": (0x28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "SEC": (0x38, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "PHA": (0x48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "CLI": (0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "PLA": (0x68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "SEI": (0x78, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "DEY": (0x88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "TYA": (0x98, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "TAY": (0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "CLV": (0xB8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "INY": (0xC8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "CLD": (0xD8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "INX": (0xE8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "SED": (0xF8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "TXA": (0x8A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "TXS": (0x9A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "TAX": (0xAA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "TSX": (0xBA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "DEX": (0xCA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
    "NOP": (0xEA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
}

BRANCH_OPS = {
    "BPL": 0x10, "BMI": 0x30, "BVC": 0x50, "BVS": 0x70,
    "BCC": 0x90, "BCS": 0xB0, "BNE": 0xD0, "BEQ": 0xF0,
}


def is_implied_only(ins):
    """True if the instruction has only the implied addressing mode.
    (BRK is 0x00, so the opcode itself cannot be used to detect it.)"""
    return all(op == 0 for op in ins[1:])


class Assembler(object):
    def __init__(self):
        self.prg = bytearray(PRG_SIZE)
        self.list_file = None       # listing output stream (None = disabled)
        self.silent = False         # suppress error output (intermediate passes)
        self.force_abs = False      # the ABS directive is active
        self.org = 0
        self.stop = False
        self.errors = 0
        self.labels = {}            # lower name -> Label
        self.patches = []
        self.defines = {}           # lower name -> Define
        self.source_stack = []
        self.linenum_stack = []
        self.prev_labels = {}       # lower name -> orig (from the previous pass)
        self.listing_lines = []

    # ------------------------------------------------------------------
    # Diagnostics
    # ------------------------------------------------------------------

    def get_linenum(self):
        return self.linenum_stack[-1] if self.linenum_stack else 0

    def get_source_name(self):
        return self.source_stack[-1] if self.source_stack else ""

    def nextline(self):
        self.linenum_stack[-1] += 1

    def err_at(self, source, line, msg):
        if not self.silent:
            print("ERROR(%s,%i): %s" % (source, line, msg))
        self.errors += 1

    def error(self, msg):
        self.err_at(self.get_source_name(), self.get_linenum(), msg)

    def not_enough_parameters(self, cmd):
        self.error("%s not enough parameters" % cmd)

    def wrong_parameters(self, cmd, op):
        self.error("%s wrong parameters: %s" % (cmd, op))

    # ------------------------------------------------------------------
    # Labels / patches / defines
    # ------------------------------------------------------------------

    def add_label(self, name, orig):
        temp = name
        while temp and temp[-1] <= ' ':
            temp = temp[:-1]
        key = temp.lower()
        lab = self.labels.get(key)
        if lab is None:
            lab = Label(temp, orig, self.get_source_name(), self.get_linenum())
            self.labels[key] = lab
        else:
            if lab.orig == KEYWORD:
                self.error("Reserved keyword used as label '%s'" % temp)
            elif orig != UNDEF and lab.orig != UNDEF:
                self.error("label '%s' already defined in %s, line %i"
                           % (temp, lab.source, lab.line))
            elif orig != UNDEF:
                lab.orig = orig
                lab.source = self.get_source_name()
                lab.line = self.get_linenum()
        return lab

    def add_patch(self, label, orig, branch):
        self.patches.append(Patch(label, orig, branch,
                                  self.get_source_name(), self.get_linenum()))

    def do_expr_labels(self):
        """Evaluate composite labels (complex expressions) at the end of a pass."""
        for lab in self.labels.values():
            if lab.composite:
                lab.orig = self.eval_expr(lab.name, quiet=False)[0]

    def add_define(self, name, replace):
        key = name.lower()
        lab = self.labels.get(key)
        if lab is not None:
            self.error("Already defined as label in %s, line %i" % (lab.source, lab.line))
            return None
        if key in self.defines:
            self.defines[key].replace = replace
        else:
            self.defines[key] = Define(name, replace)
        return self.defines[key]

    def do_patch(self):
        for patch in self.patches:
            orig = patch.label.orig
            if orig == UNDEF:
                self.err_at(patch.source, patch.line,
                            "Undefined label '%s' (%08X)" % (patch.label.name, orig))
            else:
                if patch.branch == 1:                   # relative branch
                    self.org = patch.orig
                    rel = orig - self.org - 1
                    if rel > 127 or rel < -128:
                        self.err_at(patch.source, patch.line,
                                    "Branch relative offset to %s out of range"
                                    % patch.label.name)
                    else:
                        self.emit(rel & 0xff)
                elif patch.branch == 2:                 # zero page operand
                    self.org = patch.orig
                    self.emit(orig & 0xff)
                else:                                   # absolute / word operand
                    self.org = patch.orig
                    self.emit(orig & 0xff)
                    self.emit((orig >> 8) & 0xff)

    # ------------------------------------------------------------------
    # Expression engine (the port of asmexpr.cpp)
    # ------------------------------------------------------------------

    def next_token(self, text, i, quiet):
        """Lexer: returns (token, next_index) or (None, i) at end of input."""
        n = len(text)
        while i < n and text[i] <= ' ':
            i += 1
        if i >= n:
            return None, i
        ch = text[i]

        if ch == '#' or ch == '$' or is_digit(ch):      # Number
            base = 16 if ch == '$' else 10
            buf = []
            if is_digit(ch):
                buf.append(ch)
            i += 1
            while i < n:
                c = text[i]
                if c == '#':
                    pass
                elif c == '$':
                    base = 16
                elif is_xdigit(c):
                    buf.append(c)
                else:
                    break
                i += 1
            return Token(TOKEN_NUMBER, number=strtoul_like(''.join(buf), base)), i

        if ch == '_' or is_alpha(ch):                   # Identifier
            buf = [ch]
            i += 1
            while i < n and is_ident_char(text[i]):
                buf.append(text[i])
                i += 1
            return Token(TOKEN_IDENT, string=''.join(buf)), i

        if ch == '"' or ch == "'":                      # "String" 'String'
            single = (ch == "'")
            buf = []
            i += 1
            while i < n:
                c = text[i]
                if c == "'" or c == '"':
                    if single:
                        if c == "'":
                            break
                    else:
                        if c == '"':
                            break
                buf.append(c)
                i += 1
            if i < n:
                i += 1                                  # consume the closing quote
            return Token(TOKEN_STRING, string=''.join(buf)), i

        ops = {
            '(': OPS.LPAREN, ')': OPS.RPAREN, '+': OPS.PLUS, '-': OPS.MINUS,
            '~': OPS.NEG, '*': OPS.MUL, '/': OPS.DIV, '%': OPS.MOD,
            '&': OPS.AND, '|': OPS.OR, '^': OPS.XOR,
        }
        if ch in ops:                                   # simple operators
            return Token(TOKEN_OP, op=ops[ch]), i + 1

        if ch == '!':                                   # ! !=
            if i + 1 < n and text[i + 1] == '=':
                return Token(TOKEN_OP, op=OPS.LOGICAL_NOTEQ), i + 2
            return Token(TOKEN_OP, op=OPS.NOT), i + 1

        if ch == '>':                                   # > >= >> >>>
            if i + 1 < n and text[i + 1] == '=':
                return Token(TOKEN_OP, op=OPS.GREATER_EQ), i + 2
            if i + 1 < n and text[i + 1] == '>':
                if i + 2 < n and text[i + 2] == '>':
                    return Token(TOKEN_OP, op=OPS.ROTR), i + 3
                return Token(TOKEN_OP, op=OPS.SHR), i + 2
            return Token(TOKEN_OP, op=OPS.GREATER), i + 1

        if ch == '<':                                   # < <= << <<<
            if i + 1 < n and text[i + 1] == '=':
                return Token(TOKEN_OP, op=OPS.LESS_EQ), i + 2
            if i + 1 < n and text[i + 1] == '<':
                if i + 2 < n and text[i + 2] == '<':
                    return Token(TOKEN_OP, op=OPS.ROTL), i + 3
                return Token(TOKEN_OP, op=OPS.SHL), i + 2
            return Token(TOKEN_OP, op=OPS.LESS), i + 1

        if ch == '=':                                   # = ==
            if i + 1 < n and text[i + 1] == '=':
                return Token(TOKEN_OP, op=OPS.LOGICAL_EQ), i + 2
            return Token(TOKEN_OP, op=OPS.EQ), i + 1

        if not quiet:
            self.error("unknown character in string: %c" % ch)
        return None, i + 1

    def tokenize(self, text, quiet=False):
        tokens = []
        i = 0
        while True:
            tok, i = self.next_token(text, i, quiet)
            if tok is None:
                break
            tokens.append(tok)
        return tokens

    def grow(self, tree, token):
        """Grow the syntax tree (the port of grow() in asmexpr.cpp)."""
        node = Node(token, tree['depth'])
        if not tree['initialized']:
            tree['curr'] = node
            tree['root'] = node
            tree['depth'] = 0
            tree['prio'] = 0
            tree['prio_stack'] = [0] * 1000
            tree['initialized'] = True
        else:
            if token.type == TOKEN_OP:
                if token.op == OPS.LPAREN:
                    tree['prio'] += 1
                    tree['depth'] += 1
                    return
                if token.op == OPS.RPAREN:
                    tree['prio'] -= 1
                    if tree['depth'] > 0:
                        tree['depth'] -= 1
                    # unmatched closing parentheses are ignored (as in the C++)
                    return
                if OPPRIO[token.op] > tree['prio_stack'][tree['prio']]:
                    # priority increase
                    if token.op in BINARY_OPS:
                        tree['curr'].depth += 1
                    node.depth += 1
                    tree['prio_stack'][tree['prio']] = OPPRIO[token.op]
                    tree['depth'] += 1
                elif OPPRIO[token.op] < tree['prio_stack'][tree['prio']]:
                    # de-prioritizing
                    tree['prio_stack'][tree['prio']] = OPPRIO[token.op]
                    node.depth -= 1
                    tree['depth'] -= 1
            tree['curr'].rvalue = node
            node.lvalue = tree['curr']
            tree['curr'] = node
        return node

    def apply_binary(self, op, lvalue, mvalue):
        if op == OPS.MINUS:
            return lvalue - mvalue
        if op == OPS.PLUS:
            return lvalue + mvalue
        if op == OPS.MUL:
            return lvalue * mvalue
        if op == OPS.DIV:
            return lvalue // mvalue if mvalue else lvalue
        if op == OPS.MOD:
            return lvalue % mvalue if mvalue else lvalue
        if op == OPS.SHL:
            return lvalue << mvalue
        if op == OPS.SHR:
            return (lvalue & 0xFFFFFFFF) >> mvalue
        if op == OPS.ROTL:
            return ((lvalue << mvalue) | ((lvalue & 0xFFFFFFFF) >> (32 - mvalue))) & 0xFFFFFFFF
        if op == OPS.ROTR:
            return (((lvalue & 0xFFFFFFFF) >> mvalue) | (lvalue << (32 - mvalue))) & 0xFFFFFFFF
        if op == OPS.GREATER:
            return 1 if lvalue > mvalue else 0
        if op == OPS.GREATER_EQ:
            return 1 if lvalue >= mvalue else 0
        if op == OPS.LESS:
            return 1 if lvalue < mvalue else 0
        if op == OPS.LESS_EQ:
            return 1 if lvalue <= mvalue else 0
        if op == OPS.LOGICAL_EQ:
            return 1 if lvalue == mvalue else 0
        if op == OPS.LOGICAL_NOTEQ:
            return 1 if lvalue != mvalue else 0
        if op == OPS.AND:
            return lvalue & mvalue
        if op == OPS.OR:
            return lvalue | mvalue
        if op == OPS.XOR:
            return lvalue ^ mvalue
        return lvalue

    def resolve_ident(self, name, quiet):
        """Resolve an identifier to its numeric value (define or label).
        Returns (ok, value)."""
        key = name.lower()
        if key in self.defines:
            value, ok = self.eval_expr(self.defines[key].replace, quiet=quiet)
            return ok, value
        lab = self.labels.get(key)
        if lab is not None and lab.orig != UNDEF and not lab.composite:
            return True, lab.orig
        if not quiet:
            self.error("Undefined identifier: %s" % name)
        return False, 0

    def eval_operand(self, tree, node, min_depth, quiet):
        """Evaluate a single operand of the tree. Returns (value, node, syntax_error)."""
        curr = node
        uops = []
        while curr is not None and curr.token.type == TOKEN_OP \
                and curr.token.op in UNARY_OPS and len(uops) < 8:
            uops.append(curr.token.op)
            curr = curr.rvalue

        if curr is None or curr.depth < min_depth:
            if not quiet:
                self.error("Missing identifier")
            return 0, None, True

        syntax_error = False
        value = 0

        if curr.depth > min_depth:                  # higher-priority run: nested expression
            sub = curr
            value, sub, syntax_error = self.evaluate(tree, sub, curr.depth, quiet)
            curr = sub
        elif curr.token.type == TOKEN_IDENT:
            ok, value = self.resolve_ident(curr.token.string, quiet)
            if not ok:
                value = 0
                syntax_error = True
            curr = curr.rvalue
        elif curr.token.type == TOKEN_NUMBER:
            value = curr.token.number
            curr = curr.rvalue
        else:
            if not quiet:
                self.error("Identifier required")
            return 0, curr.rvalue, True

        # apply the unary operations (right to left, as written)
        for uop in reversed(uops):
            if uop == OPS.NOT:
                value = 0 if value else 1
            elif uop == OPS.NEG:
                value = ~value

        return value, curr, syntax_error

    def evaluate(self, tree, node, min_depth, quiet):
        """Evaluate a run of terms joined by binary operators at depth >= min_depth.
        Returns (value, node, syntax_error)."""
        syntax_error = False
        lvalue, node, se = self.eval_operand(tree, node, min_depth, quiet)
        syntax_error = syntax_error or se

        while not syntax_error and node is not None:
            curr = node
            if curr.depth < min_depth:
                break
            if curr.token.type != TOKEN_OP or curr.token.op not in BINARY_OPS:
                break
            op = curr.token.op
            node = curr.rvalue
            mvalue, node, se = self.eval_operand(tree, node, min_depth, quiet)
            syntax_error = syntax_error or se
            lvalue = self.apply_binary(op, lvalue, mvalue)

        return lvalue, node, syntax_error

    def eval_expr(self, text, quiet=False):
        """Calculate the expression and return (value, resolved)."""
        expr = "res=" + text
        tokens = self.tokenize(expr, quiet)

        tree = {'nodes': [], 'curr': None, 'root': None, 'depth': 0,
                'prio': 0, 'prio_stack': [0] * 1000, 'initialized': False}
        for tok in tokens:
            self.grow(tree, tok)

        result = 0
        syntax_error = False
        root = tree['root']
        if root is not None:
            start = root.rvalue.rvalue
            result, _, syntax_error = self.evaluate(tree, start, 0, quiet)
        if syntax_error:
            result = 0
        return result, not syntax_error

    # ------------------------------------------------------------------
    # Operand classification (the port of eval() in asm.cpp)
    # ------------------------------------------------------------------

    def strip_outer_parens(self, text):
        """If the whole operand is wrapped in a single pair of parentheses,
        replace them with spaces and return (text, True). Otherwise (text, False).
        Whole-operand parentheses mean indirect addressing (JMP (addr));
        parentheses inside an expression are grouping only."""
        n = len(text)
        i = 0
        while i < n and text[i] <= ' ':
            i += 1
        j = n - 1
        while j >= 0 and text[j] <= ' ':
            j -= 1
        if i >= j or text[i] != '(' or text[j] != ')':
            return text, False
        depth = 0
        for k in range(i, j + 1):
            if text[k] == '(':
                depth += 1
            elif text[k] == ')':
                depth -= 1
                if depth == 0 and k != j:
                    return text, False
        if depth != 0:
            return text, False
        lst = list(text)
        lst[i] = ' '
        lst[j] = ' '
        return ''.join(lst), True

    def eval_op(self, text):
        """Classify an operand. Returns an Eval object."""
        text, indirect = self.strip_outer_parens(text)
        text = text.strip()

        # String
        if text and (text[0] == "'" or text[0] == '"'):
            quot = text[0]
            buf = []
            for c in text[1:]:
                if c == quot:
                    break
                buf.append(c)
            return Eval(EVAL_STRING, string=''.join(buf), indirect=indirect)

        # Immediate: the '#' prefix. Evaluate the rest of the expression,
        # e.g. #(MONSTER_TAB & #$FF), #MONSTER_TAB, #5+1, #-1.
        if text and text[0] == '#':
            rest = text[1:].lstrip()
            if rest[:1] in ('-', '+'):
                rest = '0' + rest     # the expression engine has no unary +/-
            value, resolved = self.eval_expr(rest, quiet=True)
            if not resolved:
                # Forward reference: the value is computed on a later pass and
                # the immediate operand is patched with the final value. A pure
                # identifier is a real label; complex expressions are evaluated
                # by do_expr_labels().
                lab = self.add_label(rest, UNDEF)
                lab.composite = not all(is_ident_char(c) for c in rest)
                return Eval(EVAL_NUMBER, number=0, label=lab)
            return Eval(EVAL_NUMBER, number=value)

        # A pure identifier [_a-zA-Z][_a-zA-Z0-9]* is a label or a define
        if text and text[0] != '#' and not is_digit(text[0]) and text[0] != '$':
            if all(is_ident_char(c) for c in text):
                key = text.lower()
                if key in self.defines:
                    # define: evaluate the replacement (the recursion loses the
                    # indirect flag, as in the C++)
                    return self.eval_op(self.defines[key].replace)
                lab = self.labels.get(key)
                if lab is None:
                    lab = self.add_label(text, UNDEF)
                return Eval(EVAL_LABEL, label=lab, indirect=indirect)

        # Everything else is evaluated by the expression engine
        value, resolved = self.eval_expr(text, quiet=True)
        if not resolved:
            # The expression refers to not-yet-defined identifiers: register the
            # whole expression as a composite label; its value is computed by
            # do_expr_labels() at the end of the pass.
            lab = self.add_label(text, UNDEF)
            lab.composite = True
            return Eval(EVAL_LABEL, label=lab, indirect=indirect)

        if text[0] == '#' or is_digit(text[0]):     # immediate number
            return Eval(EVAL_NUMBER, number=value, indirect=indirect)
        return Eval(EVAL_ADDRESS, address=value, indirect=indirect)

    # ------------------------------------------------------------------
    # Parser helpers
    # ------------------------------------------------------------------

    def split_param(self, op):
        """Split the operand string on commas (respecting quotes)."""
        params = []
        cur = []
        quot = 0
        for c in op:
            if c == ',' and not quot:
                s = ''.join(cur).strip()
                if s:
                    params.append(s)
                cur = []
            elif c == "'" or c == '"':
                if quot == 0:
                    quot = c
                elif quot == c:
                    quot = 0
                cur.append(c)
            else:
                cur.append(c)
        s = ''.join(cur).strip()
        if s:
            params.append(s)
        return params

    def parse_line(self, line):
        """Split one source line into (label, cmd, op)."""
        semi = line.find(';')
        if semi >= 0:
            line = line[:semi]
        label = ''
        cmd = ''
        op = ''
        i = 0
        n = len(line)
        while i < n and line[i] <= ' ':
            i += 1
        pp = []
        parsing_cmd = True
        quot = False
        dquot = False
        while i < n:
            c = line[i]
            i += 1
            if c == "'" and not dquot:
                quot = not quot
            if c == '"' and not quot:
                dquot = not dquot
            if c == ':' and not (quot or dquot):
                label = ''.join(pp)
                pp = []
                parsing_cmd = True
                while i < n and line[i] <= ' ':
                    i += 1
            elif c <= ' ' and parsing_cmd:
                cmd = ''.join(pp)
                pp = []
                parsing_cmd = False
                while i < n and line[i] <= ' ':
                    i += 1
            else:
                pp.append(c)
        if parsing_cmd:
            cmd = ''.join(pp)
        else:
            op = ''.join(pp)
        return label, cmd, op

    def emit(self, b):
        self.prg[self.org] = b & 0xFF
        self.org += 1

    # ------------------------------------------------------------------
    # The unified instruction handler (the port of op_std in asmops.cpp)
    # ------------------------------------------------------------------

    def is_reg(self, e, reg):
        return e.type == EVAL_LABEL and e.label.orig == KEYWORD \
            and e.label.name.upper() == reg

    def addr_value(self, e):
        """Get the numeric address value of an evaluated operand.
        Returns (ok, addr, provisional). A provisional value comes from the
        previous pass (a forward reference): the operand must be patched with
        the final value of the current pass."""
        if e.type == EVAL_ADDRESS:
            return True, e.address, False
        if e.type == EVAL_NUMBER:
            return True, e.number, False
        if e.type == EVAL_LABEL:
            lab = e.label
            if lab.orig == KEYWORD:
                return False, 0, False     # a register name is not an address value
            if lab.composite:
                value, resolved = self.eval_expr(lab.name, quiet=True)
                if resolved:
                    return True, value, False
                key = lab.name.lower()
                if key in self.prev_labels:
                    return True, self.prev_labels[key], True
                return False, 0, False
            if lab.orig != UNDEF:
                return True, lab.orig, False
            key = lab.name.lower()
            if key in self.prev_labels:
                return True, self.prev_labels[key], True
            return False, 0, False
        return False, 0, False

    def emit_operand(self, addr, provisional, label, num_bytes, cmd, ops):
        if provisional:
            self.add_patch(label, self.org, 2 if num_bytes == 1 else 0)
        for i in range(num_bytes):
            if provisional:
                self.emit(0)
            else:
                self.emit((addr >> (8 * i)) & 0xFF)

    def emit_addr(self, ins, addr, indirect, provisional, label, cmd, ops):
        """zero page / absolute / (indirect) operand with a known address."""
        if indirect:
            if not ins[10]:
                self.wrong_parameters(cmd, ops)
                return
            self.emit(ins[10])
            self.emit_operand(addr, provisional, label, 2, cmd, ops)
            return
        if addr < 0x100 and not self.force_abs and ins[2]:      # zero page
            self.emit(ins[2])
            self.emit_operand(addr, provisional, label, 1, cmd, ops)
        elif ins[7]:                                            # absolute
            self.emit(ins[7])
            self.emit_operand(addr, provisional, label, 2, cmd, ops)
        else:
            self.wrong_parameters(cmd, ops)

    def emit_patch_addr(self, ins, label, indirect, cmd, ops):
        opcode = ins[10] if indirect else ins[7]
        if not opcode:
            self.wrong_parameters(cmd, ops)
            return
        self.emit(opcode)
        self.add_patch(label, self.org, 0)
        self.emit(0)
        self.emit(0)

    def emit_addr_x(self, ins, addr, provisional, label, cmd, ops):
        if addr < 0x100 and not self.force_abs and ins[3]:      # zero page, X
            self.emit(ins[3])
            self.emit_operand(addr, provisional, label, 1, cmd, ops)
        elif ins[8]:                                            # absolute, X
            self.emit(ins[8])
            self.emit_operand(addr, provisional, label, 2, cmd, ops)
        else:
            self.wrong_parameters(cmd, ops)

    def emit_patch_x(self, ins, label, cmd, ops):
        if not ins[8]:
            self.wrong_parameters(cmd, ops)
            return
        self.emit(ins[8])
        self.add_patch(label, self.org, 0)
        self.emit(0)
        self.emit(0)

    def emit_addr_y(self, ins, addr, provisional, label, cmd, ops):
        if addr < 0x100 and not self.force_abs:                 # (indirect),Y / zero page,Y
            if ins[6]:                                          # (indirect), Y
                self.emit(ins[6])
                self.emit_operand(addr, provisional, label, 1, cmd, ops)
                return
            if ins[4]:                                          # zero page, Y
                self.emit(ins[4])
                self.emit_operand(addr, provisional, label, 1, cmd, ops)
                return
        if ins[9]:                                              # absolute, Y
            self.emit(ins[9])
            self.emit_operand(addr, provisional, label, 2, cmd, ops)
        else:
            self.wrong_parameters(cmd, ops)

    def emit_patch_y(self, ins, label, cmd, ops):
        if not ins[9]:
            self.wrong_parameters(cmd, ops)
            return
        self.emit(ins[9])
        self.add_patch(label, self.org, 0)
        self.emit(0)
        self.emit(0)

    def emit_indx(self, ins, addr, provisional, label, cmd, ops):
        """X, addr -> (indirect, X)"""
        if not ins[5]:
            self.wrong_parameters(cmd, ops)
            return
        self.emit(ins[5])
        self.emit_operand(addr, provisional, label, 1, cmd, ops)

    def emit_patch_indx(self, ins, label, cmd, ops):
        if not ins[5]:
            self.wrong_parameters(cmd, ops)
            return
        self.emit(ins[5])
        self.add_patch(label, self.org, 2)
        self.emit(0)

    def op_std(self, cmd, ops):
        ins = INSTR.get(cmd)
        if ins is None:
            self.wrong_parameters(cmd, ops)
            return

        params = self.split_param(ops)

        # Implied / accumulator
        if not params:
            if ins[0] or is_implied_only(ins):
                self.emit(ins[0])
            else:
                self.not_enough_parameters(cmd)
            return

        # One operand
        if len(params) == 1:
            e0 = self.eval_op(params[0])
            if e0.type == EVAL_NUMBER:          # #immediate
                if ins[1]:
                    self.emit(ins[1])
                    if e0.label is not None:    # forward-referenced immediate: patch later
                        self.add_patch(e0.label, self.org, 2)
                        self.emit(0)
                    else:
                        self.emit(e0.number & 0xFF)
                else:
                    self.wrong_parameters(cmd, ops)
            elif e0.type == EVAL_STRING:
                self.wrong_parameters(cmd, ops)
            elif e0.type == EVAL_ADDRESS:
                self.emit_addr(ins, e0.address, e0.indirect, False, None, cmd, ops)
            elif e0.type == EVAL_LABEL:
                if e0.label.orig == KEYWORD:    # accumulator form: ASL A
                    if e0.label.name.upper() == 'A' and ins[0]:
                        self.emit(ins[0])
                    else:
                        self.wrong_parameters(cmd, ops)
                else:
                    ok, addr, prov = self.addr_value(e0)
                    if ok:
                        self.emit_addr(ins, addr, e0.indirect, prov, e0.label, cmd, ops)
                    else:
                        self.emit_patch_addr(ins, e0.label, e0.indirect, cmd, ops)
            else:
                self.wrong_parameters(cmd, ops)
            return

        # Two operands
        if len(params) == 2:
            e0 = self.eval_op(params[0])
            e1 = self.eval_op(params[1])

            # X, addr -> (indirect, X)
            if self.is_reg(e0, 'X'):
                ok, addr, prov = self.addr_value(e1)
                if ok:
                    self.emit_indx(ins, addr, prov, e1.label, cmd, ops)
                elif e1.type == EVAL_LABEL and e1.label.orig != KEYWORD:
                    self.emit_patch_indx(ins, e1.label, cmd, ops)
                else:
                    self.wrong_parameters(cmd, ops)
                return

            # addr, X
            if self.is_reg(e1, 'X'):
                ok, addr, prov = self.addr_value(e0)
                if ok:
                    self.emit_addr_x(ins, addr, prov, e0.label, cmd, ops)
                elif e0.type == EVAL_LABEL and e0.label.orig != KEYWORD:
                    self.emit_patch_x(ins, e0.label, cmd, ops)
                else:
                    self.wrong_parameters(cmd, ops)
                return

            # addr, Y
            if self.is_reg(e1, 'Y'):
                ok, addr, prov = self.addr_value(e0)
                if ok:
                    self.emit_addr_y(ins, addr, prov, e0.label, cmd, ops)
                elif e0.type == EVAL_LABEL and e0.label.orig != KEYWORD:
                    self.emit_patch_y(ins, e0.label, cmd, ops)
                else:
                    self.wrong_parameters(cmd, ops)
                return

            self.wrong_parameters(cmd, ops)
            return

        self.not_enough_parameters(cmd)

    def op_bra(self, cmd, ops):
        params = self.split_param(ops)
        if params:
            e = self.eval_op(params[0])
            if e.type == EVAL_LABEL:
                self.emit(BRANCH_OPS[cmd])
                lab = self.add_label(e.label.name, UNDEF)
                self.add_patch(lab, self.org, 1)
                self.emit(0)
            else:
                self.wrong_parameters(cmd, ops)
        else:
            self.not_enough_parameters(cmd)

    # ------------------------------------------------------------------
    # Directives
    # ------------------------------------------------------------------

    def op_abs(self, cmd, ops):
        self.force_abs = True

    def op_include(self, cmd, ops):
        src = ops.strip()
        name = src.strip('"\'')
        try:
            with open(name, 'rb') as f:
                data = f.read()
        except OSError:
            self.error("Failed to load the nested source file: %s" % name)
            return
        text = data.decode('latin-1').replace('\r\n', '\n')
        self.assemble_include(text, name)

    def op_define(self, cmd, ops):
        i = 0
        n = len(ops)
        while i < n and ops[i] > ' ':
            i += 1
        name = ops[:i]
        while i < n and ops[i] <= ' ':
            i += 1
        self.add_define(name, ops[i:])

    def op_byte(self, cmd, ops):
        for param in self.split_param(ops):
            e = self.eval_op(param)
            if e.type == EVAL_LABEL:
                self.error("Label cannot be used here")
            elif e.type == EVAL_NUMBER:
                if e.label is not None:         # forward-referenced immediate: patch later
                    self.add_patch(e.label, self.org, 2)
                    self.emit(0)
                else:
                    self.emit(e.number & 0xFF)
            elif e.type == EVAL_ADDRESS:
                self.emit(e.address & 0xFF)
            elif e.type == EVAL_STRING:
                for c in e.string:
                    self.emit(ord(c) & 0xFF)

    def op_word(self, cmd, ops):
        for param in self.split_param(ops):
            e = self.eval_op(param)
            if e.type == EVAL_STRING:
                self.error("String cannot be used here")
            elif e.type == EVAL_NUMBER:
                if e.label is not None:         # forward-referenced immediate: patch later
                    self.add_patch(e.label, self.org, 0)
                    self.emit(0)
                    self.emit(0)
                else:
                    self.emit(e.number & 0xFF)
                    self.emit((e.number >> 8) & 0xFF)
            elif e.type == EVAL_ADDRESS:
                self.emit(e.address & 0xFF)
                self.emit((e.address >> 8) & 0xFF)
            elif e.type == EVAL_LABEL:
                lab = self.add_label(e.label.name, UNDEF)
                self.add_patch(lab, self.org, 0)
                self.emit(0)
                self.emit(0)
            else:
                self.wrong_parameters(cmd, ops)

    def op_org(self, cmd, ops):
        params = self.split_param(ops)
        if len(params) == 1:
            e = self.eval_op(params[0])
            if e.type == EVAL_ADDRESS:
                self.org = e.address & 0xFFFF
            elif e.type == EVAL_NUMBER:
                self.org = e.number & 0xFFFF
            else:
                ok, addr, prov = self.addr_value(e)
                if ok:
                    self.org = addr & 0xFFFF
                else:
                    self.wrong_parameters(cmd, ops)
        else:
            self.not_enough_parameters(cmd)

    def op_end(self, cmd, ops):
        self.stop = True

    def op_dummy(self, cmd, ops):
        pass

    # ------------------------------------------------------------------
    # Assembling
    # ------------------------------------------------------------------

    def assemble_text(self, text):
        # A trailing newline must not produce an extra (empty) line, as in the C++.
        lines = text.split('\n')
        if lines and lines[-1] == '':
            lines.pop()
        for line in lines:
            label, cmd, op = self.parse_line(line)

            # Add label
            if len(label) > 1:
                self.add_label(label, self.org)

            org_before = self.org

            # Execute command
            if len(cmd) > 1:
                cmd_upper = cmd.upper()
                handler = OPTAB.get(cmd_upper)
                if handler is None:
                    self.error("Unknown command %s" % cmd)
                else:
                    handler(self, cmd_upper, op)
                    # The ABS hint applies to the next CPU instruction only.
                    if handler is Assembler.op_std or handler is Assembler.op_bra:
                        self.force_abs = False
                    if self.stop:
                        break

            # Listing (recorded now, formatted after patching)
            if self.list_file is not None and not self.silent:
                self.listing_lines.append(ListingRec(org_before, self.org,
                                                     label, cmd_upper if len(cmd) > 1 else '', op))

            self.nextline()

    def add_keywords(self):
        self.add_label("A", KEYWORD)
        self.add_label("X", KEYWORD)
        self.add_label("Y", KEYWORD)
        for name in OPTAB:
            self.add_label(name, KEYWORD)

    def snapshot_labels(self):
        self.prev_labels = {}
        for key, lab in self.labels.items():
            self.prev_labels[key] = lab.orig

    def labels_changed(self):
        if len(self.prev_labels) != len(self.labels):
            return True
        for key, lab in self.labels.items():
            if key not in self.prev_labels or self.prev_labels[key] != lab.orig:
                return True
        return False

    def run_pass(self, text, source_name):
        self.prg = bytearray(PRG_SIZE)      # zero the PRG: a previous pass may have
                                            # written bytes at offsets no longer reached
        self.org = 0
        self.stop = False
        self.errors = 0
        self.force_abs = False
        self.labels = {}
        self.patches = []
        self.defines = {}
        self.source_stack = [source_name]
        self.linenum_stack = [0]
        self.add_keywords()
        self.nextline()
        self.assemble_text(text)
        self.do_expr_labels()
        self.do_patch()

    def assemble(self, text, source_name):
        """Multi-pass assembling: run passes until all identifiers, labels and
        defines are resolved and the label values stop changing. The intermediate
        passes run silently; the final pass reports the errors."""
        MAX_PASSES = 100
        self.prev_labels = {}
        self.listing_lines = []

        converged = False
        for pass_num in range(MAX_PASSES):
            self.silent = True
            self.run_pass(text, source_name)
            if pass_num > 0 and not self.labels_changed():
                converged = True
                break
            self.snapshot_labels()

        if not converged:
            print("WARNING: assembly did not converge after %d passes" % MAX_PASSES)

        # Final reporting pass (also generates the listing).
        self.silent = False
        self.run_pass(text, source_name)

        # The listing is flushed after patching, so it shows the final bytes.
        if self.list_file is not None:
            for rec in self.listing_lines:
                self.list_file.write(self.format_listing(rec))
            self.listing_lines = []

        print("%i error(s)" % self.errors)
        return self.errors

    def assemble_include(self, text, source_name):
        self.source_stack.append(source_name)
        self.linenum_stack.append(1)
        self.assemble_text(text)
        self.source_stack.pop()
        self.linenum_stack.pop()

    def format_listing(self, rec):
        if rec.cmd == 'ORG':
            return "$%04X: org %s\n" % (rec.org_after, rec.op)
        num_bytes = rec.org_after - rec.org_before
        line = "$%04X: " % rec.org_before
        for i in range(num_bytes):
            line += "%02X " % self.prg[rec.org_before + i]
        for i in range(num_bytes, 4):
            line += "   "
        if rec.label:
            line += rec.label + ":"
            if rec.cmd:
                line += " %s %s" % (rec.cmd, rec.op)
        else:
            line += "%s %s" % (rec.cmd, rec.op)
        return line + "\n"


# The command table (the port of optab in asm.cpp)
OPTAB = {}
for _name in INSTR:
    OPTAB[_name] = Assembler.op_std
for _name in BRANCH_OPS:
    OPTAB[_name] = Assembler.op_bra
OPTAB.update({
    "INCLUDE": Assembler.op_include,
    "DEFINE": Assembler.op_define,
    "BYTE": Assembler.op_byte,
    "WORD": Assembler.op_word,
    "ORG": Assembler.op_org,
    "END": Assembler.op_end,
    "PROCESSOR": Assembler.op_dummy,
    "ABS": Assembler.op_abs,
})


def usage():
    print("Breakasm, ver. %s" % BREAKASM_VERSION)
    print("Use: breakasm.py [-l <file.lst>] <source.asm> <output.prg>")
    print("Example: breakasm.py -l test.lst test.asm test.prg")
    print("The -l option writes an assembly listing (address, bytes, source line) to the given file.")


def main(argv):
    listing_name = None
    source_name = None
    out_name = None

    i = 1
    while i < len(argv):
        a = argv[i]
        if a == "-l" and i + 1 < len(argv):
            listing_name = argv[i + 1]
            i += 2
        elif source_name is None:
            source_name = a
            i += 1
        elif out_name is None:
            out_name = a
            i += 1
        else:
            usage()
            return -1

    if source_name is None or out_name is None:
        usage()
        return -1

    # Open the listing file, if requested.
    # (The default newline handling translates \n to the platform line ending,
    # like the C++ fopen("w") text mode.)
    list_file = None
    if listing_name is not None:
        try:
            list_file = open(listing_name, "w", encoding="latin-1")
        except OSError:
            print("ERROR: Unable to create the listing file %s." % listing_name)
            return -110
        list_file.write("; Breakasm %s listing: %s -> %s\n"
                        % (BREAKASM_VERSION, source_name, out_name))

    # Load the source
    try:
        with open(source_name, "rb") as f:
            data = f.read()
    except OSError:
        if list_file is not None:
            list_file.close()
        print("ERROR: Unable to read the source code.")
        return -100

    text = data.decode("latin-1").replace("\r\n", "\n")

    # Assemble
    asm = Assembler()
    asm.list_file = list_file
    err_count = asm.assemble(text, source_name)

    if list_file is not None:
        list_file.close()

    if err_count != 0:
        return -200

    # Save the PRG
    try:
        with open(out_name, "wb") as f:
            f.write(bytes(asm.prg))
    except OSError:
        print("ERROR: Failed to save the PRG.")
        return -300

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
