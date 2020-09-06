/*!
 *	@file debug.hpp
 *	@brief Библиотека для упрощения отладки программ и поиска SEGFAULT
 */


#pragma once

#include <fstream>


#ifndef release

	#define DEBUG printf("\x1b[35m>> debug from <%s::%d>\n\x1b[0m", __FILE__, __LINE__);
	
	#define PRINTD(format_string, ...)										\
			printf("\x1b[35m>> debug from <%s::%d>: ", __FILE__, __LINE__);	\
			printf(format_string, __VA_ARGS__);								\
			printf("\x1b[0m");												\
			
	#define SWITCH if(true)
			
#else

	#define DEBUG
	#define PRINTD(format_string, ...)
	#define SWITCH if(false)
	
#endif
