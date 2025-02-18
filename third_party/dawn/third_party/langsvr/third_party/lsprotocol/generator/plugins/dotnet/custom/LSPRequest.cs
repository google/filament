using System;

[AttributeUsage(AttributeTargets.Class)]
public class LSPRequestAttribute : Attribute
{
    public LSPRequestAttribute(string method, Type response)
    {
        Method = method;
        Response = response;
    }

    public LSPRequestAttribute(string method, Type response, Type partialResponse)
    {
        Method = method;
        Response = response;
        PartialResponse = partialResponse;
    }

    public string Method { get; }
    public Type Response { get; }
    public Type? PartialResponse { get; }
}