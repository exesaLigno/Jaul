/*!
 *	@file elf.hpp
 *	@brief Описание класса ELF и его константы
 */

#pragma once

#include <cstring>
#include <cstdio>

typedef unsigned char byte;

class ELF
{
  public:
	class FileHeader;
	class ProgramHeader;
	class SectionHeader;

	FileHeader* file_header = nullptr;			///< Заголовок ELF64 файла
	
	ProgramHeader* program_headers = nullptr;	///< Массив заголовков программ
	int program_headers_count = 0;				///< Количество заголовков программ
	
	SectionHeader* section_headers = nullptr;	///< Массив заголовков секций
	int section_headers_count = 0;				///< Количество заголовков секций
	
	
	char* text = nullptr;
	unsigned long int text_length = 0;

	
	char* text_section = nullptr;				///< Скомпилированный исполняемый код (.text)
	unsigned long int text_section_length = 0;	///< Длина исполняемого кода .text

	char* bss_section = nullptr;				///< Данные из секции .bss
	unsigned long int bss_section_length = 0;	///< Длина секции .bss

	char* data_section = nullptr;				///< Данные из секции .data
	unsigned long int data_section_length = 0;	///< Длина секции .data

	void addText(char* text_section, unsigned long int text_section_length);
	void addBss(char* bss_section, unsigned long int bss_section_length);
	void addData(char* data_section, unsigned long int data_section_length);
	
	void assemble();
	
	void addBin(const int bytes_count, const int number);
	void addBin(const char* string);
	void substituteNumber(long long int position, int bytes_count, int number);
	void makeHeader();
	
	void write(const char* filename);

	ELF(char* text, unsigned long int text_length);
	~ELF();
};


class ELF::FileHeader
{
	friend ELF;

  public:

	//! Константы заголовка файла
	enum Constants
	{
		ELFCLASSNONE = 0,						///< Некорректный класс
		ELFCLASS32 = 1,							///< 32-битный объектный файл
		ELFCLASS64 = 2,							///< 64-битный объектный файл

		ELFDATANONE = 0,						///< Некорректный тип
		ELFDATA2LSB = 1,						///< Little Endian
		ELFDATA2MSB = 2,						///< Big Endian

		EV_NONE = 0,							///< Устаревшая версия
		EV_CURRENT = 1,							///< Текущая версия

		ELFOSABI_GNU = 3,						///< GNU/Linux формат

		ET_NONE = 0,							///< Неопределенный тип
		ET_REL = 1,								///< Перемещаемый файл
		ET_EXEC = 2,							///< Исполняемый файл
		ET_DYN = 3,								///< Разделяемый объектный файл
		ET_CORE = 4,							///< Core file

		EM_NONE = 0,							///< Неопределенный тип платформы
		EM_386 = 3,								///< Intel 80386
		EM_PPC = 20,							///< PowerPC
		EM_PPC64 = 21,							///< 64-bit PowerPC
		EM_X86_64 = 62							///< x86-64
	};

	byte EI_MAG[4] = {0x7f, 0x45, 0x4c, 0x46};	///< Магические числа заголовка ELF
	byte EI_CLASS = ELFCLASS64;					///< Разрядность скомпилированной программы
	byte EI_DATA = ELFDATA2LSB; 				///< Big или Little Endian
	byte EI_VERSION = EV_CURRENT;				///< Номер версии формата ELF
	byte EI_OSABI = ELFOSABI_GNU;				///< ABI расширение
	byte EI_ABIVERSION = 0;						///< Версия ABI
	byte EI_PAD[7] = {};						///< Bytes Padding (резервные байты на будущее)

	unsigned short int e_type = ET_EXEC;		///< Тип ELF файла
	unsigned short int e_machine = EM_X86_64;	///< Архитектура платформы
	unsigned int e_version = EV_CURRENT;		///< Номер версии формата ELF

	unsigned long int e_entry = 0x401000;		///< Виртуальный адрес точки входа
	unsigned long int e_phoff = 0x40;			///< Смещение таблицы заголовков программы от начала файла в байтах
	unsigned long int e_shoff = 0;				///< Смещение таблицы заголовков секций от начала файла в байтах

	unsigned int e_flags = 0;					///< Флаги, зависящие от процессора
	unsigned short int e_ehsize = 64;			///< Размер заголовка
	unsigned short int e_phentsize = 56;		///< Размер одного заголовка программы
	unsigned short int e_phnum = 0;				///< Количество заголовков программы (Всегда 2 вроде ???)
	unsigned short int e_shentsize = 64;		///< Размер одного заголовка секции
	unsigned short int e_shnum = 0;				///< Число заголовков секций (Пустая, shstrtab, .text, .bss? .data?)
	unsigned short int e_shstrndx = 0;			///< Индекс записи .shstrtab в таблице заголовков секций (Последняя)

	FileHeader();
	~FileHeader();
};


class ELF::ProgramHeader
{
	friend ELF;

  public:

	//! Константы заголовка программы
	enum Constants
	{
		PT_NULL = 0,				///< Заголовок не используется
		PT_LOAD = 1,				///< Загружаемый сегмент
		PT_DYNAMIC = 2,				///< Заголовок с информацией о динамической компоновке
		PT_INTERP = 3,				///< Размер и местоположение пути для запуска в качестве интерпретатора
		PT_NOTE = 4,				///< Размер и местоположение вспомогательной информации
		PT_SHLIB = 5,				///< Тип зарезервирован, но его смысл не определен
		PT_PHDR = 6,				///< Местоположение и размер таблицы заголовков программы
		PT_TLS = 7,					///< Шаблон Thread-Local Storage

		PF_X = 0x1,					///< Разрешение на исполнение
		PF_W = 0x2,					///< Разрешение на запись
		PF_R = 0x4,					///< Разрешение на чтение
		PF_MASKOS = 0x0ff00000,		///< Маска значений, зависящих от операционной системы
		PF_MASKPROC = 0xf0000000	///< Маска значение, зависящих от процессора
	};

	unsigned int p_type = PT_LOAD;		///< Тип сегмента, который описывает данный заголовок
	unsigned int p_flags = PF_R;		///< Флаги сегмента (для ELF64)
	unsigned long int p_offset = 0;		///< Смещение сегмента от начала файла
	unsigned long int p_vaddr = 0;		///< Виртуальный адрес сегмента в памяти
	unsigned long int p_paddr = 0;		///< Физический адрес сегмента
	unsigned long int p_filesz = 0;		///< Размер сегмента в файле
	unsigned long int p_memsz = 0;		///< Размер сегмента в памяти
	unsigned long int p_align = 0x1000;	///< Выравнивание сегмента

	ProgramHeader();
	~ProgramHeader();
};


class ELF::SectionHeader
{
	friend ELF;

  public:

	//! Константы заголовка секции
	enum Constants
	{
		SHT_NULL = 0,					///< Заголовок не используется
		SHT_PROGBITS = 1,				///< Секция содержит информацию, определенную программой
		SHT_SYMTAB = 2,					///< Секция содержит таблицу символов
		SHT_STRTAB = 3,					///< Секция содержит таблицу строк
		SHT_RELA = 4,					///< Секция содержит расширенную информацию о перемещениях
		SHT_HASH = 5,					///< Секция содержит таблицу хэшей символов
		SHT_DYNAMIC = 6,				///< Секция содержит информацию о динамической компоновке
		SHT_NOTE = 7,					///< Секция содержит информацию, которая каким-то образом отмечает файл
		SHT_NOBITS = 8,					///< Секция не занимает места в файле
		SHT_REL = 9,					///< Секция содержит информацию о перемещениях
		SHT_SHLIB = 10,					///< Тип секции определен, но не имеет значения
		SHT_DYNSYM = 11,				///< Содержит таблицу символов
		SHT_INIT_ARRAY = 14,			///< Содержит массив указателей на функции инициализации программы
		SHT_FINI_ARRAY = 15,			///< Содержит массив указателей на функции финализации программы
		SHT_PREINIT_ARRAY = 16,			///< Содержит массив указателей на функции, вызываемые до инициализации программы
		SHT_GROUP = 17,					///< Секция с определением группы секций
		SHT_SYMTAB_SHNDX = 18,			///< Секция связана с таблицей символов

		SHF_WRITE = 0x1,				///< Разрешение на запись
		SHF_ALLOC = 0x2,				///< Занимает память во время выполнения
		SHF_EXECINSTR = 0x4,			///< Содержит исполняемые машинные инструкции
		SHF_MERGE = 0x10,				///< Данные могут быть объединены для устранения дублирования
		SHF_STRINGS = 0x20,				///< Состоит из массивов символов с завершающим нулем
		SHF_INFO_LINK = 0x40,			///< Поле sh_info содержит индекс элемента таблицы заголовков секций
		SHF_LINK_ORDER = 0x80,			///< Особые требования по расположению
		SHF_OS_NONCONFORMING = 0x100,	///< Секция требует особой обработки
		SHF_GROUP = 0x200,				///< Секция - элемент группы секций
		SHF_TLS = 0x400,				///< Содержит Thread-Local Storage
		SHF_COMPRESSED = 0x800,			///< Секция содержит сжатые данные
		SHF_MASKOS = 0x0ff00000,		///< Маска ОС
		SHF_MASKPROC = 0xf0000000		///< Маска процессора
	};

	unsigned int sh_name = 0;			///< Смещение названия секции в .shstrtab
	unsigned int sh_type = SHT_NULL;	///< Тип заголовка
	unsigned long int sh_flags = 0;		///< Атрибуты секции
	unsigned long int sh_addr = 0;		///< Адрес, в который необходимо загружать секцию
	unsigned long int sh_offset = 0;	///< Смещение секции от начала файла в байтах
	unsigned long int sh_size = 0;		///< Размер секции в файле
	unsigned int sh_link = 0;			///< Индекс ассоциированной секции (Всегда 0 в рамках компилятора)
	unsigned int sh_info = 0;			///< Дополнительная информация о секции (Всегда 0 в рамках компилятора)
	unsigned long int sh_addralign = 0;	///< Необходимое выравнивание секции (0x10 для .text, 0x1 для shstrtab)
	unsigned long int sh_entsize = 0;	///< Размер в байтах каждой записи (Если они одинакого размера) (Всегда 0 в рамках компилятора)

	SectionHeader();
	~SectionHeader();
};









