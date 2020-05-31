#include "../Headers/library.hpp"


Library::Library()
{
	libname = 0;
	libname_length = 0;
	libtext = 0;
	libtext_length = 0;
}

Library::~Library()
{	
	if (libname)
		delete[] libname;
	if (libtext)
		delete[] libtext;
}

int Library::readLibrary()
{
	if (strcmp(libname + (strlen(libname) - 5), ".jaul"))
	{
		printf("\x1b[1;31mError\x1b[0m: file \x1b[1m\"%s\"\x1b[0m is not a library.\n", libname);
		exit(WRONG_EXTENSION);
	}
	
	FILE* source_file = fopen(libname, "r");
	
	if (not source_file)
	{
		printf("\x1b[1;31mError\x1b[0m: library \x1b[1m\"%s\"\x1b[0m not exist.\n", libname);
		exit(FILE_NOT_EXIST);
	}
	
	fseek(source_file, 0, SEEK_END);
	libtext_length = ftell(source_file);
	rewind(source_file);

	libtext = new char[libtext_length + 1]{0};
	(libtext)[libtext_length] = '\0';

	libtext_length = fread(libtext, sizeof(char), libtext_length, source_file);
	
	fclose(source_file);

	return libtext_length;
}


