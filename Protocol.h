#pragma once

#include <vector>
#include <string>

#include "Message.h"

class Protocol
{
public:
	std::vector<Message> messages;

	std::string name, prefix;

	uint32_t crc;
};

