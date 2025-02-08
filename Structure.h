#pragma once

#include <vector>
#include <set>

#include "Field.h"

class Structure
{
public:
	std::vector<Field> fields, parent_fields;

	std::set<std::string> dependencies, delayed_dependencies, system_dependencies, application_dependencies, hidden_dependencies;

	bool up, down;
	bool used_up, used_down;

	std::string parent_name;
	bool parent_initialized;

	std::set<std::string> child_type_names;
	int child_type_index;

	bool operator==(const Structure& rhs) const;

	bool flat() const;
	bool superFlat() const;

	int maxSize() const;

	void addParentFields(const Structure& parent);

	void cascadeUsed();
	void cascadeUsed(bool use_up, bool use_down);
};

