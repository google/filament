using System;

[AttributeUsage(AttributeTargets.Class | AttributeTargets.Property | AttributeTargets.Enum | AttributeTargets.Interface)]
public class SinceAttribute : Attribute
{
    public SinceAttribute()
    {
        Version = null;
    }

    public SinceAttribute(string version)
    {
        Version = version;
    }

    public string? Version { get; }
}