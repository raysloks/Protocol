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

int Field::maxSize() const
{
	int base_size = 1;
	if (type_name == "int16")
		base_size = 2;
	if (type_name == "uint16")
		base_size = 2;
	if (type_name == "int32")
		base_size = 4;
	if (type_name == "uint32")
		base_size = 4;
	if (type_name == "int64")
		base_size = 8;
	if (type_name == "uint64")
		base_size = 8;
	if (type_name == "string")
		return 65535 + 2;
	if (type)
		base_size = type->maxSize();
	if (special == FS_VECTOR)
		return base_size * 65535 + 2;
	if (special == FS_POINTER)
		return base_size + 1;
	return base_size;
}
