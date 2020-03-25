#pragma once

// WARNING : Auto-generated file, changes made will disappear when re-generated.

#include "Upper.h"

class Test
{
public:
	float time;
	uint8_t magic;
	Upper upper;

	void serialize(ostream& os) const;
	void deserialize(istream& is);
};
