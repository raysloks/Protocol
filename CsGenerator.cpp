#include "CsGenerator.h"

#include <fstream>

const std::map<std::string, std::string> writer_translations = { {"float", "float"}, {"double", "double"},
	{"int8", "sbyte"}, {"int16", "short"}, {"int32", "int"}, {"int64", "long"},
	{"uint8", "byte"}, {"uint16", "ushort"}, {"uint32", "uint"}, {"uint64", "ulong"} };

const std::map<std::string, std::string> reader_translations = { {"float", "Single"}, {"double", "Double"},
	{"int8", "SByte"}, {"int16", "Int16"}, {"int32", "Int32"}, {"int64", "Int64"},
	{"uint8", "Byte"}, {"uint16", "UInt16"}, {"uint32", "UInt32"}, {"uint64", "UInt64"} };

const std::map<std::string, std::string> basic_translations = { {"float", "float"}, {"double", "double"},
	{"int8", "sbyte"}, {"int16", "short"}, {"int32", "int"}, {"int64", "long"},
	{"uint8", "byte"}, {"uint16", "ushort"}, {"uint32", "uint"}, {"uint64", "ulong"},
	{"string", "string"},
	{"vec2", "Vector2"}, {"vec3", "Vector3"}, {"vec4", "Vector4"},
	{"uuid", "Guid"} };

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
		if (field.type_name == "uuid")
		{
			f << "		writer.Write(" << field.name << ".ToByteArray());" << std::endl;
			break;
		}
		if (writer_translations.find(field.type_name) != writer_translations.end())
			f << "		writer.Write((" << writer_translations.at(field.type_name) << ")" << field.name << ");" << std::endl;
		else
			f << "		writer.Write(MemoryMarshal.AsBytes<" << translateCs(field.type_name) << ">(new[] { " << field.name << " }));" << std::endl;
		break;
	case FS_POINTER:
		if (field.type && field.type->child_type_names.size() > 1)
		{
			f << "		if (" << field.name << " != null)" << std::endl;
			f << "		{" << std::endl;
			Field value = field;
			value.special = FS_NONE;
			serializeFieldCs(f, value);
			f << "		}" << std::endl;
			f << "		else" << std::endl;
			f << "		{" << std::endl;
			f << "			writer.Write((byte)0xff);" << std::endl;
			f << "		}" << std::endl;
		}
		else
		{
			if (field.shouldBeNullable())
			{
				f << "		if (" << field.name << ".HasValue)" << std::endl;
				f << "		{" << std::endl;
				f << "			writer.Write(true);" << std::endl;
				Field value = field;
				value.special = FS_NONE;
				value.name += ".Value";
				serializeFieldCs(f, value);
				f << "		}" << std::endl;
				f << "		else" << std::endl;
				f << "		{" << std::endl;
				f << "			writer.Write(false);" << std::endl;
				f << "		}" << std::endl;
			}
			else
			{
				f << "		if (" << field.name << " != null)" << std::endl;
				f << "		{" << std::endl;
				f << "			writer.Write(true);" << std::endl;
				Field value = field;
				value.special = FS_NONE;
				serializeFieldCs(f, value);
				f << "		}" << std::endl;
				f << "		else" << std::endl;
				f << "		{" << std::endl;
				f << "			writer.Write(false);" << std::endl;
				f << "		}" << std::endl;
			}
		}
		break;
	case FS_VECTOR:
		f << "		{" << std::endl;
		f << "			ushort size = (ushort)this." << field.name << ".Count;" << std::endl;
		f << "			writer.Write(size);" << std::endl;
		Field i = field;
		i.name = "i";
		i.special = FS_NONE;
		if (i.superFlat())
		{
			f << "			writer.Write(MemoryMarshal.AsBytes<" << translateCs(field.type_name) << ">(" << field.name << ".ToArray()));" << std::endl;
		}
		else
		{
			f << "			foreach (var i in this." << field.name << ")" << std::endl;
			f << "			{" << std::endl;
			serializeFieldCs(f, i);
			f << "			}" << std::endl;
		}
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
		if (field.type_name == "uuid")
		{
			f << "		" << field.name << " = new Guid(reader.ReadBytes(16));" << std::endl;
			break;
		}
		if (reader_translations.find(field.type_name) != reader_translations.end())
			f << "		" << field.name << " = (" << translateCs(field.type_name) << ")reader.Read" << reader_translations.at(field.type_name) << "();" << std::endl;
		else
			f << "		" << field.name << " = MemoryMarshal.Read<" << field.type_name << ">(reader.ReadBytes(Marshal.SizeOf(typeof(" << field.type_name << "))));" << std::endl;
		break;
	case FS_POINTER:
		if (field.type && field.type->child_type_names.size() > 1)
		{
			Field value = field;
			value.special = FS_NONE;
			deserializeFieldCs(f, value);
		}
		else
		{
			f << "		if (reader.ReadBoolean())" << std::endl;
			f << "		{" << std::endl;
			Field value = field;
			value.special = FS_NONE;
			deserializeFieldCs(f, value);
			f << "		}" << std::endl;
		}
		break;
	case FS_VECTOR:
		f << "		{" << std::endl;
		f << "			ushort size = reader.ReadUInt16();" << std::endl;
		Field element = field;
		element.name = "element";
		element.special = FS_NONE;
		if (element.superFlat() && false)
		{
			f << "			" << field.name << " = new List<" << translateCs(field.type_name) << ">(MemoryMarshal.Cast<byte, " << translateCs(field.type_name) << ">(reader.ReadBytes(Marshal.SizeOf(typeof(" << translateCs(field.type_name) << ")) * size)).ToArray());" << std::endl;
		}
		else
		{
			f << "			" << field.name << " = new List<" << translateCs(field.type_name) << ">();" << std::endl;
			f << "			for (int i = 0; i < size; ++i)" << std::endl;
			f << "			{" << std::endl;
			if (field.type)
			{
				f << "				" << translateCs(field.type_name) << " element = new " << field.type_name << "();" << std::endl;
			}
			else
			{
				f << "				" << translateCs(field.type_name) << " element;" << std::endl;
			}
			deserializeFieldCs(f, element);
			f << "				" << field.name << ".Add(element);" << std::endl;
			f << "			}" << std::endl;
		}
		f << "		}" << std::endl;
		break;
	}
}

void CsGenerator::generate(const std::map<std::string, Structure>& types, const Protocol& protocol) const
{
	auto destination_path = folder;

	for (auto type : types)
	{
		std::ofstream f(destination_path / (type.first + ".cs"));

		f << "using System;" << std::endl;
		f << "using System.Collections.Generic;" << std::endl;
		f << "using System.IO;" << std::endl;
		f << "using System.Runtime.InteropServices;" << std::endl;
		
		//if (type.second.application_dependencies.size())
		{
			f << "using UnityEngine;" << std::endl;
		}

		f << std::endl;

		f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

		if (type.second.child_type_names.size() > 1 || type.second.parent_name.size() > 0)
		{
			if (type.second.parent_name.size() > 0)
			{
				f << "public class " << type.first << " : " << type.second.parent_name << std::endl;
			}
			else
			{
				f << "public class " << type.first << std::endl;
			}
		}
		else
		{
			f << "public struct " << type.first << std::endl;
		}

		f << "{" << std::endl;

		for (auto field : type.second.fields)
		{
			switch (field.special)
			{
			case FS_NONE:
				f << "	public " << translateCs(field.type_name) << " " << field.name << ";" << std::endl;
				break;
			case FS_POINTER:
				if (field.shouldBeNullable())
					f << "	public " << translateCs(field.type_name) << "? " << field.name << ";" << std::endl;
				else
					f << "	public " << translateCs(field.type_name) << " " << field.name << ";" << std::endl;
				break;
			case FS_VECTOR:
				f << "	public List<" << translateCs(field.type_name) << "> " << field.name << ";" << std::endl;
				break;
			}
		}
		if (type.second.fields.size() || type.second.parent_fields.size())
			f << std::endl;

		if (type.second.parent_name.size() > 0)
		{
			f << "	public override void Serialize(BinaryWriter writer)" << std::endl;
		}
		else
		{
			if (type.second.child_type_names.size() > 1)
			{
				f << "	public virtual void Serialize(BinaryWriter writer)" << std::endl;
			}
			else
			{
				f << "	public readonly void Serialize(BinaryWriter writer)" << std::endl;
			}
		}
		f << "	{" << std::endl;

		if (type.second.child_type_index != 0xff)
		{
			f << "		writer.Write((byte)" << type.second.child_type_index << ");" << std::endl;
		}
		for (auto field : type.second.parent_fields)
		{
			serializeFieldCs(f, field);
		}
		for (auto field : type.second.fields)
		{
			serializeFieldCs(f, field);
		}

		f << "	}" << std::endl << std::endl;

		if (type.second.parent_name.size() > 0)
		{
			f << "	public static new " << type.first << " Deserialize(BinaryReader reader)" << std::endl;
		}
		else
		{
			f << "	public static " << type.first << " Deserialize(BinaryReader reader)" << std::endl;
		}
		f << "	{" << std::endl;
		if (type.second.child_type_names.size() > 1)
		{
			f << "		switch (reader.ReadByte())" << std::endl;
			f << "		{" << std::endl;
			for (auto& child_type_name : type.second.child_type_names)
			{
				f << "			case " << types.at(child_type_name).child_type_index << ":" << std::endl;
				if (child_type_name == type.first)
				{
					f << "				" << type.first << " _ret = new();" << std::endl;
					for (auto field : type.second.parent_fields)
					{
						field.name = "_ret." + field.name;
						deserializeFieldCs(f, field);
					}
					for (auto field : type.second.fields)
					{
						field.name = "_ret." + field.name;
						deserializeFieldCs(f, field);
					}
					f << "				return _ret;" << std::endl;
				}
				else
				{
					f << "				return " << child_type_name << ".Deserialize(reader);" << std::endl;
				}
			}
			f << "			default:" << std::endl;
			f << "				return null;" << std::endl;
			f << "		}" << std::endl;
		}
		else
		{
			f << "		" << type.first << " _ret = new();" << std::endl;
			for (auto field : type.second.parent_fields)
			{
				field.name = "_ret." + field.name;
				deserializeFieldCs(f, field);
			}
			for (auto field : type.second.fields)
			{
				field.name = "_ret." + field.name;
				deserializeFieldCs(f, field);
			}
			f << "		return _ret;" << std::endl;
		}
		f << "	}" << std::endl;

		f << "};" << std::endl;

		f.close();
	}

	bool can_connect = down;
	bool can_accept = up;

	{
		std::ofstream f(destination_path / "Link.cs");

		f << "using System;" << std::endl;
		f << "using System.Collections.Concurrent;" << std::endl;
		f << "using System.Collections.Generic;" << std::endl;
		f << "using System.Diagnostics;" << std::endl;
		f << "using System.IO;" << std::endl;
		f << "using System.Net;" << std::endl;
		f << "using System.Net.Sockets;" << std::endl << std::endl;

		f << "// WARNING : Auto-generated file, changes made will disappear when re-generated." << std::endl << std::endl;

		f << "public class Link" << std::endl;
		f << "{" << std::endl;

		f << "	public " << protocol.handler << " handler;" << std::endl << std::endl;

		f << "	public ConcurrentQueue<Action> message_queue = new ConcurrentQueue<Action>();" << std::endl << std::endl;

		f << "	public const int MaxSmallPacketSize = " << 508 << ";" << std::endl;
		f << "	public const int BigPacketChunkSize = MaxSmallPacketSize - " << 4 << ";" << std::endl;
		f << "	public const int MaxBigPacketChunkCount = " << 65536 << ";" << std::endl;
		f << "	public const int MaxBigPacketSize = BigPacketChunkSize * MaxBigPacketChunkCount;" << std::endl;

		f << std::endl;

		f << "	public class BigPacketReceiver" << std::endl;
		f << "	{" << std::endl;
		f << "		public int chunksReceived;" << std::endl;
		f << "		public int lastChunkIndex;" << std::endl;
		f << "		public byte[] buffer = new byte[MaxBigPacketSize];" << std::endl;
		f << "		public long timeout;" << std::endl;
		f << std::endl;
		f << "		public void InsertChunk(int chunkIndex, BinaryReader reader)" << std::endl;
		f << "		{" << std::endl;
		f << "			++chunksReceived;" << std::endl;
		f << "			int bytesRead = reader.Read(buffer, chunkIndex * BigPacketChunkSize, BigPacketChunkSize);" << std::endl;
		f << "			if (bytesRead != BigPacketChunkSize || chunkIndex == MaxBigPacketChunkCount - 1)" << std::endl;
		f << "				lastChunkIndex = chunkIndex;" << std::endl;
		f << "		}" << std::endl;
		f << std::endl;
		f << "		public bool Done => chunksReceived == lastChunkIndex + 1;" << std::endl;
		f << std::endl;
		f << "		public void Reset()" << std::endl;
		f << "		{" << std::endl;
		f << "			chunksReceived = 0;" << std::endl;
		f << "			lastChunkIndex = -2;" << std::endl;
		f << "			timeout = Stopwatch.GetTimestamp() * 1_000_000_000 / Stopwatch.Frequency + 20_000_000_000;" << std::endl;
		f << "		}" << std::endl;
		f << "	}" << std::endl;

		f << std::endl;

		f << "	public Stack<BigPacketReceiver> receiverPool = new();" << std::endl;
		f << "	public Dictionary<int, BigPacketReceiver> allocatedReceivers = new();" << std::endl;

		f << std::endl;

		f << "	public void Poll()" << std::endl;
		f << "	{" << std::endl;
		f << "		while (message_queue.TryDequeue(out Action action)) action();" << std::endl;
		f << "	}" << std::endl << std::endl;

		f << "	public void Open(IPEndPoint endpoint)" << std::endl;
		f << "	{" << std::endl;
		f << "		client = new UdpClient(endpoint);" << std::endl;
		f << "		client.Client.ReceiveBufferSize = " << 0x2000000 << ";" << std::endl;
		f << "	}" << std::endl << std::endl;

		f << "	public void Receive()" << std::endl;
		f << "	{" << std::endl;
		f << "		client.ReceiveAsync().ContinueWith(task =>" << std::endl;
		f << "		{" << std::endl;
		f << "			Receive();" << std::endl;
		f << "			Dispatch(task.Result.Buffer, task.Result.RemoteEndPoint);" << std::endl;
		f << "		});" << std::endl;
		f << "	}" << std::endl << std::endl;

		if (can_connect)
		{
			f << "	public IPEndPoint endpoint;" << std::endl << std::endl;

			f << "	public void Connect(IPEndPoint endpoint)" << std::endl;
			f << "	{" << std::endl;
			f << "		MemoryStream stream = new MemoryStream();" << std::endl;
			f << "		BinaryWriter writer = new BinaryWriter(stream);" << std::endl;
			f << "		writer.Write((byte)0);" << std::endl;
			f << "		writer.Write(crc);" << std::endl;
			f << "		writer.Write((byte)0);" << std::endl;
			f << "		byte[] bytes = stream.ToArray();" << std::endl;
			f << "		client.SendAsync(bytes, bytes.Length, endpoint);" << std::endl;
			f << "	}" << std::endl << std::endl;
		}

		if (can_accept)
		{
			uint8_t message_index = 1;
			for (auto& type : types)
			{
				if (up && type.second.down || down && type.second.up)
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
				}
				if (type.second.down || type.second.up)
					++message_index;
			}
		}

		if (can_connect)
		{
			uint8_t message_index = 1;
			for (auto& type : types)
			{
				if (up && type.second.down || down && type.second.up)
				{
					f << "	public void Send(in " << type.first << " message)" << std::endl;
					f << "	{" << std::endl;
					f << "		if (endpoint == null)" << std::endl; // perhaps there is a better solution?
					f << "			return;" << std::endl;
					f << "		MemoryStream stream = new MemoryStream();" << std::endl;
					f << "		BinaryWriter writer = new BinaryWriter(stream);" << std::endl;
					f << "		writer.Write((byte)" << std::to_string(message_index) << ");" << std::endl;
					f << "		message.Serialize(writer);" << std::endl;
					f << "		byte[] bytes = stream.ToArray();" << std::endl;
					f << "		client.SendAsync(bytes, bytes.Length, endpoint);" << std::endl;
					f << "	}" << std::endl << std::endl;
				}
				if (up && type.second.used_up || down && type.second.used_down)
				{
					if (type.second.child_type_names.size() > 1)
					{
						f << "void Dispatch(in " << type.first << " message)" << std::endl;
						f << "{" << std::endl;
						f << "	switch (message.GetChildTypeIndex())" << std::endl;
						f << "	{" << std::endl;
						for (auto& child_type_name : type.second.child_type_names)
						{
							f << "		case " << types.at(child_type_name).child_type_index << ":" << std::endl;
							f << "		{" << std::endl;
							f << "			handler->" << child_type_name << "Handler(endpoint, *(const " << child_type_name << "*)message);" << std::endl;
							f << "			break;" << std::endl;
							f << "		}" << std::endl;
						}
						f << "		default:" << std::endl;
						f << "			break;" << std::endl;
						f << "	}" << std::endl;
						f << "}" << std::endl << std::endl;
					}
				}
				if (type.second.down || type.second.up)
					++message_index;
			}
		}

		{
			f << "	public void Dispatch(byte[] bytes, IPEndPoint endpoint)" << std::endl;
			f << "	{" << std::endl;
			f << "		BinaryReader reader = new BinaryReader(new MemoryStream(bytes));" << std::endl;
			f << "		switch (reader.ReadByte())" << std::endl;
			f << "		{" << std::endl;

			{
				f << "			case 0:" << std::endl;
				f << "			{" << std::endl;
				f << "				uint remote_crc = reader.ReadUInt32();" << std::endl;
				f << "				if (remote_crc != crc)" << std::endl;
				f << "					break;" << std::endl;
				f << "				switch (reader.ReadByte())" << std::endl;
				f << "				{" << std::endl;
				f << "					case 0:" << std::endl;
				f << "					{" << std::endl;
				if (can_accept)
				{
					f << "						MemoryStream stream = new MemoryStream();" << std::endl;
					f << "						BinaryWriter writer = new BinaryWriter(stream);" << std::endl;
					f << "						writer.Write((byte)0);" << std::endl;
					f << "						writer.Write(crc);" << std::endl;
					f << "						writer.Write((byte)1);" << std::endl;
					f << "						byte[] out_bytes = stream.ToArray();" << std::endl;
					f << "						client.SendAsync(out_bytes, out_bytes.Length, endpoint);" << std::endl;
				}
				f << "						break;" << std::endl;
				f << "					}" << std::endl;
				f << "					case 1:" << std::endl;
				f << "					{" << std::endl;
				if (can_connect)
				{
					f << "						this.endpoint = endpoint;" << std::endl;
					f << "						message_queue.Enqueue(() => handler.ConnectHandler(endpoint));" << std::endl;
				}
				f << "						break;" << std::endl;
				f << "					}" << std::endl;
				f << "					case 2:" << std::endl;
				f << "					{" << std::endl;
				f << "						byte code = reader.ReadByte();" << std::endl;
				f << "						message_queue.Enqueue(() => handler.DisconnectHandler(endpoint, code));" << std::endl;
				f << "						break;" << std::endl;
				f << "					}" << std::endl;
				f << "					default:" << std::endl;
				f << "						break;" << std::endl;
				f << "				}" << std::endl;
				f << "				break;" << std::endl;
				f << "			}" << std::endl;
			}

			{
				uint8_t message_index = 1;
				for (auto& type : types)
				{
					if (down && type.second.down || up && type.second.up)
					{
						f << "		case " << std::to_string(message_index) << ":" << std::endl;
						f << "		{" << std::endl;
						f << "			" << type.first << " message = " << type.first << ".Deserialize(reader);" << std::endl;
						f << "			message_queue.Enqueue(() => handler." << type.first << "Handler(endpoint, message));" << std::endl;
						f << "			break;" << std::endl;
						f << "		}" << std::endl;
					}
					if (type.second.down || type.second.up)
						++message_index;
				}
			}

			{
				f << "		case 255:" << std::endl;
				f << "		{" << std::endl;
				f << "			byte messageId = reader.ReadByte();" << std::endl;
				f << "			ushort chunkIndex = reader.ReadUInt16();" << std::endl;
				f << "			BigPacketReceiver filledReceiver = null;" << std::endl;
				f << "			lock (allocatedReceivers)" << std::endl;
				f << "			{" << std::endl;
				f << "				BigPacketReceiver receiver;" << std::endl;
				f << "				if (allocatedReceivers.ContainsKey(messageId))" << std::endl;
				f << "					receiver = allocatedReceivers[messageId];" << std::endl;
				f << "				else" << std::endl;
				f << "				{" << std::endl;
				f << "					if (receiverPool.Count > 0)" << std::endl;
				f << "						receiver = receiverPool.Pop();" << std::endl;
				f << "					else" << std::endl;
				f << "						receiver = new BigPacketReceiver();" << std::endl;
				f << "					receiver.Reset();" << std::endl;
				f << "					allocatedReceivers[messageId] = receiver;" << std::endl;
				f << "				}" << std::endl;
				f << "				receiver.InsertChunk(chunkIndex, reader);" << std::endl;
				f << "				if (receiver.Done)" << std::endl;
				f << "				{" << std::endl;
				f << "					filledReceiver = receiver;" << std::endl;
				f << "					allocatedReceivers.Remove(messageId);" << std::endl;
				f << "				}" << std::endl;
				f << "			}" << std::endl;
				f << "			if (filledReceiver != null)" << std::endl;
				f << "			{" << std::endl;
				f << "				Dispatch(filledReceiver.buffer, endpoint);" << std::endl;
				f << "				lock (allocatedReceivers)" << std::endl;
				f << "				{" << std::endl;
				f << "					receiverPool.Push(filledReceiver);" << std::endl;
				f << "				}" << std::endl;
				f << "			}" << std::endl;
				f << "			break;" << std::endl;
				f << "		}" << std::endl;
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
