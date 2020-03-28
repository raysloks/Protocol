#pragma once

// WARNING : Auto-generated file, changes made will disappear when re-generated.

#include <asio.hpp>

#include "Mega.h"
#include "Test.h"
#include "Upper.h"

class Link
{
public:
	void Open(const asio::ip::udp::endpoint& endpoint);
	void Send(const asio::ip::udp::endpoint& endpoint, const Mega& message) const;
	void Send(const asio::ip::udp::endpoint& endpoint, const Test& message) const;
	void Send(const asio::ip::udp::endpoint& endpoint, const Upper& message) const;
	static const uint32_t crc = 0xa610a95;
private:
	asio::ip::udp::socket socket;
}
