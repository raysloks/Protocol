#include "Upper.h"

// WARNING : Auto-generated file, changes made will disappear when re-generated.

#include <iostream>

void Upper::serialize(ostream& os) const
{
	os.write((char*)this, sizeof(Upper));
}

void Upper::deserialize(istream& is)
{
	is.read((char*)this, sizeof(Upper));
}

