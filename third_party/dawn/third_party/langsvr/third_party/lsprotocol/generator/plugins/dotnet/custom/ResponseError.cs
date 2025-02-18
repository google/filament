using Newtonsoft.Json;
using System.Runtime.Serialization;

[DataContract]
public class ResponseError
{
    [JsonConstructor]
    public ResponseError(
        int code,
        string message,
        LSPObject? data = null
    )
    {
        Code = code;
        Message = message;
        Data = data;
    }

    [DataMember(Name = "code")]
    int Code { get; }

    [DataMember(Name = "message")]
    string Message { get; }

    [DataMember(Name = "data")]
    LSPObject? Data { get; }
}