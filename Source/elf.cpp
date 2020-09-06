#include "../Headers/elf.hpp"


ELF::ELF(char* text, unsigned long int text_length)
{
	this -> text = text;
	this -> text_length = text_length;
}

ELF::~ELF()
{
	if (this -> text_section != nullptr)
		delete[] this -> text_section;
	
	if (this -> bss_section != nullptr)
		delete[] this -> bss_section;
	
	if (this -> data_section != nullptr)
		delete[] this -> data_section;
}

void ELF::addText(char* text_section, unsigned long int text_section_length)
{
	this -> text_section = text_section;
	this -> text_section_length = text_section_length;
}

void ELF::addBss(char* bss_section, unsigned long int bss_section_length)
{
	this -> bss_section = bss_section;
	this -> bss_section_length = bss_section_length;
}

void ELF::addData(char* data_section, unsigned long int data_section_length)
{
	this -> data_section = data_section;
	this -> data_section_length = data_section_length;
}


void ELF::assemble()
{
	
}


/*!
 * @brief Я уже конкретно заебался
 * 
 * Написано на отъебись в тупую блять
 */
void ELF::write(const char* filename)
{
	
}

void ELF::makeHeader()
{
	char* compiled_text = this -> text;
	long long int compiled_text_length = this -> text_length;
	
	this -> text = 0;
	this -> text_length = 0;
	
	this -> addBin("7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 02 00 3e 00 01 00 00 00 00 10 40 00 00 00 00 00 40 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 38 00 02 00 40 00 03 00 02 00 01 00 00 00 04 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 40 00 00 00 00 00 b0 00 00 00 00 00 00 00 b0 00 00 00 00 00 00 00 00 10 00 00 00 00 00 00 01 00 00 00 05 00 00 00 00 10 00 00 00 00 00 00 00 10 40 00 00 00 00 00 00 10 40 00 00 00 00 00"); 
	this -> addBin(8, compiled_text_length);
	this -> addBin(8, compiled_text_length);
	this -> addBin("00 10 00 00 00 00 00 00");
	
	for (int counter = 0; counter < 3920; counter++)
		this -> addBin("00");
	
	long long int new_text_length = compiled_text_length + (this -> text_length);
	char* new_text = new char[new_text_length];
	
	for (long long int i = 0; i < this -> text_length; i++)
		new_text[i] = (this -> text)[i];
	
	for (long long int i = 0; i < compiled_text_length; i++)
		new_text[(this -> text_length) + i] = compiled_text[i];
	
	delete[] this -> text;
	
	this -> text = new_text;
	this -> text_length = new_text_length;
	
	long long int shstrtab_indent = this -> text_length;
	
	this -> addBin("00 2e 73 68 73 74 72 74 61 62 00 2e 74 65 78 74");
	
	while (this -> text_length % 8 != 0)
		this -> addBin("00");
	
	long long int section_header_offset = this -> text_length;
	
	this -> addBin("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	this -> addBin("0b 00 00 00 01 00 00 00 06 00 00 00 00 00 00 00 00 10 40 00 00 00 00 00 00 10 00 00 00 00 00 00");
	this -> addBin(8, compiled_text_length);
	this -> addBin("00 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	this -> addBin("01 00 00 00 03 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	this -> addBin(8, shstrtab_indent);//52 10 00 00    00 00 00 00    
	this -> addBin("11 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	
	this -> substituteNumber(40, 8, section_header_offset);
	
	delete[] compiled_text;
}


void ELF::substituteNumber(long long int position, int bytes_count, int number)
{
	unsigned int quotient = number;
	char balance = 0;
	int counter = 0;
	while (counter < bytes_count)
	{
		balance = quotient % 256;
		quotient = quotient / 256;		
		(this -> text)[position + counter] = balance;
		
		counter++;
	}
	return;
}



void ELF::addBin(const char* string)	///< @bug Valgrind error: lose 4696 bytes
{
	int counter = 0;
	char symbol = 0;
	while (true)
	{
		int number = 0;
		if (string[counter] >= '0' and string[counter] <= '9')
			number = string[counter] - '0';
		else if (string[counter] >= 'a' and string[counter] <= 'f')
			number = string[counter] - 'a' + 10;
		
		if (counter % 3 == 0)
			symbol = 16 * number;
		
		else if (counter % 3 == 1)
			symbol += number;
		
		else
		{
			long long int new_length = this -> text_length + 1;
			char* old_text = this -> text;
			this -> text = new char[new_length];
			
			for (int i = 0; i < this -> text_length; i++)
				(this -> text)[i] = (old_text)[i];
			
			(this -> text)[this -> text_length] = symbol;
			
			if (old_text != nullptr)
				delete[] old_text;
			
			this -> text_length = new_length;
		}
				
		if (string[counter] == '\0')
			break;
		
		counter++;
	}
	return;
}


void ELF::addBin(const int bytes_count, const int number)
{
	unsigned int quotient = number;
	char balance = 0;
	int counter = 0;
	while (counter < bytes_count)
	{
		balance = quotient % 256;
		quotient = quotient / 256;
		counter++;
		
		long long int new_length = this -> text_length + 1;
		char* new_text = new char[new_length];
		
		for (int i = 0; i < this -> text_length; i++)
			new_text[i] = (this -> text)[i];
		new_text[this -> text_length] = balance;
		
		if (this -> text)
			delete[] this -> text;
		
		this -> text = new_text;
		this -> text_length = new_length;
		
	}
	return;
}
