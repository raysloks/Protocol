#pragma once

#include <filesystem>
#include <map>

#include "Structure.h"
#include "Protocol.h"

class Generator
{
public:
	void generate_if_new(const std::filesystem::path& folder, const std::map<std::string, Structure>& types, const Protocol& protocol, const std::string& build_data) const;

	virtual void generate(const std::filesystem::path& folder, const std::map<std::string, Structure>& types, const Protocol& protocol) const = 0;
};

