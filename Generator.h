#pragma once

#include <filesystem>
#include <map>

#include "Structure.h"
#include "Protocol.h"

class Generator
{
public:
	virtual void generate(const std::filesystem::path& folder, const std::map<std::string, Structure>& types, const Protocol& protocol) const = 0;
};

