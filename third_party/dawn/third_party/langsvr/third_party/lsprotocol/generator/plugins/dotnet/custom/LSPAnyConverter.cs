using Newtonsoft.Json;

public class LSPAnyConverter : JsonConverter
{
    public override bool CanConvert(Type objectType)
    {
        return objectType == typeof(LSPAny);
    }

    public override object? ReadJson(JsonReader reader, Type objectType, object? existingValue, JsonSerializer serializer)
    {
        reader = reader ?? throw new ArgumentNullException(nameof(reader));
        switch (reader.TokenType)
        {
            case JsonToken.Null:
                return null;

            case JsonToken.Integer:
                return new LSPAny(serializer.Deserialize<long>(reader));

            case JsonToken.Float:
                return new LSPAny(serializer.Deserialize<float>(reader));

            case JsonToken.Boolean:
                return new LSPAny(serializer.Deserialize<bool>(reader));

            case JsonToken.String:
                return new LSPAny(serializer.Deserialize<string>(reader));

            case JsonToken.StartArray:
                List<object>? l = serializer.Deserialize<List<object>>(reader);
                if (l == null)
                {
                    return null;
                }
                return new LSPAny(new LSPArray(l));

            case JsonToken.StartObject:
                Dictionary<string, object?>? o = serializer.Deserialize<Dictionary<string, object?>>(reader);
                if (o == null)
                {
                    return null;
                }
                return new LSPAny(new LSPObject(o));
        }

        throw new JsonSerializationException($"Unexpected token type '{reader.TokenType}' while deserializing '{objectType.Name}'.");
    }

    public override void WriteJson(JsonWriter writer, object? value, JsonSerializer serializer)
    {
        if (value is null)
        {
            writer.WriteNull();
        }
        else
        {
            serializer.Serialize(writer, ((LSPAny)value).Value);
        }
    }
}