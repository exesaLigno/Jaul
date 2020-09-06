/*!
 *	@file compiler.hpp
 *	@brief Заголовочный файл с описанием класса компилятора
 */

#pragma once

#include <cstring>
#include <fstream>
#include <iostream>

#include "source.hpp"
#include "binary.hpp"

/*!
 *	@brief Класс компилятора
 *	@date Июль 2020 года
 *	@bug Иногда некорректно преобразует исходный код в абстрактное синтаксическое дерево. 
 *		 Пока не понятно, в каких случаях возникает ошибка и как ее исправить.
 *
 *	Основной класс компилятора, созданный для предоставления пользователю удобного api для компиляции/сборки программ. 
 *	Также хранит в себе исходный и скомпилированный код программ и параметры компиляции.
 */
class Compiler
{
  public:	// protected
  
	//------------- SETTINGS -------------//
	char** source_pathes = nullptr;
	int source_count = 0;
	char* output_path = nullptr;
	
	bool verbose = false;
	
	bool only_preprocess = false;
	bool nasm_listing = false;
	bool virtual_compilation = false;
	bool obj_generation = false;
	bool hex_view = false;
	bool executable = false;
	
	bool show_help = false;
	
	bool feature = false;
	
	short optimization_level = 2;
	
	
	bool include_stdio = true;
	bool include_stdlib = true;
	
	short int status = OK;
	//------------------------------------//
	
	Source** source_list = nullptr;	///< Массив ссылок на объекты исходных кодов
	Binary* binary = nullptr;	///< Ссылка на объект скомпилированного кода
	
  public:
	
	//! Ошибки
	enum Errors
	{
		OK,					///< Ошибок при выполнении не было
		ARGUMENT_ERROR,		///< Неверный ключ компиляции
		INPUT_FILE_ERROR,	///< Указан некорректный входной тип данных
		COMPILATION_ERROR,	///< Ошибка компиляции
		LINKER_ERROR,		///< Ошибка сброки программы
		EXPORT_ERROR,		///< Ошибка экспорта
		ERRORS_COUNT		///< Количество ошибок
	};
	
	Compiler(int argc, char* argv[]);
	~Compiler();
	
	void addPath(const char* source_path);
	void showSettings();
	void showHelp();
	
	void readSource();
	void showSource();
	
	void makeAST();
	void dumpAST();
	
	void compile();
	void write();
};

