#pragma once

#include "Generator.h"

class CppGenerator :
	public Generator
{
public:
	void generate(std::map<std::string, Structure>& types, const Protocol& protocol) const;
};

