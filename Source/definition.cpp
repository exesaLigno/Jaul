#include "../Headers/definition.hpp"


Definition::Definition()
{
	defname = 0;
	defstatement = 0;
	defname_length = 0;
	defstatement_length = 0;
}

Definition::~Definition()
{
	if (defname)
		delete[] defname;
	if (defstatement)
		delete[] defstatement;
}

