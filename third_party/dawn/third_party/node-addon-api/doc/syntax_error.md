# SyntaxError

The `Napi::SyntaxError` class is a representation of the JavaScript
`SyntaxError` that is thrown when the engine encounters tokens or token order
that does not conform to the syntax of the language when parsing code.

The `Napi::SyntaxError` class inherits its behaviors from the `Napi::Error`
class (for more info see: [`Napi::Error`](error.md)).

For more details about error handling refer to the section titled [Error
handling](error_handling.md).

## Methods

### New

Creates a new instance of a `Napi::SyntaxError` object.

```cpp
Napi::SyntaxError::New(Napi::Env env, const char* message);
```

- `[in] Env`: The environment in which to construct the `Napi::SyntaxError`
  object.
- `[in] message`: Null-terminated string to be used as the message for the
  `Napi::SyntaxError`.

Returns an instance of a `Napi::SyntaxError` object.

### New

Creates a new instance of a `Napi::SyntaxError` object.

```cpp
Napi::SyntaxError::New(Napi::Env env, const std::string& message);
```

- `[in] Env`: The environment in which to construct the `Napi::SyntaxError`
  object.
- `[in] message`: Reference string to be used as the message for the
  `Napi::SyntaxError`.

Returns an instance of a `Napi::SyntaxError` object.

### Constructor

Creates a new empty instance of a `Napi::SyntaxError`.

```cpp
Napi::SyntaxError::SyntaxError();
```

### Constructor

Initializes a `Napi::SyntaxError` instance from an existing Javascript error
object.

```cpp
Napi::SyntaxError::SyntaxError(napi_env env, napi_value value);
```

- `[in] Env`: The environment in which to construct the `Napi::SyntaxError`
  object.
- `[in] value`: The `Napi::Error` reference to wrap.

Returns an instance of a `Napi::SyntaxError` object.
