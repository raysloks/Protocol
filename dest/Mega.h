#pragma once

// WARNING : Auto-generated file, changes made will disappear when re-generated.

#include <memory>
#include <vector>

class Test;
class Upper;

class Mega
{
public:
	std::vector<float> health_values;
	std::unique_ptr<Upper> target;
	std::vector<Test> peeps;
	int32_t hoho;
	int64_t muh_dick;
	std::vector<Upper> testero;

	void serialize(ostream& os) const;
	void deserialize(istream& is);
};
