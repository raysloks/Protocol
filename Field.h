#pragma once

#include <string>

enum FieldSpecial
{
	FS_NONE,
	FS_POINTER,
	FS_VECTOR
};

class Structure;

class Field
{
public:
	std::string type_name, name;
	FieldSpecial special;

	size_t nullable_index;

	Structure * type;

	bool operator==(const Field& rhs) const;

	bool flat() const;
	bool superFlat() const;
	bool shouldBeNullable() const;

	int maxSize() const;
};

