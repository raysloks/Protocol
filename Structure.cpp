#include "Structure.h"

bool Structure::operator==(const Structure& rhs) const
{
	return fields == rhs.fields && dependencies == rhs.dependencies && delayed_dependencies == rhs.delayed_dependencies && system_dependencies == rhs.system_dependencies;
}

bool Structure::flat() const
{
	for (auto field : fields)
		if (!field.flat())
			return false;
	return true;
}
