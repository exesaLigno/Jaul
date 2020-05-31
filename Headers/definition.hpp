#pragma once

#include <iostream>


class Definition
{
  public:
	char* defname;
	char* defstatement;
	int defname_length;
	int defstatement_length;
	
	Definition();
	~Definition();
};
