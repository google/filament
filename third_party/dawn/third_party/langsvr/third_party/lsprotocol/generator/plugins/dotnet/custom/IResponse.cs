public interface IResponse<TResponse> : IMessage
{

    OrType<string, int> Id { get; }

    TResponse? Result { get; }

    ResponseError? Error { get; }
}