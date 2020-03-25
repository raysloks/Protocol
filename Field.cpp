#include "Field.h"

bool Field::operator==(const Field& rhs) const
{
	return type_name == rhs.type_name && name == rhs.name && special == rhs.special;
}
