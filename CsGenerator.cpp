#include "CsGenerator.h"

#include <fstream>

const std::map<std::string, std::string> reader_translations = { {"float", "Single"},
	{"int8_t", "SByte"}, {"int16_t", "Int16"}, {"int32_t", "Int32"}, {"int64_t", "Int64"},
	{"uint8_t", "Byte"}, {"uint16_t", "UInt16"}, {"uint32_t", "UInt32"}, {"uint64_t", "UInt64"} };

const std::map<std::string, std::string> basic_translations = { {"float", "float"},
	{"int8_t", "sbyte"}, {"int16_t", "short"}, {"int32_t", "int"}, {"int64_t", "long"},
	{"uint8_t", "bye"}, {"uint16_t", "ushort"}, {"uint32_t", "uint"}, {"uint64_t", "ulong"} };

std::string translate(const std::string& type)
{
	auto i = basic_translations.find(type);
	if (i != basic_translations.end())
		return i->second;
	return type;
}

void CsGenerator::generate(const std::filesystem::path& folder, const std::map<std::string, Structure>& types, const Protocol& protocol) const
{
	auto destination_path = folder;

	for (auto type : types)
	{
		std::ofstream f(destination_path / (type.first + ".cs"));

		f << "using System.IO;" << std::endl << std::endl;

		f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

		f << "public struct " << type.first << std::endl;
		f << "{" << std::endl;

		for (auto field : type.second.fields)
		{
			switch (field.special)
			{
			case FS_NONE:
				f << "	public " << translate(field.type_name) << " " << field.name << ";" << std::endl;
				break;
			case FS_POINTER:
				f << "	public " << translate(field.type_name) << "? " << field.name << ";" << std::endl;
				break;
			case FS_VECTOR:
				f << "	public List<" << translate(field.type_name) << "> " << field.name << ";" << std::endl;
				break;
			}
		}
		if (type.second.fields.size())
			f << std::endl;

		f << "	public void Serialize(BinaryWriter writer)" << std::endl;
		f << "	{" << std::endl;

		for (auto field : type.second.fields)
		{
			switch (field.special)
			{
			case FS_NONE:
				if (field.type)
				{
					f << "		" << field.name << ".Serialize(writer);" << std::endl;
					break;
				}
				f << "		writer.Write(" << field.name << ");" << std::endl;
				break;
			case FS_POINTER:
				f << "		if (" << field.name << ".HasValue)" << std::endl;
				f << "		{" << std::endl;
				f << "			writer.Write(true);" << std::endl;
				if (field.type)
				{
					f << "			" << field.name << ".Serialize(writer);" << std::endl;
				}
				else
				{
					f << "			writer.Write(" << field.name << ".Value);" << std::endl;
				}
				f << "		}" << std::endl;
				f << "		else" << std::endl;
				f << "		{" << std::endl;
				f << "			writer.Write(false);" << std::endl;
				f << "		}" << std::endl;
				break;
			case FS_VECTOR:
				f << "		{" << std::endl;
				f << "			ushort size = this." << field.name << ".Count;" << std::endl;
				f << "			writer.Write(size);" << std::endl;
				if (field.type)
				{
					f << "			foreach (var i in this." << field.name << ")" << std::endl;
					f << "				i.Serialize(writer);" << std::endl;
					f << "		}" << std::endl;
					break;
				}
				f << "			foreach (var i in this." << field.name << ")" << std::endl;
				f << "				writer.Write(i);" << std::endl;
				f << "		}" << std::endl;
				break;
			}
		}

		f << "	}" << std::endl << std::endl;

		f << "	public void Deserialize(BinaryReader reader)" << std::endl;
		f << "	{" << std::endl;

		for (auto field : type.second.fields)
		{
			switch (field.special)
			{
			case FS_NONE:
				if (field.type)
				{
					f << "		" << field.name << ".Deserialize(reader);" << std::endl;
					break;
				}
				f << "		" << field.name << " = reader.Read" << reader_translations.at(field.type_name) << "();" << std::endl;
				break;
			case FS_POINTER:
				f << "		if (reader.ReadBoolean())" << std::endl;
				f << "		{" << std::endl;
				if (field.type)
				{
					f << "			" << field.name << " = new " << field.type_name << "();" << std::endl;
					f << "			" << field.name << ".Deserialize(reader);" << std::endl;
				}
				else
				{
					f << "			" << field.name << " = reader.Read" << reader_translations.at(field.type_name) << "();" << std::endl;
				}
				f << "		}" << std::endl;
				break;
			case FS_VECTOR:
				f << "	{" << std::endl;
				f << "		ushort size = reader.ReadUInt16();" << std::endl;
				f << "		this." << field.name << ".Clear();" << std::endl;
				f << "		for (int i = 0; i < size; ++i)" << std::endl;
				f << "		{" << std::endl;
				if (field.type)
				{
					f << "			" << field.type_name << " element = new " << field.type_name << "();" << std::endl;
					f << "			element.Deserialize(reader);" << std::endl;
					f << "			this." << field.name << ".Add(element);" << std::endl;
				}
				else
				{
					f << "			this." << field.name << ".Add(reader.Read" << reader_translations.at(field.type_name) << "());" << std::endl;
				}
				f << "		}" << std::endl;
				f << "	}" << std::endl;
				break;
			}
		}

		f << "	}" << std::endl;

		f << "};" << std::endl;

		f.close();
	}

	{
		std::ofstream f(destination_path / "Link.cs");

		f << "using System.IO;" << std::endl;
		f << "using System.Net;" << std::endl;
		f << "using System.Net.Sockets;" << std::endl << std::endl;

		f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

		f << "public class Link" << std::endl;
		f << "{" << std::endl;

		f << "	public " << protocol.name << "Handler handler;" << std::endl;

		f << "	public void Open(IPEndPoint endpoint)" << std::endl;
		f << "	{" << std::endl;
		f << "		client = new UdpClient(endpoint);" << std::endl;
		f << "	}" << std::endl << std::endl;

		f << "	public void Receive()" << std::endl;
		f << "	{" << std::endl;
		f << "		client.ReceiveAsync().ContinueWith(task =>" << std::endl;
		f << "		{" << std::endl;
		f << "			Receive();" << std::endl;
		f << "			Dispatch(task.Result.Buffer, task.Result.RemoteEndPoint);" << std::endl;
		f << "		});" << std::endl;
		f << "	}" << std::endl << std::endl;

		uint8_t message_index = 0;
		for (auto type : types) // TODO only message types
		{
			f << "	public void Send(IPEndPoint endpoint, in " << type.first << " message)" << std::endl;
			f << "	{" << std::endl;
			f << "		MemoryStream stream = new MemoryStream();" << std::endl;
			f << "		BinaryWriter writer = new BinaryWriter(stream);" << std::endl;
			f << "		writer.Write((byte)" << std::to_string(message_index) << ");" << std::endl;
			f << "		message.Serialize(writer);" << std::endl;
			f << "		byte[] bytes = stream.ToArray();" << std::endl;
			f << "		client.SendAsync(bytes, bytes.Length, endpoint);" << std::endl;
			f << "	}" << std::endl << std::endl;
			++message_index;
		}

		{
			f << "	public void Dispatch(byte[] bytes, IPEndPoint endpoint)" << std::endl;
			f << "	{" << std::endl;
			f << "		BinaryReader reader = new BinaryReader(new MemoryStream(bytes));" << std::endl;
			f << "		switch (reader.ReadByte())" << std::endl;
			f << "		{" << std::endl;

			uint8_t message_index = 0;
			for (auto type : types) // TODO only message types
			{
				f << "		case " << std::to_string(message_index) << ":" << std::endl;
				f << "		{" << std::endl;
				f << "			" << type.first << " message = new " << type.first << "();" << std::endl;
				f << "			message.Deserialize(reader);" << std::endl;
				f << "			handler." << type.first << "Handler(endpoint, message);" << std::endl;
				f << "			break;" << std::endl;
				f << "		}" << std::endl;
				++message_index;
			}

			f << "		default:" << std::endl;
			f << "			break;" << std::endl;
			f << "		}" << std::endl;
			f << "	}" << std::endl << std::endl;
		}

		f << "	public const uint crc = 0x" << std::hex << protocol.crc << ";" << std::endl << std::dec;

		f << "	private UdpClient client;" << std::endl;

		f << "}" << std::endl;

		f.close();
	}
}
