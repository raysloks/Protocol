// WARNING : Auto-generated file, changes made will disappear when re-generated.

public struct Test
{
	public float time;
	public uint8_t magic;
	public Upper upper;

	public void serialize(BinaryWriter writer)
	{
		writer.Write(time);
		writer.Write(magic);
		upper.serialize(writer);
	}

	public void deserialize(BinaryReader reader)
	{
		time = reader.ReadSingle();
		magic = reader.ReadByte();
		upper.deserialize(reader);
	}
};
