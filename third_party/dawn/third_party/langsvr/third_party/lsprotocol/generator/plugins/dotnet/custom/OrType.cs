using System;

public record OrType<T, U> : IOrType
{
    public object? Value { get; }
    public OrType(T t)
    {
        Value = t ?? throw new ArgumentNullException(nameof(t));
    }

    public OrType(U u)
    {
        Value = u ?? throw new ArgumentNullException(nameof(u));
    }

    public static explicit operator U?(OrType<T, U> obj)
    {
        return obj.Value is U x ? x : default;
    }

    public static explicit operator T?(OrType<T, U> obj)
    {
        return obj.Value is T x ? x : default;
    }

    public static explicit operator OrType<T, U>(U obj) => obj is null ? null! : new OrType<T, U>(obj);
    public static explicit operator OrType<T, U>(T obj) => obj is null ? null! : new OrType<T, U>(obj);

    public override string ToString()
    {
        return Value?.ToString()!;
    }
}

public record OrType<T, U, V> : IOrType
{
    public object? Value { get; }

    public OrType(T t)
    {
        Value = t ?? throw new ArgumentNullException(nameof(t));
    }

    public OrType(U u)
    {
        Value = u ?? throw new ArgumentNullException(nameof(u));
    }

    public OrType(V v)
    {
        Value = v ?? throw new ArgumentNullException(nameof(v));
    }

    public static explicit operator U?(OrType<T, U, V> obj)
    {
        return obj.Value is U x ? x : default;
    }

    public static explicit operator T?(OrType<T, U, V> obj)
    {
        return obj.Value is T x ? x : default;
    }

    public static explicit operator V?(OrType<T, U, V> obj)
    {
        return obj.Value is V x ? x : default;
    }

    public static explicit operator OrType<T, U, V>(U obj) => obj is null ? null! : new OrType<T, U, V>(obj);

    public static explicit operator OrType<T, U, V>(T obj) => obj is null ? null! : new OrType<T, U, V>(obj);

    public static explicit operator OrType<T, U, V>(V obj) => obj is null ? null! : new OrType<T, U, V>(obj);

    public override string ToString()
    {
        return Value?.ToString()!;
    }
}


public record OrType<T, U, V, W> : IOrType
{
    public object? Value { get; }

    public OrType(T t)
    {
        Value = t ?? throw new ArgumentNullException(nameof(t));
    }

    public OrType(U u)
    {
        Value = u ?? throw new ArgumentNullException(nameof(u));
    }

    public OrType(V v)
    {
        Value = v ?? throw new ArgumentNullException(nameof(v));
    }

    public OrType(W w)
    {
        Value = w ?? throw new ArgumentNullException(nameof(w));
    }

    public static explicit operator U?(OrType<T, U, V, W> obj)
    {
        return obj.Value is U x ? x : default;
    }

    public static explicit operator T?(OrType<T, U, V, W> obj)
    {
        return obj.Value is T x ? x : default;
    }

    public static explicit operator V?(OrType<T, U, V, W> obj)
    {
        return obj.Value is V x ? x : default;
    }

    public static explicit operator W?(OrType<T, U, V, W> obj)
    {
        return obj.Value is W x ? x : default;
    }

    public static explicit operator OrType<T, U, V, W>(U obj) => obj is null ? null! : new OrType<T, U, V, W>(obj);

    public static explicit operator OrType<T, U, V, W>(T obj) => obj is null ? null! : new OrType<T, U, V, W>(obj);

    public static explicit operator OrType<T, U, V, W>(V obj) => obj is null ? null! : new OrType<T, U, V, W>(obj);

    public static explicit operator OrType<T, U, V, W>(W obj) => obj is null ? null! : new OrType<T, U, V, W>(obj);

    public override string ToString()
    {
        return Value?.ToString()!;
    }
}
