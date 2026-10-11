#pragma once

#include "Generator.h"

class CsGenerator :
	public Generator
{
public:
	void generate(std::map<std::string, Structure>& types, const Protocol& protocol) const;
};

