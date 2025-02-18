public interface INotification<TParams> : IMessage
{
    string Method { get; }

    TParams? Params { get; }
}