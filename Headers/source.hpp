/*!
 *	@file source.hpp
 *	@brief Описание класса исходного кода
 */


#pragma once

#include <cstring>
#include "ast.hpp"
#include "debug.hpp"
#include <cmath>
#include <cstdio>

const float APPROXIMATION = 0.0001;

class Source
{
  public:
	char* name = nullptr;
	char* text = nullptr;
	long long int text_length = 0;
	int current_line = 1;
	int status = OK;
	
  private:
	char* text_pointer = nullptr;
	
  public:
	short int source_type = 0;
	AST* ast = nullptr;
	
	Source();
	Source(const char* name);
	~Source();
	
	bool open();
	short int getType();
	void print();
	
	void makeAST();
	void optimizeAST();
	void setTypes();		///< Оптимизация типизации
	void setFloats();		///< Простановка типов в старом стиле - только float, только хардкор
	void prepareAST();
	
	void dumpAST(const char* title);
	
  private:
	int calculateIndent(char* text);
	bool nextLine(char** _text);
	
	ASN* parseBlock(int indent, char** _text);		///< Part of recursive descent: parsing block of code
	ASN* parseLine(int indent, char** _text);		///< Part of recursive descent: parsing line
	ASN* getInclude(int indent, char** _line);		///< Part of recursive descent: getting including library
	ASN* getDef(int indent, char** _line);			///< Part of recursive descent: parsing function declaration
	ASN* getAssignment(int indent, char** _line);	///< Part of recursive descent: parsing assignment
	ASN* getOperators(int indent, char** _line);	///< Part of recursive descent: parsing branching control operators
	ASN* getLogic(int indent, char** _line);		///< Part of recursive descent: parsing logic operators
	ASN* getCmp(int indent, char** _line);			///< Part of recursive descent: parsing comparison operators
	ASN* getAddSub(int indent, char** _line);		///< Part of recursive descent: parsing add and sub operators
	ASN* getMulDiv(int indent, char** _line);		///< Part of recursive descent: parsing multiply and divide operators
	ASN* getPow(int indent, char** _line);			///< Part of recursive descent: parsing power operator
	ASN* getNumVarFunc(int indent, char** _line);	///< Part of recursive descent: parsing operand
	ASN* getItemize(int indent, char** _line);		///< Part of recursive descent: parsing itemize
	
	void splitFunctions();		///< Выделение функций из дерева программы и их расположение друг за другом
	void enumerateMembers();	///< Нумерация переменных и операторов ветвления
	
	void enumerateBranching(ASN* node, int* number);						///< Нумерация операторов ветвления
	void setSide(ASN* node, const char* varname, int vartype);				///< Установка информации о том, является ли переменная LValue или RValue
	char* getUnnumeratedVariable(ASN* node, int vartype);					///< Получение любой непронумерованной переменной в переданной ветви
	void enumerateVariables(ASN* node, const char* varname, int varnumber);	///< Установка номера определенной переменной в указаной ветви
	
	void setFloats(ASN* node);	///< Рекурсивная простановка типов в старом стиле
	int setTypes(ASN* node);	///< Рекурсивная установка типов данных для дерева
	int setVariablesTypes(ASN* node, ASN* variable, int mode);
	void initParameters(ASN* node);
	int verifyDeclarationCoincidence();
	int verifyDeclarationCoincidence(ASN* node, ASN* declaration);
	void flushTypes(ASN* node);
	int getFunctionType(ASN* node);
	
	int substituteStatic(ASN* node);
	int setStatic(ASN* node, ASN* variable, ASN* value, bool* declaration_needed, bool* substituted);
	int processBranching(ASN* node, ASN* variable);
	
	int foldConstants(ASN* node);
	int foldArithmeticConstants(ASN* node);
	int foldCmpConstants(ASN* node);
	int foldCtrlConstants(ASN* node);
	
	int inlineFunctions(ASN* node);
	
	int deleteUnused(ASN* node);	///< @todo TODO make deleting of unused variables
	int checkUsage(ASN* node);
	
	int optimizeBuiltInFunctions(ASN* node);
	
	
	ASN* getLine(ASN* node);		///< Возвращает объект строки, в которой находится переданный в параметры узел
	ASN* getFunction(ASN* node);		///< Возвращает объект функции, в которой находится переданный в параметры узел
	
  public:
	
	enum SourceTypes
	{
		JAUL_SOURCE = 1,		// .j		| Simple hi-level source
		JASM_SOURCE,			// .jasm	| Special assembler (like nasm, but without macroses etc)
		JAUL_OBJ,				// .jo		| Object format
		VIRTUAL_EXECUTABLE,		// .jv		| Virtual executable format
		DEFINITION,				// fileless	| Not implemented fuck
		ERRTYPE
	};
	
	enum Errors
	{
		OK,
		OPENNING_PROHIBITED,
		FILE_NOT_EXIST,
		UNSUPPORTED_FILE_EXTENSION,
		NOT_SUBSTITUTABLE_TYPE,
		SYNTAX_ERROR
	};
	
	enum Constants
	{
		DEFAULT,
		FORCE
	};
};

