using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;

public class DocumentSelectorConverter : JsonConverter<DocumentSelector>
{
    public override void WriteJson(JsonWriter writer, DocumentSelector? value, JsonSerializer serializer)
    {
        if (value is null)
        {
            writer.WriteNull();
        }
        else
        {
            serializer.Serialize(writer, (DocumentFilter[])value);
        }
    }

    public override DocumentSelector ReadJson(JsonReader reader, Type objectType, DocumentSelector? existingValue, bool hasExistingValue, JsonSerializer serializer)
    {
        if (reader.TokenType == JsonToken.Null)
        {
            return null!;
        }
        var token = JToken.Load(reader);
        if (token.Type == JTokenType.Array)
        {
            var filters = token.ToObject<DocumentFilter[]>(serializer);
            return new DocumentSelector(filters ?? Array.Empty<DocumentFilter>());
        }

        throw new JsonSerializationException("Invalid JSON for DocumentSelector");
    }
}
