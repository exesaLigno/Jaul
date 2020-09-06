#include "../Headers/compiler.hpp"

/*!
 *	@file compiler.cpp
 *	@brief Файл с исполняемым кодом класса Compiler
 */
 
/*!
 *	@brief Конструктор класса Compiler
 *	@param[in] argc Количество аргументов командной строки
 *	@param[in] argv Парметры командной строки
 *
 *	Конструирует класс на основе переданных ключей запуска, выставляет параметры запуска и 
 *	сохраняет имена исходников.
 */
Compiler::Compiler(int argc, char* argv[])
{
	if (argc <= 1)
		this -> status = ARGUMENT_ERROR;
	
	else
	{
		for (int counter = 1; counter < argc; counter++)
		{
			if (!strcmp("-v", argv[counter]) or !strcmp("--verbose", argv[counter]))
				this -> verbose = true;
			
			else if (!strcmp("-p", argv[counter]) or !strcmp("--only-preprocess", argv[counter]))
				this -> only_preprocess = true;
			
			else if (!strcmp("-l", argv[counter]) or !strcmp("--nasm-listing", argv[counter]))
				this -> nasm_listing = true;
				
			else if (!strcmp("--virtual", argv[counter]))
			{
				this -> virtual_compilation = true;
				this -> include_stdio = false;
				this -> include_stdlib = false;
			}
				
			else if (!strcmp("-h", argv[counter]) or !strcmp("--help", argv[counter]))
				this -> show_help = true;
				
			else if (!strcmp("--obj", argv[counter]))
				this -> obj_generation = true;
				
			else if (!strcmp("--hex-view", argv[counter]))
				this -> hex_view = true;
			
			else if (!strcmp("--feature", argv[counter]))
				this -> feature = true;
			
			else if (!strcmp("--disable-stdio", argv[counter]))
				this -> include_stdio = false;
			
			else if (!strcmp("--disable-stdlib", argv[counter]))
				this -> include_stdlib = false;
			
			else if (!strcmp("--disable-std", argv[counter]))
			{
				this -> include_stdio = false;
				this -> include_stdlib = false;
			}
			
			else if (!strcmp("-o", argv[counter]))
			{
				counter++;
				this -> output_path = new char[strlen(argv[counter]) + 1]{0};
				strcpy(this -> output_path, argv[counter]);
			}
			
			else if (!strcmp("-o0", argv[counter]) or !strcmp("-o1", argv[counter]) or !strcmp("-o2", argv[counter]) or !strcmp("-o3", argv[counter]))
				this -> optimization_level = argv[counter][2] - '0';
			
			else if (argv[counter][0] != '-')
				this -> addPath(argv[counter]);
				
			else
			{
				printf("\x1b[1;31merror:\x1b[0m unknown parameter \x1b[3m\'%s\'\x1b[0m\n", argv[counter]);
				this -> status = ARGUMENT_ERROR;
				return;
			}
		}
		
		if (not (this -> only_preprocess or this -> nasm_listing or this -> virtual_compilation or 
			this -> obj_generation or this -> hex_view))
		{
			this -> executable = true;
		}
		
		if (not (this -> only_preprocess xor this -> nasm_listing xor this -> virtual_compilation xor 
			this -> obj_generation xor this -> hex_view xor  this -> executable))
		{
			printf("\x1b[1;31merror:\x1b[0m Keys \x1b[3m--only-preprocess\x1b[0m, \x1b[3m--nasm-listing\x1b[0m, \x1b[3m--virtual\x1b[0m, \x1b[3m--obj\x1b[0m, \x1b[3m--hex-view\x1b[0m can'not be combined!\n");
			this -> status = ARGUMENT_ERROR;
			return;
		}
		
		if (this -> output_path == nullptr)
		{
			char* name = nullptr;
			char* default_name = new char[7]{};
			strcpy(default_name, "output");
			
			if (this -> source_count == 1)
				name = (this -> source_pathes)[0];
				
			else
				name = default_name;
			
			this -> output_path = new char[strlen(name) + 5]{0};
			strcpy(this -> output_path, name);
			
			char* extension = strrchr(this -> output_path, '.');	// Excluding old extension
			if (extension != nullptr)
			{
				while (*extension != '\0')
					*(extension++) = '\0';
			}
			
			if (this -> only_preprocess)
				strcat(this -> output_path, ".j");
				
			else if (this -> nasm_listing)
				strcat(this -> output_path, ".s");
				
			else if (this -> virtual_compilation)
				strcat(this -> output_path, ".jv");
				
			else if (this -> obj_generation)
				strcat(this -> output_path, ".jo");
				
			else if (this -> hex_view)
				strcat(this -> output_path, ".hex");
			
			delete[] default_name;
		}
		
		if (this -> source_count == 0)
			this -> status = INPUT_FILE_ERROR;
		
		
		if (this -> include_stdio)
			this -> addPath(".std/stdio.jo");
		
		if (this -> include_stdlib)
			this -> addPath(".std/stdlib.jo");
	}
}


/*!
 *	@brief Деструктор класса Compiler
 */
Compiler::~Compiler()
{
	if (this -> source_pathes != nullptr)
	{
		for (int counter = 0; counter < this -> source_count; counter++)
		{
			if ((this -> source_pathes)[counter])
				delete[] (this -> source_pathes)[counter];
		}
		delete[] this -> source_pathes;
	}

	if (this -> source_list != nullptr)
	{
		for (int counter = 0; counter < this -> source_count; counter++)
		{
			if ((this -> source_list)[counter])
				delete (this -> source_list)[counter];
		}
		delete[] this -> source_list;
	}
	
	if (this -> binary)
		delete (this -> binary);

	if (this -> output_path != nullptr)
		delete[] this -> output_path;
}



/*!
 *	@brief Функция добавления нового файла в список исходников
 *	@param[in] source_path Имя исходника
 *
 *	Расширяет массив имен исходников на 1 элемент и копирует в этот элемент имя исходника. Так же увеличивает счетчик исходников на 1.
 */
void Compiler::addPath(const char* source_path)
{
	int new_source_count = this -> source_count + 1;
	char** new_source_pathes = new char*[new_source_count];
	
	for (int counter = 0; counter < this -> source_count; counter++)
		new_source_pathes[counter] = (this -> source_pathes)[counter];
	
	new_source_pathes[this -> source_count] = new char[strlen(source_path) + 1];
	strcpy(new_source_pathes[this -> source_count], source_path);
	
	delete[] this -> source_pathes;
	this -> source_pathes = new_source_pathes;
	this -> source_count = new_source_count;
}


/*!
 *	@brief Показ настроек компилятора
 */
void Compiler::showSettings()
{
	if (not this -> verbose)
		return;
	
	printf("\n---------------\x1b[1;32m Compiler \x1b[0m---------------\n");
	
	if (this -> source_pathes)
	{
		printf("Source list: ");
		for (int counter = 0; counter < this -> source_count; counter++)
			printf("%s ", (this -> source_pathes)[counter]);
		printf("\n");
	}
	else
		printf("No sources listed!\n");
	
	if (this -> output_path)
		printf("Output path: %s\n", this -> output_path);
	else
		printf("Using default output path\n");
	
	printf("%s", this -> verbose ? "Verbose\n" : "");
	
	printf("%s", this -> only_preprocess ? "Preprocessor\n" : "");
	printf("%s", this -> nasm_listing ? "Nasm listing\n" : "");
	printf("%s", this -> obj_generation ? "Obj generation\n" : "");
	
	printf("%s", this -> virtual_compilation ? "Virtual compilation\n" : "");
	
	printf("%s", this -> hex_view ? "Hex view\n" : "");
	
	printf("%s", this -> feature ? "\x1b[1;31mFeature testing\x1b[0m\n" : "");
	
	printf("Optimization level: %d\n", this -> optimization_level);
	
	printf("----------------------------------------\n\n");
}


/*!
 *	@brief Показ страницы помощи
 */
void Compiler::showHelp()
{
	if (not this -> show_help and this -> status == OK)
		return;
	
	printf("Jaul Compiler (2020)\n");
	printf("Usage: ./jc [options] file...\n");
	printf("Options:\n\n");
	printf("  -v --verbose             Detailed compilation process output\n\n");
	printf("  \x1b[9m-p --only-preprocess     Only preprocess code and save it\x1b[0m\n");
	printf("  -l --nasm-listing        Generate NASM listing\n");
	printf("     --obj                 Generate jaul object file\n");
	printf("     --virtual             Compile code to virtual executable\n");
	printf("     --hex-view            Generate hex view of compiled code \x1b[2m(only for debugging)\x1b[0m\n\n");
	printf("  -o \x1b[2m<file>\x1b[0m                Write compiled code to \x1b[2m<file>\x1b[0m. If not specified, using \x1b[2moutput\x1b[0m\n\n");
	printf("  -o#                      Optimization level, # = 0, 1 or 2\n");
	printf("                              0 - No optimizations at all\n");
	printf("                              1 - Only on-tree optimizations\n");
	printf("                              2 - Assembler and byte-code optimizations\n");
	printf("                              3 - Pseudo-dynamic tipization, int data\n\n");
	printf("     --feature             Enable all experimental functions\n");
}



/*!
 *	@brief Оболочка для вызова метода Source::open()
 *	
 *	Для каждого объекта исходного кода вызывает метод open(), тем самым читая текст каждого их исходников и сохраняя его в
 *	этом же объекте исходника.
 */
void Compiler::readSource()
{
	this -> source_list = new Source*[this -> source_count];
	for (int counter = 0; counter < this -> source_count; counter++)
	{
		(this -> source_list)[counter] = new Source((this -> source_pathes)[counter]);
		(this -> source_list)[counter] -> open();
		if ((this -> source_list)[counter] -> status != Source::OK)
			this -> status = INPUT_FILE_ERROR;
	}
}


/*!
 *	@brief Показ всех считанных исходных кодов
 *	
 *	Обходит все объекты класса Source и показывает считанный исходный код.
 */
void Compiler::showSource()
{		
	if (this -> source_list and this -> verbose)
	{
		for (int counter = 0; counter < this -> source_count; counter++)
			(this -> source_list)[counter] -> print();
		
		printf("\n\n");
	}
}


/*!
 *	@brief Оболочка для вызова метода Source::makeAST()
 *	
 *	Для каждого объекта Source, для которого это возможно, создает абстрактное синтаксическое дерево.
 */
void Compiler::makeAST()
{
	if (this -> source_list)
	{
		for (int counter = 0; counter < this -> source_count; counter++)
		{
			if ((this -> source_list)[counter] -> source_type == Source::JAUL_SOURCE)
			{
				(this -> source_list)[counter] -> makeAST();
				
				if (this -> optimization_level > 0)
					(this -> source_list)[counter] -> optimizeAST();
				
				(this -> source_list)[counter] -> prepareAST();
				
				if (this -> optimization_level > 2)
				{
					(this -> source_list)[counter] -> setTypes();
					//(this -> source_list)[counter] -> optimizeAST(); // FIXME After type setting optimizing corrupting all arithmetical operations
				}
				
				else
					(this -> source_list)[counter] -> setFloats();
				
				if ((this -> source_list)[counter] -> status != Source::OK)
					this -> status = COMPILATION_ERROR;
			}
		}
	}
}



/*!
 *	@brief Дамп синтаксических деревьев в файлы
 *	
 *	Для каждого исходника, для которого это возможно, сохраняет графический дамп дерева в файл.
 */
void Compiler::dumpAST()
{
	if (not this -> verbose)
		return;
		
	bool dumped = false;
	
	if (this -> source_list)
	{
		for (int counter = 0; counter < this -> source_count; counter++)
		{
			if ((this -> source_list)[counter] -> source_type == Source::JAUL_SOURCE)
			{
				char* title = new char[strlen((this -> source_list)[counter] -> name) + strlen(" -o#") + 1]{};
				strcpy(title, strrchr((this -> source_list)[counter] -> name, '/') != nullptr ? 
								  strrchr((this -> source_list)[counter] -> name, '/') + 1 : 
								  (this -> source_list)[counter] -> name);
				
				strcat(title, this -> optimization_level == 0 ? " -o0" :
							  this -> optimization_level == 1 ? " -o1" :
							  this -> optimization_level == 2 ? " -o2" :
							  this -> optimization_level == 3 ? " -o3" : " fck");
				
				(this -> source_list)[counter] -> dumpAST(title);
				dumped = true;
			}
		}
	}
	
	if (not dumped)
		printf("No abstract syntax tree builded! Maybe you not listed .j file with code or trying to compile code to object library?\n");
}



/*!
 *	@brief Компиляция на базе всех объектов Source
 *	
 */
void Compiler::compile()
{
	this -> binary = new Binary;
	
	for (int counter = 0; counter < source_count; counter++)
	{
		if ((this -> source_list)[counter] -> source_type == Source::JAUL_SOURCE)
			this -> binary -> importAST((this -> source_list)[counter] -> ast);
		
		else if ((this -> source_list)[counter] -> source_type == Source::JASM_SOURCE)
			this -> binary -> importNasm((this -> source_list)[counter] -> text);
		
		else if ((this -> source_list)[counter] -> source_type == Source::JAUL_OBJ)
			this -> binary -> importObj((this -> source_list)[counter] -> text, (this -> source_list)[counter] -> text_length);
	}
	
	this -> binary -> storeLabels();
	
	if (this -> optimization_level > 1)
		this -> binary -> optimize();
	
	this -> binary -> compile();	
	
	this -> binary -> setLabels();
}



/*!
 *	@brief Экспорт полученного кода в файл
 */
void Compiler::write()
{
	if (this -> only_preprocess)
		printf("preproc is not implemented!\n");
		
	else if (this -> nasm_listing)
		this -> binary -> exportNasm(this -> output_path);
		
	else if (this -> virtual_compilation)
		this -> binary -> exportVirtualExecutable(this -> output_path);
		
	else if (this -> obj_generation)
		this -> binary -> exportObj(this -> output_path);
		
	else if (this -> hex_view)
		this -> binary -> exportHex(this -> output_path);
		
	else
		this -> binary -> exportExecutable(this -> output_path);

	printf("\x1b[1m%s%s%s%s%s%s\x1b[0m successfully exported to \x1b[1;32m<%s>\x1b[0m\n",
			this -> only_preprocess ? "Preprocessed source" : "",
			this -> nasm_listing ? "Nasm listing" : "",
			this -> virtual_compilation ? "Virtual executable" : "",
			this -> obj_generation ? "Object file" : "",
			this -> hex_view ? "Hex view of executable" : "",
			this -> executable ? "Executable" : "",
			this -> output_path);
}


