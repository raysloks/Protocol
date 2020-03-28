// WARNING : Auto-generated file, changes made will disappear when re-generated.

public struct Mega
{
	public List<float> health_values;
	public Upper? target;
	public List<Test> peeps;
	public int32_t hoho;
	public int64_t muh_dick;
	public List<Upper> testero;

	public void serialize(BinaryWriter writer)
	{
		{
			ushort size = this.health_values.Count;
			writer.Write(size);
			foreach (var i in this.health_values)
				writer.Write(i);
		}
		if (target.HasValue)
		{
			writer.Write(true);
			target.serialize(writer);
		}
		else
		{
			writer.Write(false);
		}
		{
			ushort size = this.peeps.Count;
			writer.Write(size);
			foreach (var i in this.peeps)
				i.serialize(writer);
		}
		writer.Write(hoho);
		writer.Write(muh_dick);
		{
			ushort size = this.testero.Count;
			writer.Write(size);
			foreach (var i in this.testero)
				i.serialize(writer);
		}
	}

	public void deserialize(BinaryReader reader)
	{
	{
		ushort size = reader.ReadUInt16();
		this.health_values.Clear();
		for (int i = 0; i < size; ++i)
		{
			this.health_values.Add(reader.ReadSingle());
		}
	}
		if (reader.ReadBoolean())
		{
			target = new Upper();
			target.deserialize(reader);
		}
	{
		ushort size = reader.ReadUInt16();
		this.peeps.Clear();
		for (int i = 0; i < size; ++i)
		{
			Test element = new Test();
			element.deserialize(reader);
			this.peeps.Add(element);
		}
	}
		hoho = reader.ReadInt32();
		muh_dick = reader.ReadInt64();
	{
		ushort size = reader.ReadUInt16();
		this.testero.Clear();
		for (int i = 0; i < size; ++i)
		{
			Upper element = new Upper();
			element.deserialize(reader);
			this.testero.Add(element);
		}
	}
	}
};
