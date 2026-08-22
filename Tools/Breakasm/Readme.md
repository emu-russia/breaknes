# Breakasm

![breakasm_logo](breakasm_logo.png)

As simple and dumb assembler as possible, to generate code.

To run:

```
Breakasm [-l <file.lst>] <source.asm> <output.prg>
Example: Breakasm -l test.lst test.asm test.prg
```

The `-l <file.lst>` option writes an assembly listing (address, emitted bytes and the source line) to the given file.

There is also a Python port with the same behaviour and command line:

```
python breakasm.py [-l <file.lst>] <source.asm> <output.prg>
```

PRG file is always 64 Kbytes (the size of 6502 address space). The current assembly pointer (`ORG`) can be set anywhere in the PRG.

## Syntax

The source text is split into lines of the following format:

```
[LABEL:] COMMAND [OPERAND1, OPERAND2, OPERAND3] ; Comments
```

The label (`LABEL`) is optional. The command (`COMMAND`) contains 6502 instruction or one of the assebmler directives. The operands depend on the command.

## Expressions

All operands are evaluated with the built-in expression engine. Labels, defines and complex expressions are supported in every operand position:

```
LDA     MyData + 32 * entry_size + 12
STA     (Base << 4) | 1
BYTE    Table + 2
```

Supported operations (from high to low priority):

```
()      grouping
! ~     logical not, bitwise not (unary)
* / %   multiply, divide, modulo
+ -     add, subtract
<< >>   shift left, shift right
<<< >>> rotate left, rotate right
> >= < <=  comparisons (result: 0 or 1)
== !=   equality (result: 0 or 1)
& | ^   bitwise and, or, xor
```

Numbers can be decimal (`12`), hexadecimal (`$12`, `#$12`) or binary via defines. An immediate operand starts with `#` (`LDA #5`). A whole operand wrapped in parentheses is indirect addressing (`JMP (addr)`), while parentheses inside an expression are grouping only.

## Multi-pass assembling

Breakasm assembles the source in several passes until all identifiers, labels and defines are resolved. This also resolves the Zero Page / Absolute ambiguity: if a forward-referenced label turns out to be below `$100`, the assembler automatically uses the shorter zero page opcode (unless the `ABS` directive forces the absolute one).

## Embedded Directives

|Directive|Description|
|---|---|
|ORG|Set the current PRG assembly position.|
|INCLUDE|Process a nested source file|
|DEFINE|Define a simple constant|
|BYTE|Output a byte or string|
|WORD|Output uint16_t in little-endian order. You can use both numbers as well as labels and addresses.|
|END|Finish the assembling|
|PROCESSOR|Defines type of processor for informational purposes|
|ABS|Hint: the next instruction with a Zero Page / Absolute ambiguity (address below `$100`) must use the absolute opcode|

Example of the `ABS` directive:

```asm
        ABS
        LDA     $10         ; uses AD 10 00 (absolute), not A5 10 (zero page)
        LDA     $20         ; the hint is consumed by the previous instruction: A5 20 (zero page)
```

## Example Source Code

Not to write too much, I will just show you an example of the source code. Do it the same way and it should work.

```asm
; Test program

LABEL1:

    PROCESSOR 6502
;    ORG     $100

    DEFINE  KONST   #5

    LDX     KONST

AGAIN:
    NOP
    LDA     SOMEDATA, X         ; Load some data
    JSR     ADDSOME             ; Call sub
    STA     $12, X
    CLC
    BCC     AGAIN               ; Test branch logic

ADDSOME:                        ; Test ALU
    ADC     KONST
    PHP                         ; Test flags in/out
    PLP
    RTS

    ASL     A

SOMEDATA:
    BYTE    12, $FF, "Hello, world" 
    WORD    AGAIN

END
```

## Support for NES and mappers

Dragging some NES and mapper prisserans into a generalized assembler is a bad idea.

Build multiple PRG files and compile a .nes file from them yourself if you need to.
