#pragma once

#include "Generator.h"

class CsGenerator :
	public Generator
{
public:
	void generate(const std::filesystem::path& folder, const std::map<std::string, Structure>& types, const Protocol& protocol) const;
};

