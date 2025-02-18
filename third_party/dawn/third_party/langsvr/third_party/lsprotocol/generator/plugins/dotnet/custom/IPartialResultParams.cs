using System;

/// <summary>
/// Interface to describe parameters for requests that support streaming results.
///
/// See the <see href="https://microsoft.github.io/language-server-protocol/specifications/specification-current/#partialResultParams">Language Server Protocol specification</see> for additional information.
/// </summary>
/// <typeparam name="T">The type to be reported by <see cref="PartialResultToken"/>.</typeparam>
public interface IPartialResultParams
{
    /// <summary>
    /// An optional token that a server can use to report partial results (e.g. streaming) to the client.
    /// </summary>
    public ProgressToken? PartialResultToken { get; set; }
}