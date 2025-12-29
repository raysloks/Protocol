#include "CppGenerator.h"

#include <fstream>

std::map<std::string, std::string> basic_translations = {
	{"bool", "bool"},
	{"float", "float"}, {"double", "double"},
	{"int8", "int8_t"}, {"int16", "int16_t"}, {"int32", "int32_t"}, {"int64", "int64_t"},
	{"uint8", "uint8_t"}, {"uint16", "uint16_t"}, {"uint32", "uint32_t"}, {"uint64", "uint64_t"},
	{"string", "std::string"},
	{"vec2", "Vec2"}, {"vec3", "Vec3"}, {"vec4", "Vec4"},
	{"uuid", "boost::uuids::uuid"}
};

std::string translateCpp(const std::string& type)
{
	auto i = basic_translations.find(type);
	if (i != basic_translations.end())
		return i->second;
	return type;
}

const int MAX_SMALL_PACKET_SIZE = 508;

void serializeFieldCpp(std::ofstream& f, Field field, const std::map<std::string, Structure>& types)
{
	switch (field.special)
	{
	case FS_NONE:
	{
		if (field.type)
		{
			if (!field.type->flat())
			{
				f << "	" << field.name << ".serialize(os);" << std::endl;
				break;
			}
		}
		if (field.type_name == "string")
		{
			f << "	{" << std::endl;
			f << "		uint16_t size = " << field.name << ".size();" << std::endl;
			f << "		os.write((char*)&size, sizeof(size));" << std::endl;
			f << "		os.write((char*)" << field.name << ".data(), size);" << std::endl;
			f << "	}" << std::endl;
			break;
		}
		f << "	os.write((char*)&" << field.name << ", sizeof(" << field.name << "));" << std::endl;
	}
	break;
	case FS_POINTER:
	{
		if (field.type && field.type->child_type_names.size() > 1)
		{
			f << "	if (" << field.name << ")" << std::endl;
			f << "	{" << std::endl;
			f << "		switch (" << field.name << "->getChildTypeIndex())" << std::endl;
			f << "		{" << std::endl;
			for (auto& child_type_name : field.type->child_type_names)
			{
				f << "			case " << types.at(child_type_name).child_type_index << ":" << std::endl;
				f << "			{" << std::endl;
				f << "				os.put(" << types.at(child_type_name).child_type_index << ");" << std::endl;
				Field value = field;
				value.name = "value";
				value.special = FS_NONE;
				if (value.flat())
				{
					f << "				os.write((char*)" << field.name << ".get() + sizeof(char*), sizeof(" << translateCpp(child_type_name) << ") - sizeof(char*));" << std::endl;
				}
				else
				{
					f << "				" << child_type_name << "& value = *" << field.name << ";" << std::endl;
					serializeFieldCpp(f, value, types);
				}
				f << "				break;" << std::endl;
				f << "			}" << std::endl;
			}
			f << "		}" << std::endl;
			f << "	}" << std::endl;
			f << "	else" << std::endl;
			f << "	{" << std::endl;
			f << "		os.put(0xff);" << std::endl;
			f << "	}" << std::endl;
		}
		else
		{
			f << "	if (" << field.name << ")" << std::endl;
			f << "	{" << std::endl;
			f << "		os.put(true);" << std::endl;
			Field value = field;
			value.name = "value";
			value.special = FS_NONE;
			if (value.flat())
			{
				f << "		os.write((char*)" << field.name << ".get(), sizeof(" << translateCpp(field.type_name) << "));" << std::endl;
			}
			else
			{
				f << "		auto&& value = *" << field.name << ";" << std::endl;
				serializeFieldCpp(f, value, types);
			}
			f << "	}" << std::endl;
			f << "	else" << std::endl;
			f << "	{" << std::endl;
			f << "		os.put(false);" << std::endl;
			f << "	}" << std::endl;
		}
	}
	break;
	case FS_VECTOR:
	{
		f << "	{" << std::endl;
		f << "		uint16_t size = " << field.name << ".size();" << std::endl;
		f << "		os.write((char*)&size, sizeof(size));" << std::endl;
		Field element = field;
		element.name = "element";
		element.special = FS_NONE;
		if (element.flat())
		{
			// TODO add static assert that type is trivially copyable
			f << "		os.write((char*)" << field.name << ".data(), sizeof(" << translateCpp(field.type_name) << ") * size);" << std::endl;
		}
		else
		{
			f << "		for (size_t i = 0; i < size; ++i)" << std::endl;
			f << "		{" << std::endl;
			f << "			auto&& element = " << field.name << "[i];" << std::endl;
			serializeFieldCpp(f, element, types);
			f << "		}" << std::endl;
		}
		f << "	}" << std::endl;
	}
	break;
	}
}

void deserializeFieldCpp(std::ofstream& f, Field field, const std::map<std::string, Structure>& types)
{
	switch (field.special)
	{
	case FS_NONE:
	{
		if (field.type)
		{
			if (!field.type->flat())
			{
				f << "	" << field.name << ".deserialize(is);" << std::endl;
				break;
			}
		}
		if (field.type_name == "string")
		{
			f << "	{" << std::endl;
			f << "		uint16_t size;" << std::endl;
			f << "		is.read((char*)&size, sizeof(size));" << std::endl;
			f << "		" << field.name << ".resize(size);" << std::endl;
			f << "		is.read((char*)" << field.name << ".data(), size);" << std::endl;
			f << "	}" << std::endl;
			break;
		}
		f << "	is.read((char*)&" << field.name << ", sizeof(" << field.name << "));" << std::endl;
	}
	break;
	case FS_POINTER:
	{
		if (field.type && field.type->child_type_names.size() > 1)
		{
			f << "	switch (is.get())" << std::endl;
			f << "	{" << std::endl;
			for (auto& child_type_name : field.type->child_type_names)
			{
				f << "		case " << types.at(child_type_name).child_type_index << ":" << std::endl;
				f << "		{" << std::endl;
				f << "			" << field.name << " = std::make_unique<" << translateCpp(child_type_name) << ">();" << std::endl;
				Field value = field;
				value.name = "value";
				value.special = FS_NONE;
				if (value.flat())
				{
					f << "			is.read((char*)" << field.name << ".get() + sizeof(char*), sizeof(" << translateCpp(child_type_name) << ") - sizeof(char*));" << std::endl;
				}
				else
				{
					f << "			" << child_type_name << "& value = *" << field.name << ";" << std::endl;
					deserializeFieldCpp(f, value, types);
				}
				f << "			break;" << std::endl;
				f << "		}" << std::endl;
			}
			f << "		default:" << std::endl;
			f << "			break;" << std::endl;
			f << "	}" << std::endl;
		}
		else
		{
			f << "	if (is.get())" << std::endl;
			f << "	{" << std::endl;
			f << "		" << field.name << " = std::make_unique<" << translateCpp(field.type_name) << ">();" << std::endl;
			Field value = field;
			value.name = "value";
			value.special = FS_NONE;
			if (value.flat())
			{
				f << "		is.read((char*)" << field.name << ".get(), sizeof(" << translateCpp(field.type_name) << "));" << std::endl;
			}
			else
			{
				f << "		auto&& value = *" << field.name << ";" << std::endl;
				deserializeFieldCpp(f, value, types);
			}
			f << "	}" << std::endl;
		}
	}
	break;
	case FS_VECTOR:
	{
		f << "	{" << std::endl;
		f << "		uint16_t size;" << std::endl;
		f << "		is.read((char*)&size, sizeof(size));" << std::endl;
		f << "		" << field.name << ".resize(size);" << std::endl;
		Field element = field;
		element.name = "element";
		element.special = FS_NONE;
		if (element.flat())
		{
			f << "		is.read((char*)" << field.name << ".data(), sizeof(" << translateCpp(field.type_name) << ") * size);" << std::endl;
		}
		else
		{
			f << "		for (size_t i = 0; i < size; ++i)" << std::endl;
			f << "		{" << std::endl;
			f << "			auto&& element = " << field.name << "[i];" << std::endl;
			deserializeFieldCpp(f, element, types);
			f << "		}" << std::endl;
		}
		f << "	}" << std::endl;
	}
	break;
	}
}

void CppGenerator::generate(const std::map<std::string, Structure>& types, const Protocol& protocol) const
{
	auto& destination_path = folder;

	for (auto& type : types)
	{
		{
			std::ofstream f(destination_path / (type.first + ".h"));

			f << "#pragma once" << std::endl << std::endl;

			f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

			for (auto& dependency : type.second.system_dependencies)
			{
				f << "#include <" << dependency << ">" << std::endl;
			}
			f << "#include <cstdint>" << std::endl << std::endl;
			f << "#include <iostream>" << std::endl << std::endl;

			for (auto& dependency : type.second.dependencies)
			{
				f << "#include \"" << dependency << ".h\"" << std::endl;
			}
			for (auto& dependency : type.second.application_dependencies)
			{
				if (builtins_in_superdirectory)
					f << "#include \"../" << dependency << ".h\"" << std::endl;
				else
					f << "#include \"" << dependency << ".h\"" << std::endl;
			}
			if (type.second.dependencies.size() + type.second.application_dependencies.size())
				f << std::endl;

			for (auto& dependency : type.second.delayed_dependencies)
			{
				f << "class " << dependency << ";" << std::endl;
			}
			if (type.second.delayed_dependencies.size())
				f << std::endl;

			f << "#pragma pack(push, 1)" << std::endl;

			if (type.second.parent_name.size() > 0)
			{
				f << "class " << type.first << " :" << std::endl;
				f << "\tpublic " << type.second.parent_name << std::endl;
			}
			else
				f << "class " << type.first << std::endl;
			f << "{" << std::endl;

			f << "public:" << std::endl;

			if (type.second.child_type_names.size() > 1)
			{
				f << "	virtual ~" << type.first << "();" << std::endl;
			}
			if (type.second.child_type_index != 0xff)
			{
				f << "\t";
				if (type.second.child_type_names.size() > 1)
					f << "virtual ";
				f << "uint8_t getChildTypeIndex() const;" << std::endl;
			}

			for (auto& field : type.second.fields)
			{
				switch (field.special)
				{
				case FS_NONE:
					f << "	" << translateCpp(field.type_name) << " " << field.name << ";" << std::endl;
					break;
				case FS_POINTER:
					f << "	std::unique_ptr<" << translateCpp(field.type_name) << "> " << field.name << ";" << std::endl;
					break;
				case FS_VECTOR:
					f << "	std::vector<" << translateCpp(field.type_name) << "> " << field.name << ";" << std::endl;
					break;
				}
			}
			if (type.second.fields.size())
				f << std::endl;

			f << "	void serialize(std::ostream& os) const;" << std::endl;
			f << "	void deserialize(std::istream& is);" << std::endl;

			f << "};" << std::endl;

			f << "#pragma pack(pop)" << std::endl;

			f.close();
		}

		{
			std::ofstream f(destination_path / (type.first + ".cpp"));

			f << "#include \"" << type.first << ".h\"" << std::endl << std::endl;

			f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

			f << "#include <iostream>" << std::endl << std::endl;

			for (auto& dependency : type.second.delayed_dependencies)
			{
				f << "#include \"" << dependency << ".h\"" << std::endl;
			}
			if (type.second.delayed_dependencies.size())
				f << std::endl;

			for (auto& dependency : type.second.hidden_dependencies)
			{
				f << "#include \"" << dependency << ".h\"" << std::endl;
			}
			if (type.second.hidden_dependencies.size())
				f << std::endl;

			if (type.second.child_type_names.size() > 1)
			{
				f << type.first << "::~" << type.first << "() {}" << std::endl;
			}
			if (type.second.child_type_index != 0xff)
			{
				f << "uint8_t " << type.first << "::getChildTypeIndex() const { return " << type.second.child_type_index << "; }" << std::endl;
			}

			f << "void " << type.first << "::serialize(std::ostream& os) const" << std::endl;
			f << "{" << std::endl;

			if (type.second.flat())
			{
				f << "	os.write((char*)this, sizeof(" << type.first << "));" << std::endl;
			}
			else
			{
				for (Field field : type.second.parent_fields)
				{
					field.name = "this->" + field.name;
					serializeFieldCpp(f, field, types);
				}
				for (Field field : type.second.fields)
				{
					field.name = "this->" + field.name;
					serializeFieldCpp(f, field, types);
				}
			}

			f << "}" << std::endl << std::endl;

			f << "void " << type.first << "::deserialize(std::istream& is)" << std::endl;
			f << "{" << std::endl;

			if (type.second.flat())
			{
				f << "	is.read((char*)this, sizeof(" << type.first << "));" << std::endl;
			}
			else
			{
				for (Field field : type.second.parent_fields)
				{
					field.name = "this->" + field.name;
					deserializeFieldCpp(f, field, types);
				}
				for (Field field : type.second.fields)
				{
					field.name = "this->" + field.name;
					deserializeFieldCpp(f, field, types);
				}
			}

			f << "}" << std::endl << std::endl;

			f.close();
		}
	}

	bool can_accept = up;
	bool can_connect = down;

	std::string link_name = protocol.prefix + "Link";

	{
		std::ofstream f(destination_path / (link_name + ".h"));

		f << "#pragma once" << std::endl << std::endl;

		f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

		f << "#include <asio.hpp>" << std::endl << std::endl;

		if (can_accept)
		{
			f << "#include <map>" << std::endl << std::endl;
		}

		for (auto& type : types)
		{
			f << "#include \"" << type.first << ".h\"" << std::endl;
		}
		if (types.size())
			f << std::endl;

		f << "class " << protocol.handler << ";" << std::endl << std::endl;

		f << "class " << link_name << std::endl;
		f << "{" << std::endl;
		f << "public:" << std::endl;

		f << "	" << protocol.handler << " * handler;" << std::endl;

		f << "	" << link_name << "();" << std::endl;

		f << "	void Open(const asio::ip::udp::endpoint& endpoint);" << std::endl;

		f << "	void Receive();" << std::endl;

		if (can_connect)
		{
			f << "	void Connect(const asio::ip::udp::endpoint& endpoint);" << std::endl;
		}

		if (can_accept)
		{
			f << "	void Disconnect(const asio::ip::udp::endpoint& endpoint);" << std::endl;
		}

		f << "	void Dispatch(asio::streambuf& buffer, const asio::ip::udp::endpoint& endpoint);" << std::endl;

		f << "	void SendBigPacket(const asio::ip::udp::endpoint& endpoint, const std::shared_ptr<asio::streambuf>& buffer);" << std::endl;

		for (auto& type : types)
		{
			if (up && type.second.down || down && type.second.up)
			{
				f << "	void Send(const asio::ip::udp::endpoint& endpoint, const " << type.first << "& message);" << std::endl;
			}
			if (up && type.second.used_up || down && type.second.used_down)
			{
				if (type.second.child_type_names.size() > 1)
				{
					f << "	void Dispatch(const " << type.first << " * message, const asio::ip::udp::endpoint& endpoint);" << std::endl;
				}
			}
		}

		f << "	static const uint32_t crc;" << std::endl << std::dec;

		f << "private:" << std::endl;

		f << "	asio::io_context io_context;" << std::endl;
		f << "	asio::ip::udp::socket socket;" << std::endl;
		f << "	asio::executor_work_guard<asio::io_context::executor_type> work_guard;" << std::endl;

		if (can_accept)
		{
			f << "	std::map<asio::ip::udp::endpoint, int64_t> connections;" << std::endl;
		}

		f << "};" << std::endl;

		f.close();
	}

	{
		std::ofstream f(destination_path / (link_name + ".cpp"));

		f << "#include \"" << link_name << ".h\"" << std::endl << std::endl;

		f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

		f << "// Application should implement this class using the prototypes in HandlerPrototypes.h" << std::endl;
		f << "#include \"../" << protocol.handler << ".h\"" << std::endl << std::endl;

		f << "const uint32_t " << link_name << "::crc = 0x" << std::hex << protocol.crc << ";" << std::endl << std::dec;

		f << "const size_t MAX_SMALL_PACKET_SIZE = " << MAX_SMALL_PACKET_SIZE << ";" << std::endl;
		f << "const size_t BIG_PACKET_CHUNK_SIZE = MAX_SMALL_PACKET_SIZE - " << 4 << ";" << std::endl;

		f << link_name << "::" << link_name << "() : io_context(), socket(io_context), work_guard(asio::make_work_guard(io_context))" << std::endl;
		f << "{" << std::endl;
		f << "}" << std::endl << std::endl;

		f << "void " << link_name << "::Open(const asio::ip::udp::endpoint& endpoint)" << std::endl;
		f << "{" << std::endl;
		f << "	socket.open(endpoint.protocol());" << std::endl;
		f << "	socket.bind(endpoint);" << std::endl;
		f << "}" << std::endl << std::endl;

		f << "void " << link_name << "::Receive()" << std::endl;
		f << "{" << std::endl;
		f << "	{" << std::endl;
		f << "		std::thread t([this]()" << std::endl;
		f << "			{" << std::endl;
		f << "				while (true)" << std::endl;
		f << "				{" << std::endl;
		f << "					try" << std::endl;
		f << "					{" << std::endl;
		f << "						asio::streambuf buffer(65507); " << std::endl;
		f << "						asio::ip::udp::endpoint endpoint;" << std::endl;
		f << "						buffer.commit(socket.receive_from(buffer.prepare(65507), endpoint));" << std::endl;
		f << "						Dispatch(buffer, endpoint);" << std::endl;
		f << "					}" << std::endl;
		f << "					catch (...)" << std::endl;
		f << "					{" << std::endl;
		f << "					}" << std::endl;
		f << "				}" << std::endl;
		f << "			}" << std::endl;
		f << "		);" << std::endl;
		f << "		t.detach();" << std::endl;
		f << "	}" << std::endl;
		f << "	{" << std::endl;
		f << "		std::thread t([this]()" << std::endl;
		f << "			{" << std::endl;
		f << "				io_context.run();" << std::endl;
		f << "			}" << std::endl;
		f << "		);" << std::endl;
		f << "		t.detach();" << std::endl;
		f << "	}" << std::endl;
		f << "}" << std::endl << std::endl;

		if (can_connect)
		{
			f << "void " << link_name << "::Connect(const asio::ip::udp::endpoint& endpoint)" << std::endl;
			f << "{" << std::endl;
			f << "	std::shared_ptr<asio::streambuf> buffer = std::make_shared<asio::streambuf>();" << std::endl;
			f << "	std::ostream os(buffer.get());" << std::endl;
			f << "	os.put(0);" << std::endl;
			f << "	os.write((const char*)&crc, sizeof(crc));" << std::endl;
			f << "	os.put(0);" << std::endl;
			f << "	socket.async_send_to(buffer->data(), endpoint, [buffer](const asio::error_code&, size_t) {});" << std::endl;
			f << "}" << std::endl << std::endl;
		}

		if (can_accept)
		{
			f << "void " << link_name << "::Disconnect(const asio::ip::udp::endpoint& endpoint)" << std::endl;
			f << "{" << std::endl;
			f << "	auto it = connections.find(endpoint);" << std::endl;
			f << "	if (it != connections.end())" << std::endl;
			f << "	{" << std::endl;
			f << "		connections.erase(it);" << std::endl;
			f << "		std::shared_ptr<asio::streambuf> buffer = std::make_shared<asio::streambuf>();" << std::endl;
			f << "		std::ostream os(buffer.get());" << std::endl;
			f << "		os.put(0);" << std::endl;
			f << "		os.write((const char*)&crc, sizeof(crc));" << std::endl;
			f << "		os.put(2);" << std::endl;
			f << "		os.put(1);" << std::endl;
			f << "		socket.async_send_to(buffer->data(), endpoint, [buffer](const asio::error_code&, size_t) {});" << std::endl;
			f << "	}" << std::endl;
			f << "}" << std::endl << std::endl;
		}

		{
			f << "void " << link_name << "::Dispatch(asio::streambuf& buffer, const asio::ip::udp::endpoint& endpoint)" << std::endl;
			f << "{" << std::endl;
			f << "	std::istream is(&buffer);" << std::endl;
			f << "	char c = is.get();" << std::endl;
			f << "	if (c == 0)" << std::endl;

			{
				f << "		{" << std::endl;
				f << "			uint32_t remote_crc;" << std::endl;
				f << "			is.read((char*)&remote_crc, sizeof(remote_crc));" << std::endl;
				f << "			if (remote_crc != crc)" << std::endl;
				f << "			{" << std::endl;
				if (can_accept)
				{
					f << "				std::shared_ptr<asio::streambuf> buffer = std::make_shared<asio::streambuf>();" << std::endl;
					f << "				std::ostream os(buffer.get());" << std::endl;
					f << "				os.put(0);" << std::endl;
					f << "				os.write((const char*)&crc, sizeof(crc));" << std::endl;
					f << "				os.put(2);" << std::endl;
					f << "				os.put(0);" << std::endl;
					f << "				socket.async_send_to(buffer->data(), endpoint, [buffer](const asio::error_code&, size_t) {});" << std::endl;
				}
				f << "				return;" << std::endl;
				f << "			}" << std::endl;
				f << "			switch (is.get())" << std::endl;
				f << "			{" << std::endl;
				f << "				case 0:" << std::endl;
				f << "				{" << std::endl;
				if (can_accept)
				{
					f << "					int64_t time = std::chrono::steady_clock::now().time_since_epoch().count();" << std::endl;
					f << "					connections[endpoint] = time;" << std::endl;
					f << "					std::shared_ptr<asio::streambuf> buffer = std::make_shared<asio::streambuf>();" << std::endl;
					f << "					std::ostream os(buffer.get());" << std::endl;
					f << "					os.put(0);" << std::endl;
					f << "					os.write((const char*)&crc, sizeof(crc));" << std::endl;
					f << "					os.put(1);" << std::endl;
					f << "					socket.async_send_to(buffer->data(), endpoint, [buffer](const asio::error_code&, size_t) {});" << std::endl;
					f << "					handler->AcceptHandler(endpoint);" << std::endl;
				}
				f << "					break;" << std::endl;
				f << "				}" << std::endl;
				f << "				case 1:" << std::endl;
				f << "				{" << std::endl;
				if (can_connect)
				{
					f << "					handler->ConnectHandler(endpoint);" << std::endl;
				}
				f << "					break;" << std::endl;
				f << "				}" << std::endl;
				f << "				case 2:" << std::endl;
				f << "				{" << std::endl;
				f << "					handler->DisconnectHandler(endpoint, is.get());" << std::endl;
				if (can_accept)
				{

					f << "					auto it = connections.find(endpoint);" << std::endl;
					f << "					if (it != connections.end())" << std::endl;
					f << "						connections.erase(it);" << std::endl;
				}
				f << "					break;" << std::endl;
				f << "				}" << std::endl;
				f << "				default:" << std::endl;
				f << "					break;" << std::endl;
				f << "			}" << std::endl;
				f << "			return;" << std::endl;
				f << "		}" << std::endl;
			}

			if (can_accept)
			{
				f << "	auto it = connections.find(endpoint);" << std::endl;
				f << "	if (it == connections.end())" << std::endl;
				f << "		return;" << std::endl;
				/*f << "	if (time - it->second > 10'000'000'000)" << std::endl;
				f << "	{" << std::endl;
				f << "		connections.erase(it);" << std::endl;
				f << "		return;" << std::endl;
				f << "	}" << std::endl;
				f << "	connections[endpoint] = time;" << std::endl;*/
			}

			f << "	switch (c)" << std::endl;
			f << "	{" << std::endl;

			{
				uint8_t message_index = 1;
				for (auto& [name, type] : types) // TODO only message types
				{
					if (up && type.up || down && type.down)
					{
						f << "	case " << std::to_string(message_index) << ":" << std::endl;
						f << "	{" << std::endl;
						f << "		" << name << " message;" << std::endl;
						f << "		message.deserialize(is);" << std::endl;
						f << "		handler->" << name << "Handler(endpoint, message);" << std::endl;
						f << "		break;" << std::endl;
						f << "	}" << std::endl;
					}
					if (type.down || type.up)
						++message_index;
				}
			}

			f << "	default:" << std::endl;
			f << "		break;" << std::endl;

			f << "	}" << std::endl;
			f << "}" << std::endl << std::endl;
		}

		{
			f << "void " << link_name << "::SendBigPacket(const asio::ip::udp::endpoint& endpoint, const std::shared_ptr<asio::streambuf>& buffer)" << std::endl;
			f << "{" << std::endl;
			f << "	std::istream is(buffer.get());" << std::endl;
			f << "	auto it = connections.find(endpoint);" << std::endl;
			f << "	if (it == connections.end())" << std::endl;
			f << "		return;" << std::endl;
			f << "	uint8_t message_id = it->second++;" << std::endl;
			f << "	for (uint16_t i = 0; i < 65535; ++i)" << std::endl;
			f << "	{" << std::endl;
			f << "		std::shared_ptr<asio::streambuf> partial_buffer = std::make_shared<asio::streambuf>();" << std::endl;
			f << "		std::ostream os(partial_buffer.get());" << std::endl;
			f << "		std::streamsize chunk_size = buffer->size() - i * BIG_PACKET_CHUNK_SIZE;" << std::endl;
			f << "		if (chunk_size > BIG_PACKET_CHUNK_SIZE)" << std::endl;
			f << "			chunk_size = BIG_PACKET_CHUNK_SIZE;" << std::endl;
			f << "		os.put(255);" << std::endl;
			f << "		os.write((char*)&message_id, sizeof(message_id));" << std::endl;
			f << "		os.write((char*)&i, sizeof(i));" << std::endl;
			f << "		os.write((char*)buffer->data().data() + i * BIG_PACKET_CHUNK_SIZE, chunk_size);" << std::endl;
			f << "		socket.async_send_to(partial_buffer->data(), endpoint, [partial_buffer](const asio::error_code&, size_t) {});" << std::endl;
			f << "		if (chunk_size < BIG_PACKET_CHUNK_SIZE)" << std::endl;
			f << "			break;" << std::endl;
			f << "	}" << std::endl;
			f << "}" << std::endl << std::endl;
		}

		{
			uint8_t message_index = 1;
			for (auto& [name, type] : types) // TODO only message types
			{
				if (up && type.down || down && type.up)
				{
					f << "void " << link_name << "::Send(const asio::ip::udp::endpoint& endpoint, const " << name << "& message)" << std::endl;
					f << "{" << std::endl;
					f << "	std::shared_ptr<asio::streambuf> buffer = std::make_shared<asio::streambuf>();" << std::endl;
					f << "	std::ostream os(buffer.get());" << std::endl;
					f << "	os.put(" << std::to_string(message_index) << ");" << std::endl;
					f << "	message.serialize(os);" << std::endl;
					if (type.maxSize() > MAX_SMALL_PACKET_SIZE)
					{
						f << "	if (buffer->size() > MAX_SMALL_PACKET_SIZE)" << std::endl;
						f << "		SendBigPacket(endpoint, buffer);" << std::endl;
						f << "	else" << std::endl;
						f << "	";
					}
					f << "	socket.async_send_to(buffer->data(), endpoint, [buffer](const asio::error_code&, size_t) {});" << std::endl;
					f << "}" << std::endl << std::endl;
				}
				if (up && type.used_up || down && type.used_down)
				{
					if (type.child_type_names.size() > 1)
					{
						f << "void " << link_name << "::Dispatch(const " << name << " * message, const asio::ip::udp::endpoint& endpoint)" << std::endl;
						f << "{" << std::endl;
						f << "	if (message == nullptr)" << std::endl;
						f << "		return;" << std::endl;
						f << "	switch (message->getChildTypeIndex())" << std::endl;
						f << "	{" << std::endl;
						for (auto& child_type_name : type.child_type_names)
						{
							f << "		case " << types.at(child_type_name).child_type_index << ":" << std::endl;
							f << "		{" << std::endl;
							f << "			handler->" << child_type_name << "Handler(endpoint, *(const " << child_type_name << "*)message);" << std::endl;
							f << "			break;" << std::endl;
							f << "		}" << std::endl;
						}
						f << "	}" << std::endl;
						f << "}" << std::endl << std::endl;
					}
				}
				if (type.down || type.up)
					++message_index;
			}
		}

		f.close();
	}

	{
		std::ofstream f(destination_path / (protocol.prefix + "HandlerPrototypes.h"));

		if (can_accept)
		{
			f << "	void AcceptHandler(const asio::ip::udp::endpoint& endpoint);" << std::endl;
		}

		if (can_connect)
		{
			f << "	void ConnectHandler(const asio::ip::udp::endpoint& endpoint);" << std::endl;
		}

		f << "	void DisconnectHandler(const asio::ip::udp::endpoint& endpoint, uint8_t code);" << std::endl;

		f << std::endl;

		for (auto& [name, type] : types)
		{
			bool handlerNeeded = up && type.up || down && type.down;
			handlerNeeded |= up && type.used_up || down && type.used_down;
			auto parent = types.find(name);
			while (parent->second.parent_name.size() > 0)
			{
				parent = types.find(parent->second.parent_name);
				handlerNeeded |= up && parent->second.used_up || down && parent->second.used_down;
			}
			if (handlerNeeded)
				f << "	void " << name << "Handler(const asio::ip::udp::endpoint& endpoint, const " << name << "& message);" << std::endl;
		}

		f.close();
	}
}
