using System;

[AttributeUsage(AttributeTargets.Class | AttributeTargets.Property | AttributeTargets.Enum)]
public class DirectionAttribute : Attribute
{
    public DirectionAttribute(MessageDirection direction)
    {
        Direction = direction;
    }

    public MessageDirection Direction { get; }
}