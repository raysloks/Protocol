// WARNING : Auto-generated file, changes made will disappear when re-generated.

public class Link
{
	public void Open(IPEndPoint endpoint)
	{
		
	}

	public void Send(IPEndPoint endpoint, in Mega message)
	{
		//asio::streambuf buffer;
		//std::ostream os(&buffer);
		//os.put( );
		//message.serialize(os);
		//socket.async_send_to(buffer.data(), endpoint, [](const asio::error_code&, size_t) {});
	}

	public void Send(IPEndPoint endpoint, in Test message)
	{
		//asio::streambuf buffer;
		//std::ostream os(&buffer);
		//os.put();
		//message.serialize(os);
		//socket.async_send_to(buffer.data(), endpoint, [](const asio::error_code&, size_t) {});
	}

	public void Send(IPEndPoint endpoint, in Upper message)
	{
		//asio::streambuf buffer;
		//std::ostream os(&buffer);
		//os.put();
		//message.serialize(os);
		//socket.async_send_to(buffer.data(), endpoint, [](const asio::error_code&, size_t) {});
	}

	void Link::Dispatch(const asio::ip::udp::endpoint& endpoint) const
	{
		std::istream is(&buffer);
		switch (is.get())
		{
		case  :
		{
			Mega message;
			//message.deserialize(is);
			break;
		}
		case :
		{
			Test message;
			//message.deserialize(is);
			break;
		}
		case :
		{
			Upper message;
			//message.deserialize(is);
			break;
		}
		}
	}

	public const uint crc = 0xa610a95;
	private UdpClient client;
}
