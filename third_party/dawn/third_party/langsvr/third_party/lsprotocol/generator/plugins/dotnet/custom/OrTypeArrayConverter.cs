using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Immutable;

public class OrTypeArrayConverter<T, U> : JsonConverter<ImmutableArray<OrType<T, U>>>
{
    private OrTypeConverter<T, U> _converter;

    public OrTypeArrayConverter()
    {
        _converter = new OrTypeConverter<T, U>();
    }

    public override ImmutableArray<OrType<T, U>> ReadJson(JsonReader reader, Type objectType, ImmutableArray<OrType<T, U>> existingValue, bool hasExistingValue, JsonSerializer serializer)
    {
        if (reader.TokenType == JsonToken.Null)
        {
            return default(ImmutableArray<OrType<T, U>>);
        }

        JArray array = JArray.Load(reader);
        ImmutableArray<OrType<T, U>>.Builder builder = ImmutableArray.CreateBuilder<OrType<T, U>>();

        for (int i = 0; i < array.Count; i++)
        {
            builder.Add((OrType<T, U>)_converter.ReadJson(array[i].CreateReader(), typeof(OrType<T, U>), null, serializer)!);
        }

        return builder.ToImmutable();
    }

    public override void WriteJson(JsonWriter writer, ImmutableArray<OrType<T, U>> value, JsonSerializer serializer)
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
                _converter.WriteJson(writer, item, serializer);
            }

            writer.WriteEndArray();
        }
    }
}
public class OrTypeArrayConverter<T, U, V> : JsonConverter<ImmutableArray<OrType<T, U, V>>>
{
    private OrTypeConverter<T, U, V> _converter;

    public OrTypeArrayConverter()
    {
        _converter = new OrTypeConverter<T, U, V>();
    }

    public override ImmutableArray<OrType<T, U, V>> ReadJson(JsonReader reader, Type objectType, ImmutableArray<OrType<T, U, V>> existingValue, bool hasExistingValue, JsonSerializer serializer)
    {
        if (reader.TokenType == JsonToken.Null)
        {
            return default(ImmutableArray<OrType<T, U, V>>);
        }

        JArray array = JArray.Load(reader);
        ImmutableArray<OrType<T, U, V>>.Builder builder = ImmutableArray.CreateBuilder<OrType<T, U, V>>();

        for (int i = 0; i < array.Count; i++)
        {
            builder.Add((OrType<T, U, V>)_converter.ReadJson(array[i].CreateReader(), typeof(OrType<T, U, V>), null, serializer)!);
        }

        return builder.ToImmutable();
    }

    public override void WriteJson(JsonWriter writer, ImmutableArray<OrType<T, U, V>> value, JsonSerializer serializer)
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
                _converter.WriteJson(writer, item, serializer);
            }

            writer.WriteEndArray();
        }
    }
}


public class OrTypeArrayConverter<T, U, V, W> : JsonConverter<ImmutableArray<OrType<T, U, V, W>>>
{
    private OrTypeConverter<T, U, V, W> _converter;

    public OrTypeArrayConverter()
    {
        _converter = new OrTypeConverter<T, U, V, W>();
    }

    public override ImmutableArray<OrType<T, U, V, W>> ReadJson(JsonReader reader, Type objectType, ImmutableArray<OrType<T, U, V, W>> existingValue, bool hasExistingValue, JsonSerializer serializer)
    {
        if (reader.TokenType == JsonToken.Null)
        {
            return default(ImmutableArray<OrType<T, U, V, W>>);
        }

        JArray array = JArray.Load(reader);
        ImmutableArray<OrType<T, U, V, W>>.Builder builder = ImmutableArray.CreateBuilder<OrType<T, U, V, W>>();

        for (int i = 0; i < array.Count; i++)
        {
            builder.Add((OrType<T, U, V, W>)_converter.ReadJson(array[i].CreateReader(), typeof(OrType<T, U, V, W>), null, serializer)!);
        }

        return builder.ToImmutable();
    }

    public override void WriteJson(JsonWriter writer, ImmutableArray<OrType<T, U, V, W>> value, JsonSerializer serializer)
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
                _converter.WriteJson(writer, item, serializer);
            }

            writer.WriteEndArray();
        }
    }
}