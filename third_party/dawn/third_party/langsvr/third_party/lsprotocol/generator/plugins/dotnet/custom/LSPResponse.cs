using System;

[AttributeUsage(AttributeTargets.Class)]
public class LSPResponseAttribute : Attribute
{
    public LSPResponseAttribute(Type request)
    {
        Request = request;
    }


    public Type Request { get; }
}