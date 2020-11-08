#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "Message.h"

class Generator;

class Protocol
{
public:
	std::vector<Message> messages;

	std::string name, prefix;

	std::string handler;

	std::filesystem::path source;

	std::vector<std::unique_ptr<Generator>> generators;

	uint32_t crc;
};

