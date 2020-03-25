#include "Test.h"

// WARNING : Auto-generated file, changes made will disappear when re-generated.

#include <iostream>

void Test::serialize(ostream& os) const
{
	os.write((char*)this, sizeof(Test));
}

void Test::deserialize(istream& is)
{
	is.read((char*)this, sizeof(Test));
}

