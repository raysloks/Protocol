// WARNING : Auto-generated file, changes made will disappear when re-generated.

public struct Upper
{
	public uint32_t id;

	public void serialize(BinaryWriter writer)
	{
		writer.Write(id);
	}

	public void deserialize(BinaryReader reader)
	{
		id = reader.ReadUInt32();
	}
};
