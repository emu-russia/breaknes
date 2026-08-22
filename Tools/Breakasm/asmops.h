#pragma once

// Unified table-driven handler for all CPU instructions (implied, immediate, memory, jumps).
void op_std(char* cmd, char* ops);

// Relative branches (BPL, BMI, BVC, BVS, BCC, BCS, BNE, BEQ).
void opBRA(char* cmd, char* ops);

// Directives
void opINCLUDE(char* cmd, char* ops);
void opDEFINE(char* cmd, char* ops);
void opBYTE(char* cmd, char* ops);
void opWORD(char* cmd, char* ops);
void opORG(char* cmd, char* ops);
void opEND(char* cmd, char* ops);
void opDUMMY(char* cmd, char* ops);

// The ABS hint directive: the next instruction with a Zero Page / Absolute ambiguity
// must use the absolute (or absolute indexed) opcode.
void opABS(char* cmd, char* ops);
