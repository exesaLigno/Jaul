#include "../Headers/settings.hpp"

Settings::Settings(int argc, char* argv[])
{
	for (int counter = 1; counter < argc; counter++)
	{
		if (!strcmp("-s", argv[counter]) or !strcmp("--silent", argv[counter]))
			this -> silent = true;
		
		else if (!strcmp("-p", argv[counter]) or !strcmp("--preprocess", argv[counter]))
			this -> only_preprocess = true;
		
		else if (!strcmp("-l", argv[counter]) or !strcmp("--listing", argv[counter]))
			this -> nasm_listing = true;
		
		else if (!strcmp("-o", argv[counter]) or !strcmp("--output", argv[counter]))
		{
			counter++;
			this -> output_path = new char[strlen(argv[counter]) + 1]{0};
			strcpy(this -> output_path, argv[counter]);
		}
		
		else if (strstr(argv[counter], ".jaul") != NULL)
		{
			this -> source_path = new char[strlen(argv[counter]) + 1]{0};
			strcpy(this -> source_path, argv[counter]);
		}
		
		else if (!strcmp("-O0", argv[counter]) or !strcmp("-O1", argv[counter]) or !strcmp("-O2", argv[counter]))
			this -> optimization_level = argv[counter][2] - '0';
		
		else
			printf("\x1b[34mUnknow parameter \x1b[1;34m<%s>\x1b[34m, ignoring...\x1b[0m\n", argv[counter]);
	}
}



Settings::~Settings()
{
	if (this -> source_path)
		delete[] source_path;
		
	if (this -> output_path)
		delete[] output_path;
}




