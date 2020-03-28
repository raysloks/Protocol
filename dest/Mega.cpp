#include "Mega.h"

// WARNING : Auto-generated file, changes made will disappear when re-generated.

#include <iostream>

#include "Test.h"
#include "Upper.h"

void Mega::serialize(ostream& os) const
{
	{
		uint16_t size = this->health_values.size();
		os.write((char*)&size, sizeof(size));
		os.write((char*)&this->health_values.data(), sizeof(float) * size);
	}
	if (target)
	{
		os.put(true);
		target->serialize(os);
	}
	else
	{
		os.put(false);
	}
	{
		uint16_t size = this->peeps.size();
		os.write((char*)&size, sizeof(size));
		os.write((char*)&this->peeps.data(), sizeof(Test) * size);
	}
	os.write((char*)&hoho, sizeof(hoho));
	os.write((char*)&muh_dick, sizeof(muh_dick));
	{
		uint16_t size = this->testero.size();
		os.write((char*)&size, sizeof(size));
		os.write((char*)&this->testero.data(), sizeof(Upper) * size);
	}
}

void Mega::deserialize(istream& is)
{
	{
		uint16_t size;
		is.read((char*)&size, sizeof(size));
		this->health_values.resize(size);
		is.read((char*)&this->health_values.data(), sizeof(float) * size);
	}
	if (is.get())
	{
		target = std::make_unique<Upper>();
		target->deserialize(is);
	}
	{
		uint16_t size;
		is.read((char*)&size, sizeof(size));
		this->peeps.resize(size);
		is.read((char*)&this->peeps.data(), sizeof(Test) * size);
	}
	is.read((char*)&hoho, sizeof(hoho));
	is.read((char*)&muh_dick, sizeof(muh_dick));
	{
		uint16_t size;
		is.read((char*)&size, sizeof(size));
		this->testero.resize(size);
		is.read((char*)&this->testero.data(), sizeof(Upper) * size);
	}
}

