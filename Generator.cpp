#include "Generator.h"

#include <fstream>
#include <iostream>

const std::string last_build_data_filename = "last_build.txt";

void Generator::generate_if_new(const std::map<std::string, Structure>& types, const Protocol& protocol, const std::string& build_data) const
{
	{
		std::ifstream f(folder / last_build_data_filename, std::ios::in | std::ios::binary);
		if (f)
		{
			std::string data;
			f.seekg(0, std::ios::end);
			data.resize(f.tellg());
			f.seekg(0, std::ios::beg);
			f.read(&data[0], data.size());
			f.close();

			if (data == build_data)
			{
				//std::cout << "No changes detected, skipping protocol code generation for " << folder << std::endl;
				//return;
				std::cout << folder << " appears up to date, protocol code will still be generated." << std::endl;
			}
		}
	}

	{
		std::ofstream f(folder / last_build_data_filename, std::ios::out | std::ios::binary);
		if (f)
		{
			f.write(build_data.data(), build_data.size());
			f.close();
		}
	}

	generate(types, protocol);
}
