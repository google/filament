# `langsvr`, a C++ 17 implementation of the Language Server Protocol

`langsvr` is an open-source C++ 17 implementation of the [Language Server Protocol](https://microsoft.github.io/language-server-protocol/)

This project is in-development.

Complete:
* C++ structures for all LSP message types ([Requests](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#requestMessage), [Responses](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#responseMessage) and [Notifications](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#dollarRequests))
* Serialization and deserialization of these types.
* Non-string failure responses of requests (`ErrorData` types)

Work remaining:
* A whole lot more unit tests around serialization / deserialization.
* Optimization work.
* Examples.

## Contributing

Please see [CONTRIBUTING](/CONTRIBUTING).

## License

BSD 3-Clause License, please see [LICENSE](/LICENSE).

## Disclaimer

This is not an officially supported Google product.
