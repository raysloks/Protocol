#include "Link.h"

// WARNING : Auto-generated file, changes made will disappear when re-generated.

void Link::Open(const asio::ip::udp::endpoint& endpoint)
{
	socket.bind(endpoint);
}

void Link::Dispatch(const asio::ip::udp::endpoint& endpoint) const
{
	std::istream is(&buffer);
	switch (is.get())
	{
	case  :
	{
		Mega message;
		message.deserialize(is);
		break;
	}
	case :
	{
		Test message;
		message.deserialize(is);
		break;
	}
	case :
	{
		Upper message;
		message.deserialize(is);
		break;
	}
	}
}

void Link::Send(const asio::ip::udp::endpoint& endpoint, const Mega& message) const
{
	asio::streambuf buffer;
	std::ostream os(&buffer);
	os.put( );
	message.serialize(os);
	socket.async_send_to(buffer.data(), endpoint, [](const asio::error_code&, size_t) {});
}

void Link::Send(const asio::ip::udp::endpoint& endpoint, const Test& message) const
{
	asio::streambuf buffer;
	std::ostream os(&buffer);
	os.put();
	message.serialize(os);
	socket.async_send_to(buffer.data(), endpoint, [](const asio::error_code&, size_t) {});
}

void Link::Send(const asio::ip::udp::endpoint& endpoint, const Upper& message) const
{
	asio::streambuf buffer;
	std::ostream os(&buffer);
	os.put();
	message.serialize(os);
	socket.async_send_to(buffer.data(), endpoint, [](const asio::error_code&, size_t) {});
}

