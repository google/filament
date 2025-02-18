public interface IRequest<TParams> : IMessage
{

    OrType<string, int> Id { get; }

    string Method { get; }

    TParams Params { get; }
}