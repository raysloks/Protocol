#include "Structure.h"

bool Structure::operator==(const Structure& rhs) const
{
	return fields == rhs.fields && dependencies == rhs.dependencies && delayed_dependencies == rhs.delayed_dependencies && system_dependencies == rhs.system_dependencies && parent_name == rhs.parent_name;
}

bool Structure::flat() const
{
	for (auto& field : parent_fields)
		if (!field.flat())
			return false;
	for (auto& field : fields)
		if (!field.flat())
			return false;
	return true;
}

void Structure::addParentFields(const Structure & parent)
{
	if (!parent_initialized)
	{
		parent_fields = parent.parent_fields;
		parent_fields.insert(parent_fields.end(), parent.fields.begin(), parent.fields.end());

		parent_initialized = true;
	}
}
