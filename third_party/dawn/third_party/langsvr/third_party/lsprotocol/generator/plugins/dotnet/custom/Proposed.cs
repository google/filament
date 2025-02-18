using System;

[AttributeUsage(AttributeTargets.Class | AttributeTargets.Property | AttributeTargets.Enum)]
public class ProposedAttribute : Attribute
{
    public ProposedAttribute()
    {
        Version = null;
    }

    public ProposedAttribute(string version)
    {
        Version = version;
    }

    public string? Version { get; }
}