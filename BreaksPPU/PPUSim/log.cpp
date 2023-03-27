// Old-school support for debug output
#include "pch.h"

static FILE* ppu_log_file;

void ppu_log(const char* fmt, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	
	//printf ("%s", buffer);

	if (!ppu_log_file) {
		ppu_log_file = fopen("ppu_log.txt", "wt");
	}
	fprintf(ppu_log_file, "%s", buffer);
	fflush(ppu_log_file);

	va_end(args);
}
