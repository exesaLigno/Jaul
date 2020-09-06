#include <cstdio>
#include <cstring>
#include <cstdlib>

int main(int argc, char* argv[])
{
	if (argc == 1)
		printf("Usage: ./makeobj file1 file2... output\nType only file names without extension\n");
	
	FILE* output = fopen(argv[argc - 1], "w");
	
	for (int counter = 1; counter < argc - 1; counter++)
	{
		char* func_name = strrchr(argv[counter], '/');
		if (func_name == nullptr)
			func_name = argv[counter];
		else
			func_name += 1;
		
		char* command = new char[strlen("nasm -f elf64 .s -o .o") + 2 * strlen(argv[counter]) + 1];
		sprintf(command, "nasm -f elf64 %s.s -o %s.o", argv[counter], argv[counter]);
		system(command);
		
		sprintf(command, "ld -s %s.o -o %s", argv[counter], argv[counter]);
		system(command);
		
		FILE* compiled_nasm = fopen(argv[counter], "r");
		
		fseek(compiled_nasm, 0, SEEK_END);
		long int text_length = ftell(compiled_nasm);
		rewind(compiled_nasm);
		
		char* text = new char[text_length + 1]{0};
		
		text_length = fread(text, sizeof(char), text_length, compiled_nasm);
		
		fclose(compiled_nasm);
		
		char* progbits = text + 0x1000;
		char* _progbits = progbits;
		
		while (strcmp(_progbits, ".shstrtab"))
			_progbits++;
		
		_progbits--;
		
		long int progbits_length = _progbits - progbits;
		int name_length = strlen(func_name);

		//	fwrite(elf.text, sizeof(char), elf.text_length, executable);
		
		fwrite(&name_length, sizeof(int), 1, output);
		fwrite(func_name, sizeof(char), name_length, output);
		fwrite(&progbits_length, sizeof(long int), 1, output);
		fwrite(progbits, sizeof(char), progbits_length, output);
		
		sprintf(command, "rm %s %s.o", argv[counter], argv[counter]);
		system(command);
		
		delete[] text;
		delete[] command;
	}
}
