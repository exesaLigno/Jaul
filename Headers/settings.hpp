#pragma once
#include <cstring>
#include <fstream>
#include <iostream>

class Settings
{
  public:
	char* source_path = NULL;
	char* output_path = NULL;
	bool silent = false;
	bool only_preprocess = false;
	bool nasm_listing = false;
	short optimization_level = 0;
	
	Settings(int argc, char* argv[]);
	~Settings();
};

