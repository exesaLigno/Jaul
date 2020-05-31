#include <fstream>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include "Headers/programm.hpp"
#include "Headers/settings.hpp"


/*----------------------------------------------|
| TODO 	In programm.cpp - symbol by symbol		|
|		printing								|
|----------------------------------------------*/

#include "Headers/debug.hpp"

int main(int argc, char* argv[])
{
	Settings settings(argc, argv);

	Programm programm;

	programm.readSource(&settings);
	programm.preprocessor(&settings);
	
	if (settings.only_preprocess)
	{
		programm.write(&settings);
		return 0;
	}
	
	programm.makeTree(&settings);
		
	if (settings.optimization_level > 0)
		programm.optimizeTree(&settings);

	programm.makeNasm(&settings);
	
	if (settings.optimization_level > 1)
		programm.optimizeNasm(&settings);
	
	programm.write(&settings);
	
    return 0;
}








// end;
