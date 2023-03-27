// Old-school support for debug output

#pragma once

#define PPU_LOG_DISABLE_ALL 1

#define PPU_LOG_LEVEL_REGS		1
#define PPU_LOG_LEVEL_HV		1
#define PPU_LOG_LEVEL_HVDEC		1
#define PPU_LOG_LEVEL_FSM		1
#define PPU_LOG_LEVEL_CRAM		1
#define PPU_LOG_LEVEL_VOUT		1
#define PPU_LOG_LEVEL_MUX		1
#define PPU_LOG_LEVEL_OAM		1
#define PPU_LOG_LEVEL_EVAL		1
#define PPU_LOG_LEVEL_FIFO		1
#define PPU_LOG_LEVEL_SCCX		1
#define PPU_LOG_LEVEL_BGC		1
#define PPU_LOG_LEVEL_PAR		1
#define PPU_LOG_LEVEL_VRAM		1

#if PPU_LOG_DISABLE_ALL
#define PPU_LOG_LEVEL 0
#else
#define PPU_LOG_LEVEL (PPU_LOG_LEVEL_REGS|PPU_LOG_LEVEL_HV|PPU_LOG_LEVEL_HVDEC|PPU_LOG_LEVEL_FSM|PPU_LOG_LEVEL_CRAM|PPU_LOG_LEVEL_VOUT|PPU_LOG_LEVEL_MUX|PPU_LOG_LEVEL_OAM|PPU_LOG_LEVEL_EVAL|PPU_LOG_LEVEL_FIFO|PPU_LOG_LEVEL_SCCX|PPU_LOG_LEVEL_BGC|PPU_LOG_LEVEL_PAR|PPU_LOG_LEVEL_VRAM)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_REGS
#define PPU_LOG_REGS(cond, fmt, ...) if(cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_REGS(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_HV
#define PPU_LOG_HV(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_HV(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_HVDEC
#define PPU_LOG_HVDEC(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_HVDEC(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_FSM
#define PPU_LOG_FSM(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_FSM(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_CRAM
#define PPU_LOG_CRAM(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_CRAM(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_VOUT
#define PPU_LOG_VOUT(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_VOUT(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_MUX
#define PPU_LOG_MUX(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_MUX(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_OAM
#define PPU_LOG_OAM(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_OAM(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_EVAL
#define PPU_LOG_EVAL(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_EVAL(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_FIFO
#define PPU_LOG_FIFO(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_FIFO(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_SCCX
#define PPU_LOG_SCCX(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_SCCX(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_BGC
#define PPU_LOG_BGC(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_BGC(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_PAR
#define PPU_LOG_PAR(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_PAR(cond, fmt, ...)
#endif

#if PPU_LOG_LEVEL & PPU_LOG_LEVEL_VRAM
#define PPU_LOG_VRAM(cond, fmt, ...) if (cond) ppu_log(fmt, ##__VA_ARGS__)
#else
#define PPU_LOG_VRAM(cond, fmt, ...)
#endif

void ppu_log(const char* fmt, ...);
