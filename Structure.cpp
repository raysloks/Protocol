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

bool Structure::superFlat() const
{
	for (auto& field : parent_fields)
		if (!field.superFlat())
			return false;
	for (auto& field : fields)
		if (!field.superFlat())
			return false;
	return true;
}

int Structure::maxSize() const
{
	int max_size = 0;
	for (auto& field : parent_fields)
		max_size += field.maxSize();
	for (auto& field : fields)
		max_size += field.maxSize();
	return max_size;
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

void Structure::cascadeUsed()
{
	used_up |= up;
	used_down |= down;

	for (auto& field : parent_fields)
		if (field.type)
			field.type->cascadeUsed(used_up, used_down);
	for (auto& field : fields)
		if (field.type)
			field.type->cascadeUsed(used_up, used_down);
}

void Structure::cascadeUsed(bool use_up, bool use_down)
{
	if (use_up && !used_up || use_down && !used_up)
	{
		used_up |= up;
		used_down |= down;

		used_up |= use_up;
		used_down |= use_down;

		for (auto& field : parent_fields)
			if (field.type)
				field.type->cascadeUsed(used_up, used_down);
		for (auto& field : fields)
			if (field.type)
				field.type->cascadeUsed(used_up, used_down);
	}
}
