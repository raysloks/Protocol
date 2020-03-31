#include "Field.h"

#include "Structure.h"

bool Field::operator==(const Field& rhs) const
{
	return type_name == rhs.type_name && name == rhs.name && special == rhs.special;
}

bool Field::flat() const
{
	if (special != FS_NONE)
		return false;
	if (type)
		if (!type->flat())
			return false;
	if (type_name == "string")
		return false;
	return true;
}
