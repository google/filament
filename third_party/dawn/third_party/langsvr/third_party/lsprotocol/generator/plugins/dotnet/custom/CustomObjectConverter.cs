using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;


class CustomObjectConverter<T> : JsonConverter<T> where T : Dictionary<string, object?>
{
    public override T ReadJson(JsonReader reader, Type objectType, T? existingValue, bool hasExistingValue, JsonSerializer serializer)
    {
        if (reader.TokenType == JsonToken.Null)
        {
            return default(T)!;
        }

        Dictionary<string, object?>? o = serializer.Deserialize<Dictionary<string, object?>>(reader);
        if (o == null)
        {
            return default(T)!;
        }
        return (T)Activator.CreateInstance(typeof(T), o)! ?? default(T)!;
    }

    public override void WriteJson(JsonWriter writer, T? value, JsonSerializer serializer)
    {
        if (value is null)
        {
            writer.WriteNull();
        }
        else
        {
            writer.WriteStartObject();
            foreach (var kvp in value)
            {
                writer.WritePropertyName(kvp.Key);
                serializer.Serialize(writer, kvp.Value);
            }
            writer.WriteEndObject();
        }
    }
}