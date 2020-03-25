#pragma once

#include <vector>

#include "Message.h"

class Protocol
{
public:
	std::vector<Message> messages;

	uint32_t crc;
};

