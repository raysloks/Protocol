#include "CppGenerator.h"

#include <fstream>

std::map<std::string, std::string> basic_translations = { {"float", "float"}, {"double", "double"},
	{"int8", "int8_t"}, {"int16", "int16_t"}, {"int32", "int32_t"}, {"int64", "int64_t"},
	{"uint8", "uint8_t"}, {"uint16", "uint16_t"}, {"uint32", "uint32_t"}, {"uint64", "uint64_t"},
	{"string", "std::string"},
	{"vec2", "Vec2"}, {"vec3", "Vec3"}, {"vec4", "Vec4"} };

std::string translateCpp(const std::string& type)
{
	auto i = basic_translations.find(type);
	if (i != basic_translations.end())
		return i->second;
	return type;
}

void serializeFieldCpp(std::ofstream& f, Field field)
{
	switch (field.special)
	{
	case FS_NONE:
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
			f << "		uint16_t size = this->" << field.name << ".size();" << std::endl;
			f << "		os.write((char*)&size, sizeof(size));" << std::endl;
			f << "		os.write((char*)this->" << field.name << ".data(), size);" << std::endl;
			f << "	}" << std::endl;
			break;
		}
		f << "	os.write((char*)&" << field.name << ", (sizeof(" << field.name << ") + 3) / 4 * 4);" << std::endl;
		break;
	case FS_POINTER:
		f << "	if (" << field.name << ")" << std::endl;
		f << "	{" << std::endl;
		f << "		os.put(true);" << std::endl;
		f << "		" << field.name << "->serialize(os);" << std::endl;
		f << "	}" << std::endl;
		f << "	else" << std::endl;
		f << "	{" << std::endl;
		f << "		os.put(false);" << std::endl;
		f << "	}" << std::endl;
		break;
	case FS_VECTOR:
		f << "	{" << std::endl;
		f << "		uint16_t size = this->" << field.name << ".size();" << std::endl;
		f << "		os.write((char*)&size, sizeof(size));" << std::endl;
		if (field.type)
		{
			if (!field.type->flat())
			{
				f << "		for (size_t i = 0; i < size; ++i)" << std::endl;
				f << "			this->" << field.name << "[i]->serialize(os);" << std::endl;
				f << "	}" << std::endl;
				break;
			}
		}
		f << "		os.write((char*)&this->" << field.name << ".data(), sizeof(" << field.type_name << ") * size);" << std::endl;
		f << "	}" << std::endl;
		break;
	}
}

void deserializeFieldCpp(std::ofstream& f, Field field)
{
	switch (field.special)
	{
	case FS_NONE:
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
			f << "		this->" << field.name << ".resize(size);" << std::endl;
			f << "		is.read((char*)this->" << field.name << ".data(), size);" << std::endl;
			f << "	}" << std::endl;
			break;
		}
		f << "	is.read((char*)&" << field.name << ", (sizeof(" << field.name << ") + 3) / 4 * 4);" << std::endl;
		break;
	case FS_POINTER:
		f << "	if (is.get())" << std::endl;
		f << "	{" << std::endl;
		f << "		" << field.name << " = std::make_unique<" << field.type_name << ">();" << std::endl;
		f << "		" << field.name << "->deserialize(is);" << std::endl;
		f << "	}" << std::endl;
		break;
	case FS_VECTOR:
		f << "	{" << std::endl;
		f << "		uint16_t size;" << std::endl;
		f << "		is.read((char*)&size, sizeof(size));" << std::endl;
		f << "		this->" << field.name << ".resize(size);" << std::endl;
		if (field.type)
		{
			if (!field.type->flat())
			{
				f << "		for (size_t i = 0; i < size; ++i)" << std::endl;
				f << "			this->" << field.name << "[i]->deserialize(is);" << std::endl;
				f << "	}" << std::endl;
				break;
			}
		}
		f << "		is.read((char*)&this->" << field.name << ".data(), sizeof(" << field.type_name << ") * size);" << std::endl;
		f << "	}" << std::endl;
		break;
	}
}

void CppGenerator::generate(const std::filesystem::path & folder, const std::map<std::string, Structure>& types, const Protocol& protocol) const
{
	auto destination_path = folder;

	for (auto type : types)
	{
		{
			std::ofstream f(destination_path / (type.first + ".h"));

			f << "#pragma once" << std::endl << std::endl;

			f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

			for (auto dependency : type.second.system_dependencies)
			{
				f << "#include <" << dependency << ">" << std::endl;
			}
			f << "#include <iostream>" << std::endl << std::endl;

			for (auto dependency : type.second.dependencies)
			{
				f << "#include \"" << dependency << ".h\"" << std::endl;
			}
			for (auto dependency : type.second.application_dependencies)
			{
				f << "#include \"../" << dependency << ".h\"" << std::endl;
			}
			if (type.second.dependencies.size() + type.second.application_dependencies.size())
				f << std::endl;

			for (auto dependency : type.second.delayed_dependencies)
			{
				f << "class " << dependency << ";" << std::endl;
			}
			if (type.second.delayed_dependencies.size())
				f << std::endl;

			f << "class " << type.first << std::endl;
			f << "{" << std::endl;

			f << "public:" << std::endl;

			for (auto field : type.second.fields)
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

			f.close();
		}

		{
			std::ofstream f(destination_path / (type.first + ".cpp"));

			f << "#include \"" << type.first << ".h\"" << std::endl << std::endl;

			f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

			f << "#include <iostream>" << std::endl << std::endl;

			for (auto dependency : type.second.delayed_dependencies)
			{
				f << "#include \"" << dependency << ".h\"" << std::endl;
			}
			if (type.second.delayed_dependencies.size())
				f << std::endl;

			f << "void " << type.first << "::serialize(std::ostream& os) const" << std::endl;
			f << "{" << std::endl;

			if (type.second.flat())
			{
				f << "	os.write((char*)this, sizeof(" << type.first << "));" << std::endl;
			}
			else
			{
				for (auto field : type.second.fields)
				{
					serializeFieldCpp(f, field);
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
				for (auto field : type.second.fields)
				{
					deserializeFieldCpp(f, field);
				}
			}

			f << "}" << std::endl << std::endl;

			f.close();
		}
	}

	{
		std::ofstream f(destination_path / "Link.h");

		f << "#pragma once" << std::endl << std::endl;

		f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

		f << "#include <asio.hpp>" << std::endl << std::endl;

		for (auto type : types)
		{
			f << "#include \"" << type.first << ".h\"" << std::endl;
		}
		if (types.size())
			f << std::endl;

		f << "class " << protocol.name << "Handler;" << std::endl << std::endl;

		f << "class Link" << std::endl;
		f << "{" << std::endl;
		f << "public:" << std::endl;

		f << "	" << protocol.name << "Handler * handler;" << std::endl;

		f << "	Link();" << std::endl;

		f << "	void Open(const asio::ip::udp::endpoint& endpoint);" << std::endl;

		f << "	void Receive();" << std::endl;

		f << "	void Dispatch(asio::streambuf& buffer, const asio::ip::udp::endpoint& endpoint) const;" << std::endl;

		for (auto type : types) // TODO only message types
		{
			f << "	void Send(const asio::ip::udp::endpoint& endpoint, const " << type.first << "& message);" << std::endl;
		}

		f << "	static const uint32_t crc = 0x" << std::hex << protocol.crc << ";" << std::endl << std::dec;

		f << "private:" << std::endl;

		f << "	asio::io_context io_context;" << std::endl;
		f << "	asio::ip::udp::socket socket;" << std::endl;

		f << "};" << std::endl;

		f.close();
	}

	{
		std::ofstream f(destination_path / "Link.cpp");

		f << "#include \"Link.h\"" << std::endl << std::endl;

		f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

		f << "// Application should implement this class using the prototypes in HandlerPrototypes.h" << std::endl;
		f << "#include \"../" << protocol.name << "Handler.h\"" << std::endl << std::endl;

		f << "Link::Link() : io_context(), socket(io_context)" << std::endl;
		f << "{" << std::endl;
		f << "}" << std::endl << std::endl;

		f << "void Link::Open(const asio::ip::udp::endpoint& endpoint)" << std::endl;
		f << "{" << std::endl;
		f << "	socket.open(endpoint.protocol());" << std::endl;
		f << "	socket.bind(endpoint);" << std::endl;
		f << "}" << std::endl << std::endl;

		f << "void Link::Receive()" << std::endl;
		f << "{" << std::endl;
		f << "	std::thread t([this]()" << std::endl;
		f << "		{" << std::endl;
		f << "			asio::streambuf buffer(65507); " << std::endl;
		f << "			asio::ip::udp::endpoint endpoint;" << std::endl;
		f << "			while (true)" << std::endl;
		f << "			{" << std::endl;
		f << "				buffer.commit(socket.receive_from(buffer.prepare(65507), endpoint));" << std::endl;
		f << "				Dispatch(buffer, endpoint);" << std::endl;
		f << "			}" << std::endl;
		f << "		}" << std::endl;
		f << "	);" << std::endl;
		f << "	t.detach();" << std::endl;
		f << "}" << std::endl << std::endl;

		{
			f << "void Link::Dispatch(asio::streambuf& buffer, const asio::ip::udp::endpoint& endpoint) const" << std::endl;
			f << "{" << std::endl;
			f << "	std::istream is(&buffer);" << std::endl;
			f << "	switch (is.get())" << std::endl;
			f << "	{" << std::endl;

			uint8_t message_index = 0;
			for (auto type : types) // TODO only message types
			{
				f << "	case " << std::to_string(message_index) << ":" << std::endl;
				f << "	{" << std::endl;
				f << "		" << type.first << " message;" << std::endl;
				f << "		message.deserialize(is);" << std::endl;
				f << "		handler->" << type.first << "Handler(endpoint, message);" << std::endl;
				f << "		break;" << std::endl;
				f << "	}" << std::endl;
				++message_index;
			}

			f << "	default:" << std::endl;
			f << "		break;" << std::endl;

			f << "	}" << std::endl;
			f << "}" << std::endl << std::endl;
		}

		{
			uint8_t message_index = 0;
			for (auto type : types) // TODO only message types
			{
				f << "void Link::Send(const asio::ip::udp::endpoint& endpoint, const " << type.first << "& message)" << std::endl;
				f << "{" << std::endl;
				f << "	std::shared_ptr<asio::streambuf> buffer = std::make_shared<asio::streambuf>();" << std::endl;
				f << "	std::ostream os(buffer.get());" << std::endl;
				f << "	os.put(" << std::to_string(message_index) << ");" << std::endl;
				f << "	message.serialize(os);" << std::endl;
				f << "	socket.async_send_to(buffer->data(), endpoint, [buffer](const asio::error_code&, size_t) {});" << std::endl;
				f << "}" << std::endl << std::endl;
				++message_index;
			}
		}

		f.close();
	}

	{
		std::ofstream f(destination_path / "HandlerPrototypes.h");

		for (auto type : types) // TODO only message types
		{
			f << "	void " << type.first << "Handler(const asio::ip::udp::endpoint& endpoint, const " << type.first << "& message);" << std::endl;
		}

		f.close();
	}
}
