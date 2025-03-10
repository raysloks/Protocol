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
#include "CsGenerator.h"

#include "Config.h"

const std::vector<std::string> basic_types = {
	"bool",
	"float", "double",
	"int8", "int16", "int32", "int64",
	"uint8", "uint16", "uint32", "uint64",
	"string",
	"vec2", "vec3", "vec4",
	"uuid"
};

const std::vector<std::string> qualifiers = { "up", "down", "is" };

const bool allow_external_types = true;

int main()
{
	Config config;
	config.load("config.txt");

	for (auto& protocol : config.protocols)
	{
		std::cout << protocol->name << std::endl;

		std::map<std::string, Structure> types;

		bool error = false;

		for (const auto & entry : std::filesystem::directory_iterator(protocol->source))
		{
			std::ifstream f(entry.path());

			Structure type;

			std::string line;
			while (std::getline(f, line))
			{
				std::istringstream iss(line);

				auto tokens = extract_tokens(iss);

				if (std::find(qualifiers.begin(), qualifiers.end(), tokens[0]) != qualifiers.end())
				{
					for (auto& token : tokens)
					{
						if (token == "up")
							type.up = true;
						if (token == "down")
							type.down = true;
					}
					if (tokens[0] == "is")
					{
						if (tokens.size() < 2)
						{
							std::cout << "Error: 'is' needs to be followed by a type name." << std::endl;
							error = true;
						}
						else
						{
							type.parent_name = tokens[1];
							type.dependencies.insert(type.parent_name);
						}
					}
					continue;
				}

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

					std::string original_type_name = type.first;
					std::function<void(const std::pair<std::string, Structure>&)> func = [original_type_name, &types, &func, &error](const std::pair<std::string, Structure>& type)
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

					if (field.special == FS_NONE || field.special == FS_VECTOR)
					{
						type.second.dependencies.insert(field.type_name);
						func(*type_found);
					}
					else
					{
						//type.second.delayed_dependencies.insert(field.type_name);
						type.second.dependencies.insert(field.type_name);
					}

					continue;
				}

				auto basic_type_found = std::find(basic_types.begin(), basic_types.end(), field.type_name);
				if (basic_type_found != basic_types.end())
				{
					if (field.type_name == "string")
						type.second.system_dependencies.insert("string");
					if (field.type_name == "vec2")
						type.second.application_dependencies.insert("Vec2");
					if (field.type_name == "vec3")
						type.second.application_dependencies.insert("Vec3");
					if (field.type_name == "vec4")
						type.second.application_dependencies.insert("Vec4");
					if (field.type_name == "uuid")
						type.second.system_dependencies.insert("boost/uuid/uuid.hpp");
					continue;
				}

				if (allow_external_types)
				{
					std::cout << "Warning: External type '" << field.type_name << "' must be trivially copyable." << std::endl;
					type.second.application_dependencies.insert(field.type_name);
					continue;
				}

				std::cout << "Error: Unknown type '" << field.type_name << "'" << std::endl;
				error = true;
			}
		}

		// TODO add error report for field inheriting from containing type
		for (auto& type : types)
		{
			std::string original_type_name = type.first;
			std::function<void(Structure&)> func = [original_type_name, &types, &func, &error](Structure& type)
			{
				type.child_type_names.insert(original_type_name);
				if (type.parent_name == original_type_name)
				{
					std::cout << "Error: '" << original_type_name << "' inherits from itself." << std::endl;
					error = true;
					return;
				}
				/*if (type.parent_initialized)
					return;*/
				if (type.parent_name.size() > 0)
				{
					auto type_found = types.find(type.parent_name);
					if (type_found != types.end())
					{
						func(type_found->second);
						type.addParentFields(type_found->second);
					}
					else
					{
						std::cout << "Error: Type '" << type.parent_name << "' not found. Inheritance from unknown types is not supported." << std::endl;
						error = true;
						return;
					}
				}
				//type.parent_initialized = true;
			};

			func(type.second);
		}

		for (auto& type : types)
		{
			type.second.cascadeUsed();
		}

		for (auto& type : types)
		{
			std::string original_type_name = type.first;
			std::function<int(Structure&)> getChildTypeIndex = [original_type_name, &types, &getChildTypeIndex, &error](Structure& type)
			{
				if (type.parent_name.size() > 0)
				{
					auto type_found = types.find(type.parent_name);
					if (type_found != types.end())
						return getChildTypeIndex(type_found->second);
				}
				if (type.child_type_names.size() == 1)
					return 0xff;
				return (int)std::distance(type.child_type_names.begin(), type.child_type_names.find(original_type_name));
			};

			type.second.child_type_index = getChildTypeIndex(type.second);
		}

		for (auto& type : types)
		{
			for (auto& field : type.second.fields)
			{
				if (field.type)
				{
					for (auto& child_type_name : field.type->child_type_names)
					{
						if (child_type_name != field.type_name)
							type.second.hidden_dependencies.insert(child_type_name);
					}
				}
			}
		}

		if (error)
		{
			std::cout << "Protocol code generation failed, there were error(s) in the protocol." << std::endl;
			return 1;
		}

		std::ostringstream oss;
		for (auto& type : types)
		{
			oss << type.first << ":" << type.second.parent_name << std::endl;
			for (auto& field : type.second.fields)
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

		protocol->crc = crc32(build_data);

		for (auto& generator : protocol->generators)
			generator->generate_if_new(types, *protocol, build_data);
	}
}