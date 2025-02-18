using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;

public class CustomStringConverter<T> : JsonConverter<T> where T : class
{
    public override T? ReadJson(JsonReader reader, Type objectType, T? existingValue, bool hasExistingValue, JsonSerializer serializer)
    {
        if (reader.TokenType == JsonToken.String && reader.Value is string str)
        {
            return Activator.CreateInstance(typeof(T), str) as T;
        }
        else if (reader.TokenType == JsonToken.Null)
        {
            return null;
        }

        throw new JsonSerializationException($"Unexpected token type '{reader.TokenType}' while deserializing '{objectType.Name}'.");
    }

    public override void WriteJson(JsonWriter writer, T? value, JsonSerializer serializer)
    {
        if (value is null)
        {
            writer.WriteNull();
        }
        else if (value is Uri u)
        {
            writer.WriteValue(u.AbsoluteUri);
        }
        else if (value is T t)
        {
            writer.WriteValue(t.ToString());
        }
        else
        {
            throw new ArgumentException($"{nameof(value)} must be of type {nameof(T)}.");
        }
    }
}
