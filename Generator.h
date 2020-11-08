#pragma once

#include <filesystem>
#include <map>

#include "Structure.h"
#include "Protocol.h"

class Generator
{
public:
	std::filesystem::path folder;

	void generate_if_new(const std::map<std::string, Structure>& types, const Protocol& protocol, const std::string& build_data) const;

	virtual void generate(const std::map<std::string, Structure>& types, const Protocol& protocol) const = 0;

	bool builtins_in_superdirectory = false;

	bool up = false;
	bool down = false;
};

