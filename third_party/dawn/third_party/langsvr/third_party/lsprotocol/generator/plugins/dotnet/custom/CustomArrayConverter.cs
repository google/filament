using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Collections.Immutable;

public class CustomArrayConverter<T> : JsonConverter<ImmutableArray<T>>
{
    public override ImmutableArray<T> ReadJson(JsonReader reader, Type objectType, ImmutableArray<T> existingValue, bool hasExistingValue, JsonSerializer serializer)
    {
        if (reader.TokenType == JsonToken.Null)
        {
            return default(ImmutableArray<T>);
        }

        JArray array = JArray.Load(reader);
        ImmutableArray<T>.Builder builder = ImmutableArray.CreateBuilder<T>();

        for (int i = 0; i < array.Count; i++)
        {
            builder.Add((T)array[i].ToObject(typeof(T))!);
        }

        return builder.ToImmutable();

    }

    public override void WriteJson(JsonWriter writer, ImmutableArray<T> value, JsonSerializer serializer)
    {
        if (value.IsDefault)
        {
            writer.WriteNull();
        }
        else
        {
            writer.WriteStartArray();
            foreach (var item in value)
            {
                serializer.Serialize(writer, item);
            }
            writer.WriteEndArray();
        }
    }
}