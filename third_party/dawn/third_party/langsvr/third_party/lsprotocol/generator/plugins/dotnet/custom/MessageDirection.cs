using System.Runtime.Serialization;

public enum MessageDirection
{
    [EnumMember(Value = "serverToClient")] ServerToClient,
    [EnumMember(Value = "clientToServer")] ClientToServer,
    [EnumMember(Value = "both")] Both,
}