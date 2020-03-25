#include <map>

#include <filesystem>
#include <fstream>
#include <sstream>

#include <iostream>
#include <functional>

#include "Structure.h"
#include "Protocol.h"

#include "Token.h"

#include "Crc32.h"

#include "CppGenerator.h"

int main()
{
	std::map<std::string, std::string> basic_types = { {"float", "float"},
		{"int8", "int8_t"}, {"int16", "int16_t"}, {"int32", "int32_t"}, {"int64", "int64_t"},
		{"uint8", "uint8_t"}, {"uint16", "uint16_t"}, {"uint32", "uint32_t"}, {"uint64", "uint64_t"} };

	std::map<std::string, Structure> types;

	Protocol protocol;

	std::string last_build_data_filename = "last_build.txt";

	auto path = std::filesystem::path("test/");
	for (const auto & entry : std::filesystem::directory_iterator(path))
	{
		if (entry.path().filename() == last_build_data_filename)
			continue;

		std::ifstream f(entry.path());

		Structure type;

		std::string line;
		while (std::getline(f, line))
		{
			std::istringstream iss(line);

			auto tokens = extract_tokens(iss);

			Field field;

			field.type_name = tokens[0];

			size_t i = 1;
			for (; tokens[i] == "*"; ++i) { field.special = FS_POINTER; }

			field.name = tokens[i];

			if (tokens.size() > i + 1)
			{
				if (tokens[i + 1] == "[" && tokens[i + 2] == "]")
				{
					field.special = FS_VECTOR;
				}
			}

			type.fields.push_back(field);
		}

		types[entry.path().stem().string()] = type;

		f.close();
	}

	bool error = false;

	for (auto& type : types)
	{
		for (auto& field : type.second.fields)
		{
			switch (field.special)
			{
			case FS_NONE:
				break;
			case FS_POINTER:
				type.second.system_dependencies.insert("memory");
				break;
			case FS_VECTOR:
				type.second.system_dependencies.insert("vector");
				break;
			}

			auto type_found = types.find(field.type_name);
			if (type_found != types.end())
			{
				field.type = &type_found->second;

				auto original_type_name = type.first;
				std::function<void(const std::pair<std::string, Structure>&)> func = [original_type_name, types, &func, &error](const std::pair<std::string, Structure>& type)
				{
					if (type.first == original_type_name)
					{
						std::cout << "Error: '" << original_type_name << "' contains itself." << std::endl;
						error = true;
						return;
					}
					for (auto& field : type.second.fields)
					{
						auto type_found = types.find(field.type_name);
						if (type_found != types.end())
						{
							func(*type_found);
						}
					}
				};

				if (field.special == FS_NONE)
				{
					type.second.dependencies.insert(field.type_name);
					func(*type_found);
				}
				else
				{
					type.second.delayed_dependencies.insert(field.type_name);
				}

				continue;
			}

			auto basic_type_found = basic_types.find(field.type_name);
			if (basic_type_found != basic_types.end())
			{
				field.type_name = basic_type_found->second;
				continue;
			}

			std::cout << "Error: Unknown type '" << field.type_name << "'" << std::endl;
			error = true;
		}
	}

	if (error)
	{
		std::cout << "Protocol code generation failed, there were error(s) in the protocol." << std::endl;
		return 1;
	}

	std::ostringstream oss;
	for (auto type : types)
	{
		oss << type.first << std::endl;
		for (auto field : type.second.fields)
		{
			switch (field.special)
			{
			case FS_NONE:
				oss << field.type_name << " " << field.name << std::endl;
				break;
			case FS_POINTER:
				oss << field.type_name << " * " << field.name << std::endl;
				break;
			case FS_VECTOR:
				oss << field.type_name << " " << field.name << "[]" << std::endl;
				break;
			}
		}
	}

	std::string build_data = oss.str();

	{
		std::ifstream f(path / last_build_data_filename, std::ios::in | std::ios::binary);
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
				std::cout << "No changes detected, skipping protocol code generation." << std::endl;
				return 0;
			}
		}
	}

	{
		std::ofstream f(path / last_build_data_filename, std::ios::out | std::ios::binary);
		if (f)
		{
			f.write(build_data.data(), build_data.size());
			f.close();
		}
	}

	protocol.crc = crc32(build_data);

	std::unique_ptr<Generator> generator = std::make_unique<CppGenerator>();

	generator->generate("dest/", types, protocol);
}