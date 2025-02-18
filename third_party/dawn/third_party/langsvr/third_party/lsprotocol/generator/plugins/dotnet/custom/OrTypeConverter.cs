using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;

internal class OrTypeConverterHelpers
{
    public static Type[] SortTypesByHeuristic(Type[] types, JToken jToken)
    {
        var typePropertyScores = new Dictionary<Type, int>();

        string[] jTokenPropertyNames = jToken.Children<JProperty>().Select(p => p.Name.ToUpper()).ToArray();

        foreach (Type type in types)
        {
            string[] typePropertyNames = type.GetProperties().Select(p => p.Name.ToUpper()).ToArray();

            int score = jTokenPropertyNames.Count(propertyName => typePropertyNames.Contains(propertyName));
            typePropertyScores[type] = score;
        }

        return types.OrderByDescending(type => typePropertyScores[type]).ToArray();
    }
}

public class OrTypeConverter<T, U> : JsonConverter<OrType<T, U>>
{
    public override OrType<T, U>? ReadJson(JsonReader reader, Type objectType, OrType<T, U>? existingValue, bool hasExistingValue, JsonSerializer serializer)
    {
        reader = reader ?? throw new ArgumentNullException(nameof(reader));

        if (reader.TokenType == JsonToken.Null)
        {
            return null;
        }

        Type[] types = new Type[] { typeof(T), typeof(U) };

        if (reader.TokenType == JsonToken.Integer && (Validators.HasType(types, typeof(long)) || Validators.HasType(types, typeof(int))))
        {
            return ReadIntegerToken(reader, serializer, types);
        }
        if (reader.TokenType == JsonToken.Float && Validators.HasType(types, typeof(float)))
        {
            return ReadFloatToken(reader, serializer, types);
        }
        if (reader.TokenType == JsonToken.Boolean && Validators.HasType(types, typeof(bool)))
        {
            return ReadBooleanToken(reader, serializer, types);
        }
        if (reader.TokenType == JsonToken.String && Validators.HasType(types, typeof(string)))
        {
            return ReadStringToken(reader, serializer, types);
        }

        var token = JToken.Load(reader);
        return OrTypeConverter<T, U>.ReadObjectToken(token, serializer, OrTypeConverterHelpers.SortTypesByHeuristic(types, token));
    }

    private static OrType<T, U> ReadIntegerToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        long integer = serializer.Deserialize<long>(reader);
        if (Validators.InUIntegerRange(integer) && Validators.HasType(types, typeof(long)))
        {
            if (typeof(T) == typeof(long))
            {
                return new OrType<T, U>((T)(object)(long)integer);
            }
            if (typeof(U) == typeof(long))
            {
                return new OrType<T, U>((U)(object)(long)integer);
            }
        }
        if (Validators.InIntegerRange(integer) && Validators.HasType(types, typeof(int)))
        {
            if (typeof(T) == typeof(int))
            {
                return new OrType<T, U>((T)(object)(int)integer);
            }
            if (typeof(U) == typeof(int))
            {
                return new OrType<T, U>((U)(object)(int)integer);
            }
        }
        throw new ArgumentOutOfRangeException($"Integer out-of-range of LSP Signed Integer[{int.MinValue}:{int.MaxValue}] and out-of-range of LSP Unsigned Integer [{long.MinValue}:{long.MaxValue}] => {integer}");
    }

    private static OrType<T, U> ReadFloatToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        float real = serializer.Deserialize<float>(reader);
        if (typeof(T) == typeof(float))
        {
            return new OrType<T, U>((T)(object)real);
        }
        if (typeof(U) == typeof(float))
        {
            return new OrType<T, U>((U)(object)real);
        }
        throw new InvalidOperationException("Invalid token type for float");
    }

    private static OrType<T, U> ReadBooleanToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        bool boolean = serializer.Deserialize<bool>(reader);
        if (typeof(T) == typeof(bool))
        {
            return new OrType<T, U>((T)(object)boolean);
        }
        if (typeof(U) == typeof(bool))
        {
            return new OrType<T, U>((U)(object)boolean);
        }
        throw new InvalidOperationException("Invalid token type for boolean");
    }

    private static OrType<T, U> ReadStringToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        string str = serializer.Deserialize<string>(reader)!;
        if (typeof(T) == typeof(string))
        {
            return new OrType<T, U>((T)(object)str);
        }
        if (typeof(U) == typeof(string))
        {
            return new OrType<T, U>((U)(object)str);
        }
        throw new InvalidOperationException("Invalid token type for string");
    }

    private static OrType<T, U> ReadObjectToken(JToken token, JsonSerializer serializer, Type[] types)
    {
        var exceptions = new List<Exception>();
        foreach (Type type in types)
        {
            try
            {
                object? value = null;
                if (token.Type == JTokenType.Array && type == typeof((long, long)))
                {
                    long[]? o = token.ToObject<long[]>(serializer);
                    if (o != null)
                    {
                        value = (o[0], o[1]);
                    }
                }
                else
                {
                    value = token.ToObject(type, serializer);
                }

                if (value != null)
                {
                    if (value is T t)
                    {
                        return new OrType<T, U>(t);
                    }
                    if (value is U u)
                    {
                        return new OrType<T, U>(u);
                    }
                }
            }
            catch (Exception ex)
            {
                exceptions.Add(ex);
                continue;
            }
        }

        throw new JsonSerializationException("Unable to deserialize object", new AggregateException(exceptions));
    }



    public override void WriteJson(JsonWriter writer, OrType<T, U>? value, JsonSerializer serializer)
    {
        if (value is null)
        {
            writer.WriteNull();
        }
        else if (value?.Value?.GetType() == typeof((long, long)))
        {
            ValueTuple<long, long> o = (ValueTuple<long, long>)(value.Value);
            serializer.Serialize(writer, new long[] { o.Item1, o.Item2 });
        }
        else
        {
            serializer.Serialize(writer, value?.Value);
        }
    }
}

public class OrTypeConverter<T, U, V> : JsonConverter<OrType<T, U, V>>
{
    public override OrType<T, U, V>? ReadJson(JsonReader reader, Type objectType, OrType<T, U, V>? existingValue, bool hasExistingValue, JsonSerializer serializer)
    {
        reader = reader ?? throw new ArgumentNullException(nameof(reader));

        if (reader.TokenType == JsonToken.Null)
        {
            return null;
        }

        Type[] types = new Type[] { typeof(T), typeof(U), typeof(V) };

        if (reader.TokenType == JsonToken.Integer && (Validators.HasType(types, typeof(long)) || Validators.HasType(types, typeof(int))))
        {
            return ReadIntegerToken(reader, serializer, types);
        }
        if (reader.TokenType == JsonToken.Float && Validators.HasType(types, typeof(float)))
        {
            return ReadFloatToken(reader, serializer, types);
        }
        if (reader.TokenType == JsonToken.Boolean && Validators.HasType(types, typeof(bool)))
        {
            return ReadBooleanToken(reader, serializer, types);
        }
        if (reader.TokenType == JsonToken.String && Validators.HasType(types, typeof(string)))
        {
            return ReadStringToken(reader, serializer, types);
        }

        var token = JToken.Load(reader);
        return OrTypeConverter<T, U, V>.ReadObjectToken(token, serializer, OrTypeConverterHelpers.SortTypesByHeuristic(types, token));
    }

    private static OrType<T, U, V> ReadIntegerToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        long integer = serializer.Deserialize<long>(reader);
        if (Validators.InUIntegerRange(integer) && Validators.HasType(types, typeof(long)))
        {
            if (typeof(T) == typeof(long))
            {
                return new OrType<T, U, V>((T)(object)(long)integer);
            }
            if (typeof(U) == typeof(long))
            {
                return new OrType<T, U, V>((U)(object)(long)integer);
            }
            if (typeof(V) == typeof(long))
            {
                return new OrType<T, U, V>((V)(object)(long)integer);
            }
        }
        if (Validators.InIntegerRange(integer) && Validators.HasType(types, typeof(int)))
        {
            if (typeof(T) == typeof(int))
            {
                return new OrType<T, U, V>((T)(object)(int)integer);
            }
            if (typeof(U) == typeof(int))
            {
                return new OrType<T, U, V>((U)(object)(int)integer);
            }
            if (typeof(V) == typeof(int))
            {
                return new OrType<T, U, V>((V)(object)(int)integer);
            }
        }
        throw new ArgumentOutOfRangeException($"Integer out-of-range of LSP Signed Integer[{int.MinValue}:{int.MaxValue}] and out-of-range of LSP Unsigned Integer [{long.MinValue}:{long.MaxValue}] => {integer}");
    }

    private static OrType<T, U, V> ReadFloatToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        float real = serializer.Deserialize<float>(reader);
        if (typeof(T) == typeof(float))
        {
            return new OrType<T, U, V>((T)(object)real);
        }
        if (typeof(U) == typeof(float))
        {
            return new OrType<T, U, V>((U)(object)real);
        }
        if (typeof(V) == typeof(float))
        {
            return new OrType<T, U, V>((V)(object)real);
        }
        throw new InvalidOperationException("Invalid token type for float");
    }

    private static OrType<T, U, V> ReadBooleanToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        bool boolean = serializer.Deserialize<bool>(reader);
        if (typeof(T) == typeof(bool))
        {
            return new OrType<T, U, V>((T)(object)boolean);
        }
        if (typeof(U) == typeof(bool))
        {
            return new OrType<T, U, V>((U)(object)boolean);
        }
        if (typeof(V) == typeof(bool))
        {
            return new OrType<T, U, V>((V)(object)boolean);
        }
        throw new InvalidOperationException("Invalid token type for boolean");
    }

    private static OrType<T, U, V> ReadStringToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        string str = serializer.Deserialize<string>(reader)!;
        if (typeof(T) == typeof(string))
        {
            return new OrType<T, U, V>((T)(object)str);
        }
        if (typeof(U) == typeof(string))
        {
            return new OrType<T, U, V>((U)(object)str);
        }
        if (typeof(V) == typeof(string))
        {
            return new OrType<T, U, V>((V)(object)str);
        }
        throw new InvalidOperationException("Invalid token type for string");
    }

    private static OrType<T, U, V> ReadObjectToken(JToken token, JsonSerializer serializer, Type[] types)
    {
        var exceptions = new List<Exception>();
        foreach (Type type in types)
        {
            try
            {
                object? value = null;
                if (token.Type == JTokenType.Array && type == typeof((long, long)))
                {
                    long[]? o = token.ToObject<long[]>(serializer);
                    if (o != null)
                    {
                        value = (o[0], o[1]);
                    }
                }
                else
                {
                    value = token.ToObject(type, serializer);
                }

                if (value != null)
                {
                    if (value is T t)
                    {
                        return new OrType<T, U, V>(t);
                    }
                    if (value is U u)
                    {
                        return new OrType<T, U, V>(u);
                    }
                    if (value is V v)
                    {
                        return new OrType<T, U, V>(v);
                    }
                }
            }
            catch (Exception ex)
            {
                exceptions.Add(ex);
                continue;
            }
        }

        throw new JsonSerializationException("Unable to deserialize object", new AggregateException(exceptions));
    }

    public override void WriteJson(JsonWriter writer, OrType<T, U, V>? value, JsonSerializer serializer)
    {
        if (value is null)
        {
            writer.WriteNull();
        }
        else if (value?.Value?.GetType() == typeof((long, long)))
        {
            ValueTuple<long, long> o = (ValueTuple<long, long>)(value.Value);
            serializer.Serialize(writer, new long[] { o.Item1, o.Item2 });
        }
        else
        {
            serializer.Serialize(writer, value?.Value);
        }
    }
}

public class OrTypeConverter<T, U, V, W> : JsonConverter<OrType<T, U, V, W>>
{
    public override OrType<T, U, V, W>? ReadJson(JsonReader reader, Type objectType, OrType<T, U, V, W>? existingValue, bool hasExistingValue, JsonSerializer serializer)
    {
        reader = reader ?? throw new ArgumentNullException(nameof(reader));

        if (reader.TokenType == JsonToken.Null)
        {
            return null;
        }

        Type[] types = new Type[] { typeof(T), typeof(U), typeof(V), typeof(W) };

        if (reader.TokenType == JsonToken.Integer && (Validators.HasType(types, typeof(long)) || Validators.HasType(types, typeof(int))))
        {
            return ReadIntegerToken(reader, serializer, types);
        }
        if (reader.TokenType == JsonToken.Float && Validators.HasType(types, typeof(float)))
        {
            return ReadFloatToken(reader, serializer, types);
        }
        if (reader.TokenType == JsonToken.Boolean && Validators.HasType(types, typeof(bool)))
        {
            return ReadBooleanToken(reader, serializer, types);
        }
        if (reader.TokenType == JsonToken.String && Validators.HasType(types, typeof(string)))
        {
            return ReadStringToken(reader, serializer, types);
        }

        var token = JToken.Load(reader);
        return OrTypeConverter<T, U, V, W>.ReadObjectToken(token, serializer, OrTypeConverterHelpers.SortTypesByHeuristic(types, token));
    }

    private static OrType<T, U, V, W> ReadIntegerToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        long integer = serializer.Deserialize<long>(reader);
        if (Validators.InUIntegerRange(integer) && Validators.HasType(types, typeof(long)))
        {
            if (typeof(T) == typeof(long))
            {
                return new OrType<T, U, V, W>((T)(object)(long)integer);
            }
            if (typeof(U) == typeof(long))
            {
                return new OrType<T, U, V, W>((U)(object)(long)integer);
            }
            if (typeof(V) == typeof(long))
            {
                return new OrType<T, U, V, W>((V)(object)(long)integer);
            }
            if (typeof(W) == typeof(long))
            {
                return new OrType<T, U, V, W>((W)(object)(long)integer);
            }
        }
        if (Validators.InIntegerRange(integer) && Validators.HasType(types, typeof(int)))
        {
            if (typeof(T) == typeof(int))
            {
                return new OrType<T, U, V, W>((T)(object)(int)integer);
            }
            if (typeof(U) == typeof(int))
            {
                return new OrType<T, U, V, W>((U)(object)(int)integer);
            }
            if (typeof(V) == typeof(int))
            {
                return new OrType<T, U, V, W>((V)(object)(int)integer);
            }
            if (typeof(W) == typeof(int))
            {
                return new OrType<T, U, V, W>((W)(object)(int)integer);
            }
        }
        throw new ArgumentOutOfRangeException($"Integer out-of-range of LSP Signed Integer[{int.MinValue}:{int.MaxValue}] and out-of-range of LSP Unsigned Integer [{long.MinValue}:{long.MaxValue}] => {integer}");
    }

    private static OrType<T, U, V, W> ReadFloatToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        float real = serializer.Deserialize<float>(reader);
        if (typeof(T) == typeof(float))
        {
            return new OrType<T, U, V, W>((T)(object)real);
        }
        if (typeof(U) == typeof(float))
        {
            return new OrType<T, U, V, W>((U)(object)real);
        }
        if (typeof(V) == typeof(float))
        {
            return new OrType<T, U, V, W>((V)(object)real);
        }
        if (typeof(W) == typeof(float))
        {
            return new OrType<T, U, V, W>((W)(object)real);
        }
        throw new InvalidOperationException("Invalid token type for float");
    }

    private static OrType<T, U, V, W> ReadBooleanToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        bool boolean = serializer.Deserialize<bool>(reader);
        if (typeof(T) == typeof(bool))
        {
            return new OrType<T, U, V, W>((T)(object)boolean);
        }
        if (typeof(U) == typeof(bool))
        {
            return new OrType<T, U, V, W>((U)(object)boolean);
        }
        if (typeof(V) == typeof(bool))
        {
            return new OrType<T, U, V, W>((V)(object)boolean);
        }
        if (typeof(W) == typeof(bool))
        {
            return new OrType<T, U, V, W>((W)(object)boolean);
        }
        throw new InvalidOperationException("Invalid token type for boolean");
    }

    private static OrType<T, U, V, W> ReadStringToken(JsonReader reader, JsonSerializer serializer, Type[] types)
    {
        string str = serializer.Deserialize<string>(reader)!;
        if (typeof(T) == typeof(string))
        {
            return new OrType<T, U, V, W>((T)(object)str);
        }
        if (typeof(U) == typeof(string))
        {
            return new OrType<T, U, V, W>((U)(object)str);
        }
        if (typeof(V) == typeof(string))
        {
            return new OrType<T, U, V, W>((V)(object)str);
        }
        if (typeof(W) == typeof(string))
        {
            return new OrType<T, U, V, W>((W)(object)str);
        }
        throw new InvalidOperationException("Invalid token type for string");
    }

    private static OrType<T, U, V, W> ReadObjectToken(JToken token, JsonSerializer serializer, Type[] types)
    {
        var exceptions = new List<Exception>();
        foreach (Type type in types)
        {
            try
            {
                object? value = null;
                if (token.Type == JTokenType.Array && type == typeof((long, long)))
                {
                    long[]? o = token.ToObject<long[]>(serializer);
                    if (o != null)
                    {
                        value = (o[0], o[1]);
                    }
                }
                else
                {
                    value = token.ToObject(type, serializer);
                }

                if (value != null)
                {
                    if (value is T t)
                    {
                        return new OrType<T, U, V, W>(t);
                    }
                    if (value is U u)
                    {
                        return new OrType<T, U, V, W>(u);
                    }
                    if (value is V v)
                    {
                        return new OrType<T, U, V, W>(v);
                    }
                    if (value is W w)
                    {
                        return new OrType<T, U, V, W>(w);
                    }
                }

            }
            catch (Exception ex)
            {
                exceptions.Add(ex);
                continue;
            }
        }

        throw new JsonSerializationException("Unable to deserialize object", new AggregateException(exceptions));
    }

    public override void WriteJson(JsonWriter writer, OrType<T, U, V, W>? value, JsonSerializer serializer)
    {
        if (value is null)
        {
            writer.WriteNull();
        }
        else if (value?.Value?.GetType() == typeof((long, long)))
        {
            ValueTuple<long, long> o = (ValueTuple<long, long>)(value.Value);
            serializer.Serialize(writer, new long[] { o.Item1, o.Item2 });
        }
        else
        {
            serializer.Serialize(writer, value?.Value);
        }
    }
}