#pragma once

#include <vector>
#include <set>

#include "Field.h"

class Structure
{
public:
	std::vector<Field> fields;

	std::set<std::string> dependencies, delayed_dependencies, system_dependencies, application_dependencies;

	bool up, down;

	bool operator==(const Structure& rhs) const;

	bool flat() const;
};

