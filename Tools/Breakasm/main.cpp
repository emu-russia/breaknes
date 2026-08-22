// The main file with the entry point.

#include "pch.h"

#define PRG_SIZE 0x10000

void Usage()
{
	printf("Breakasm, ver. %s\n", BREAKASM_VERSION);
	printf("Use: Breakasm [-l <file.lst>] <source.asm> <output.prg>\n");
	printf("Example: Breakasm -l test.lst test.asm test.prg\n");
	printf("The -l option writes an assembly listing (address, bytes, source line) to the given file.\n");
}

void test_expr_eval()
{
	const char* text = "  SPR_TAB+1";
	//const char* text = " $700";
	add_define((char *)"SPR_TAB", (char*)"$700");
	//add_label("ItemID", 5);
	auto res = eval_expr((char*)text, true, false);
}

int main(int argc, char** argv)
{
	FILE* f;
	const char* listing_name = NULL;
	char* source_name = NULL;
	char* out_name = NULL;

	// Parse the command line: [-l <file.lst>] <source.asm> <output.prg>
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-l") && i + 1 < argc) {
			listing_name = argv[++i];
		}
		else if (!source_name) source_name = argv[i];
		else if (!out_name) out_name = argv[i];
		else {
			Usage();
			return -1;
		}
	}

	//test_expr_eval();
	//return 0;

	if (!source_name || !out_name)
	{
		Usage();
		return -1;
	}

	uint8_t* prg = new uint8_t[PRG_SIZE];
	memset(prg, 0, PRG_SIZE);

	// Open the listing file, if requested.

	if (listing_name) {
		list_file = fopen(listing_name, "w");
		if (!list_file) {
			delete[] prg;
			printf("ERROR: Unable to create the listing file %s.\n", listing_name);
			return -110;
		}
		fprintf(list_file, "; Breakasm %s listing: %s -> %s\n", BREAKASM_VERSION, source_name, out_name);
	}

	// Load Source

	f = fopen(source_name, "rt");
	if (!f)
	{
		if (list_file) { fclose(list_file); list_file = NULL; }
		delete[] prg;
		printf("ERROR: Unable to read the source code.\n");
		return -100;
	}

	// One more byte of memory is allocated to complete the text with the null character (END).

	fseek(f, 0, SEEK_END);
	long size = ftell(f) + 1;
	fseek(f, 0, SEEK_SET);

	char* text = new char[size];
	memset(text, 0, size);

	size_t readSize = fread(text, 1, size, f);
	fclose(f);
	if (readSize >= size)
	{
		delete[] text;
		if (list_file) { fclose(list_file); list_file = NULL; }
		delete[] prg;
		printf("ERROR: Error loading the source file.\n");
		return -101;
	}

	// Assemble

	int err_count = assemble(text, source_name, prg);

	if (list_file) {
		fclose(list_file);
		list_file = NULL;
	}

	if (err_count != 0)
	{
		delete[] text;
		delete[] prg;
		return -200;
	}

	// Save PRG

	f = fopen(out_name, "wb");
	if (!f)
	{
		delete[] text;
		delete[] prg;
		printf("ERROR: Failed to save the PRG.\n");
		return -300;
	}

	fwrite(prg, 1, PRG_SIZE, f);
	
	delete[] text;
	delete[] prg;

	fclose(f);

	return 0;
}
