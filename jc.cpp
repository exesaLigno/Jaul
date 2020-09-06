#include <fstream>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include "Headers/compiler.hpp"


/*!
 *	@file jc.cpp
 *	@author Карцев Вадим
 *	@brief Основной файл компилятора
 */

/*!
 *	@brief Валидация состояния компилятора
 */
#define validate 								\
		if (compiler.status != Compiler::OK)	\
			return compiler.status;				\


#include "Headers/debug.hpp"


/*!
 *	Основная логика компилятора
 *	@param[in] argc Количество параметров командной строки
 *	@param[in] argv Параметры командной строки
 *	@return Статус работы компилятора
 */
int main(int argc, char* argv[])
{
	Compiler compiler(argc, argv);
	
	if (compiler.status == Compiler::ARGUMENT_ERROR or compiler.show_help == true)
		compiler.showHelp();
	
	else
	{
		compiler.showSettings();
		
		compiler.readSource();
		validate;		
		compiler.showSource();
		
		compiler.makeAST();
		validate;
		
		compiler.dumpAST();
		
		compiler.compile();
		validate;
		
		compiler.write();
	}

    return compiler.status;
}








// end;
