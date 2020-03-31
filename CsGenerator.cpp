#include "CsGenerator.h"

#include <fstream>

const std::map<std::string, std::string> writer_translations = { {"float", "float"}, {"double", "double"},
	{"int8", "int"}, {"int16", "int"}, {"int32", "int"}, {"int64", "long"},
	{"uint8", "uint"}, {"uint16", "uint"}, {"uint32", "uint"}, {"uint64", "ulong"} };

const std::map<std::string, std::string> reader_translations = { {"float", "Single"}, {"double", "Double"},
	{"int8", "Int32"}, {"int16", "Int32"}, {"int32", "Int32"}, {"int64", "Int64"},
	{"uint8", "UInt32"}, {"uint16", "UInt32"}, {"uint32", "UInt32"}, {"uint64", "UInt64"} };

const std::map<std::string, std::string> basic_translations = { {"float", "float"}, {"double", "double"},
	{"int8", "sbyte"}, {"int16", "short"}, {"int32", "int"}, {"int64", "long"},
	{"uint8", "byte"}, {"uint16", "ushort"}, {"uint32", "uint"}, {"uint64", "ulong"},
	{"string", "string"},
	{"vec2", "Vector2"}, {"vec3", "Vector3"}, {"vec4", "Vector4"} };

std::string translateCs(const std::string& type)
{
	auto i = basic_translations.find(type);
	if (i != basic_translations.end())
		return i->second;
	return type;
}

void serializeFieldCs(std::ofstream& f, Field field)
{
	switch (field.special)
	{
	case FS_NONE:
		if (field.type)
		{
			f << "		" << field.name << ".Serialize(writer);" << std::endl;
			break;
		}
		if (field.type_name == "string")
		{
			f << "		{" << std::endl;
			f << "			byte[] bytes = System.Text.Encoding.UTF8.GetBytes(" << field.name << ");" << std::endl;
			f << "			ushort size = (ushort)bytes.Length;" << std::endl;
			f << "			writer.Write(size);" << std::endl;
			f << "			writer.Write(bytes);" << std::endl;
			f << "		}" << std::endl;
			break;
		}
		if (field.type_name == "vec2")
		{
			f << "		writer.Write(" << field.name << ".x);" << std::endl;
			f << "		writer.Write(" << field.name << ".y);" << std::endl;
			break;
		}
		if (field.type_name == "vec3")
		{
			f << "		writer.Write(" << field.name << ".x);" << std::endl;
			f << "		writer.Write(" << field.name << ".y);" << std::endl;
			f << "		writer.Write(" << field.name << ".z);" << std::endl;
			break;
		}
		if (field.type_name == "vec4")
		{
			f << "		writer.Write(" << field.name << ".x);" << std::endl;
			f << "		writer.Write(" << field.name << ".y);" << std::endl;
			f << "		writer.Write(" << field.name << ".z);" << std::endl;
			f << "		writer.Write(" << field.name << ".w);" << std::endl;
			break;
		}
		f << "		writer.Write((" << writer_translations.at(field.type_name) << ")" << field.name << ");" << std::endl;
		break;
	case FS_POINTER:
		f << "		if (" << field.name << ".HasValue)" << std::endl;
		f << "		{" << std::endl;
		f << "			writer.Write(true);" << std::endl;
		{
			Field value = field;
			value.special = FS_NONE;
			value.name += ".Value";
			serializeFieldCs(f, value);
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
		Field i = field;
		i.name = "i";
		i.special = FS_NONE;
		f << "			foreach (var i in this." << field.name << ")" << std::endl;
		serializeFieldCs(f, i);
		f << "		}" << std::endl;
		break;
	}
}

void deserializeFieldCs(std::ofstream& f, Field field)
{
	switch (field.special)
	{
	case FS_NONE:
		if (field.type)
		{
			f << "		" << field.name << " = " << field.type_name << ".Deserialize(reader);" << std::endl;
			break;
		}
		if (field.type_name == "string")
		{
			f << "		{" << std::endl;
			f << "			ushort size = reader.ReadUInt16();" << std::endl;
			f << "			byte[] bytes = reader.ReadBytes(size);" << std::endl;
			f << "			" << field.name << " = System.Text.Encoding.UTF8.GetString(bytes);" << std::endl;
			f << "		}" << std::endl;
			break;
		}
		if (field.type_name == "vec2")
		{
			f << "		" << field.name << " = new Vector2(reader.ReadSingle(), reader.ReadSingle());" << std::endl;
			break;
		}
		if (field.type_name == "vec3")
		{
			f << "		" << field.name << " = new Vector3(reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle());" << std::endl;
			break;
		}
		if (field.type_name == "vec4")
		{
			f << "		" << field.name << " = new Vector4(reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle());" << std::endl;
			break;
		}
		f << "		" << field.name << " = (" << translateCs(field.type_name) << ")reader.Read" << reader_translations.at(field.type_name) << "();" << std::endl;
		break;
	case FS_POINTER:
		f << "		if (reader.ReadBoolean())" << std::endl;
		f << "		{" << std::endl;
		{
			Field value = field;
			value.special = FS_NONE;
			deserializeFieldCs(f, value);
		}
		f << "		}" << std::endl;
		break;
	case FS_VECTOR:
		f << "	{" << std::endl;
		f << "		ushort size = reader.ReadUInt16();" << std::endl;
		f << "		for (int i = 0; i < size; ++i)" << std::endl;
		f << "		{" << std::endl;
		if (field.type)
		{
			f << "			" << field.type_name << " element = new " << field.type_name << "();" << std::endl;
		}
		else
		{
			f << "			" << field.type_name << " element;" << std::endl;
		}
		Field element = field;
		field.name = "element";
		field.special = FS_NONE;
		deserializeFieldCs(f, element);
		f << "			" << field.name << ".Add(element);" << std::endl;
		f << "		}" << std::endl;
		f << "	}" << std::endl;
		break;
	}
}

void CsGenerator::generate(const std::filesystem::path& folder, const std::map<std::string, Structure>& types, const Protocol& protocol) const
{
	auto destination_path = folder;

	for (auto type : types)
	{
		std::ofstream f(destination_path / (type.first + ".cs"));

		f << "using System.IO;" << std::endl;
		
		if (type.second.application_dependencies.size())
		{
			f << "using UnityEngine;" << std::endl;
		}

		f << std::endl;

		f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

		f << "public struct " << type.first << std::endl;
		f << "{" << std::endl;

		for (auto field : type.second.fields)
		{
			switch (field.special)
			{
			case FS_NONE:
				f << "	public " << translateCs(field.type_name) << " " << field.name << ";" << std::endl;
				break;
			case FS_POINTER:
				f << "	public " << translateCs(field.type_name) << "? " << field.name << ";" << std::endl;
				break;
			case FS_VECTOR:
				f << "	public List<" << translateCs(field.type_name) << "> " << field.name << ";" << std::endl;
				break;
			}
		}
		if (type.second.fields.size())
			f << std::endl;

		f << "	public void Serialize(BinaryWriter writer)" << std::endl;
		f << "	{" << std::endl;

		for (auto field : type.second.fields)
		{
			serializeFieldCs(f, field);
		}

		f << "	}" << std::endl << std::endl;

		f << "	public static " << type.first << " Deserialize(BinaryReader reader)" << std::endl;
		f << "	{" << std::endl;
		f << "		" << type.first << " _ret = new " << type.first << "();" << std::endl;
		for (auto field : type.second.fields)
		{
			field.name = "_ret." + field.name;
			deserializeFieldCs(f, field);
		}
		f << "		return _ret;" << std::endl;
		f << "	}" << std::endl;

		f << "};" << std::endl;

		f.close();
	}

	{
		std::ofstream f(destination_path / "Link.cs");

		f << "using System;" << std::endl;
		f << "using System.Collections.Concurrent;" << std::endl;
		f << "using System.IO;" << std::endl;
		f << "using System.Net;" << std::endl;
		f << "using System.Net.Sockets;" << std::endl << std::endl;

		f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

		f << "public class Link" << std::endl;
		f << "{" << std::endl;

		f << "	public " << protocol.name << "Handler handler;" << std::endl << std::endl;

		f << "	public ConcurrentQueue<Action> message_queue = new ConcurrentQueue<Action>();" << std::endl << std::endl;

		f << "	public void Poll()" << std::endl;
		f << "	{" << std::endl;
		f << "		while (message_queue.TryDequeue(out Action action)) action();" << std::endl;
		f << "	}" << std::endl << std::endl;

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
				f << "			" << type.first << " message = " << type.first << ".Deserialize(reader);" << std::endl;
				f << "			message_queue.Enqueue(() => handler." << type.first << "Handler(endpoint, message));" << std::endl;
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
