/*!
 *	@file binary.hpp
 *	@brief Описание класса бинарного кода
 *	@todo Сначала импорт в токены, далее линковка (все объекты в первый), далее компиляция и простроение позиций, 
 *	оптимизации и простановка адресов вызовов
 */


#pragma once

#include <cstring>
#include <cassert>
#include <cstring>
#include "ast.hpp"
#include "elf.hpp"

class Binary
{
  public:
	
	enum Errors
	{
		OK,
		NO_ENTRY_POINT,
		FUNCTION_NOT_EXIST,
		FUNCTION_REDEFINED
	};
	
  	class Token;
	
	Token* start = nullptr;
	Token* end = nullptr;
	long int size = 0;
	
	Token** labels = nullptr;
	unsigned int labels_count = 0;
	
	int status = OK;
	
	Binary();
	~Binary();
	
	//------ Abstract syntax tree compilation ------//
	void importAST(AST* ast);						///< Добавление информации в объект Binary на основе AbstractSyntaxTree
	void importAST(ASN* node);						///< Рекурсивная компиляция
	void importDef(ASN* node);						///< Вспомогательный метод для importAST (импорт функции)
	void importBody(ASN* node);						///< Вспомогательный метод для importAST (импорт основных структур кода)
	void importParameters(ASN* node);				///< Вспомогательный метод для importAST (импорт загрузки параметров функции)
	void importNode(ASN* node);						///< Вспомогательный метод для importAST (импорт основных типов узлов)
	//----------------------------------------------//
	
	void importNasm(const char* nasm_code);	///< TODO Импорт ассемблерного кода
	void importObj(const char* object_code, long int object_code_length);	///< TODO Импорт объектного файла
	
	void pushBack(int type, const char* text, int ivalue, float fvalue, char cvalue, const char* svalue);
	void pushBytes(const char* function, long int function_length);
	
	//----------- Just a covers for default pushBack -----------//
	void pushBack(const char* nasm_code);						//
	void pushBack(const char* nasm_code, int ivalue);			//
	void pushBack(const char* nasm_code, float fvalue);			//
	void pushBack(const char* nasm_code, char cvalue);			//
	void pushBack(const char* nasm_code, const char* svalue);	//
																//
	void pushBack(int type, const char* text);					//
	void pushBack(int type, const char* text, int ivalue);		//
	//----------------------------------------------------------//
	
	void compile();			///< Подготовка токенов и их компиляция в байт-код
	void storeLabels();		///< Сохранение всех меток в массив в объекте Binary
	
	void optimize();		///< TODO Оптимизация байт-кода
	int deleteUnusedFunctions();
	int foldPushPop();
	int foldMovMov();
	int deleteUselessOperations();
	
	void setLabels();		///< Высчет позиций меток и их занесение в необходимые места
	long int getLabelPosition(const char* label_name);	///< Поиск адреса метки
	
	int exportString(char* destination);
	int exportNasm(const char* filename);
	int exportObj(const char* filename);
	int exportExecutable(const char* filename);
	int exportVirtualExecutable(const char* filename);
	int exportHex(const char* filename);
};


class Binary::Token
{
	friend Binary;
	
  protected:
	int type = EMPTY;

	char* text = nullptr;
	int text_length = 0;
	
	char* bytes = nullptr;
	int bytes_count = 0;
	
	int ivalue = 0;
	float fvalue = 0;
	char cvalue = 0;
	char* svalue = nullptr;
	
	int parameter_length = 0;

	unsigned long long int first_byte_position = 0;
	
	Token* prev = nullptr;
	Token* next = nullptr;
	
	Binary* container = nullptr;
	
	//! Типы данных в токене
	enum TokenTypes
	{
		EMPTY, 			///< Пустой токен
		LOCAL_LABEL, 	///< Токен содержит локальную метку
		GLOBAL_LABEL, 	///< Токен содержит глобальную метку
		FUNCTION_LABEL,	///< Токен содержит метку функции
		NASM_CODE, 		///< Токен содержит ассемблерный код
		BYTE_CODE, 		///< Токен содержит скомпилированный код
		BOTH,			///< Токен содержит ассемблерный и скомпилированный код
		TYPES_COUNT		///< Количество типов токенов
	};
	
  public:
  	Token();
	void setText(const char* text);
	void setBytes(const char* bytes, long int bytes_count);
	void setSValue(const char* svalue);
	~Token();
	
	int decompile();	///< TODO Декомпиляция из байт-кода в ассемблерный, если возможно
	int compile();		///< Компиляция из ассемблерного кода в байт-код
	int prepare();		///< Подготовка ассемблерной команды, приведение ее к обобщенному виду
};

//typedef Binary::Token Token;



