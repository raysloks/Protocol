#include "Config.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "Protocol.h"

#include "CppGenerator.h"
#include "CsGenerator.h"

void Config::load(const std::filesystem::path& file)
{
	std::ifstream f(file);

	while (f.good())
	{
		auto protocol = std::make_unique<Protocol>();

		std::getline(f, protocol->name);
		std::getline(f, protocol->prefix);
		std::getline(f, protocol->handler);

		std::string source;
		std::getline(f, source);
		protocol->source = source;

		std::string type;
		while (std::getline(f, type))
		{
			if (type.empty())
				break;

			std::unique_ptr<Generator> generator;
			if (type == "cpp")
				generator = std::make_unique<CppGenerator>();
			if (type == "cs")
				generator = std::make_unique<CsGenerator>();

			std::string options;
			std::getline(f, options);
			if (options.find("super") != std::string::npos)
				generator->builtins_in_superdirectory = true;
			if (options.find("up") != std::string::npos)
				generator->up = true;
			if (options.find("down") != std::string::npos)
				generator->down = true;

			std::string folder;
			std::getline(f, folder);
			generator->folder = folder;
			protocol->generators.emplace_back(std::move(generator));
		}

		protocols.emplace_back(std::move(protocol));
	}
}
