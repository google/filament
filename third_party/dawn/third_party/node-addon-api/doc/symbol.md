# Symbol

Class `Napi::Symbol` inherits from class [`Napi::Name`][].

## Methods

### Constructor

Instantiates a new `Napi::Symbol` value.

```cpp
Napi::Symbol::Symbol();
```

Returns a new empty `Napi::Symbol`.

### New
```cpp
Napi::Symbol::New(napi_env env, const std::string& description);
Napi::Symbol::New(napi_env env, const char* description);
Napi::Symbol::New(napi_env env, Napi::String description);
Napi::Symbol::New(napi_env env, napi_value description);
```

- `[in] env`: The `napi_env` environment in which to construct the `Napi::Symbol` object.
- `[in] value`: The C++ primitive which represents the description hint for the `Napi::Symbol`.
  `description` may be any of:
  - `std::string&` - UTF8 string description.
  - `const char*` - represents a UTF8 string description.
  - `String` - Node addon API String description.
  - `napi_value` - Node-API `napi_value` description.

If an error occurs, a `Napi::Error` will get thrown. If C++ exceptions are not
being used, callers should check the result of `Napi::Env::IsExceptionPending` before
attempting to use the returned value.

### WellKnown
```cpp
static Napi::Symbol Napi::Symbol::WellKnown(napi_env env, const std::string& name);
```

- `[in] env`: The `napi_env` environment in which to construct the `Napi::Symbol` object.
- `[in] name`: The C++ string representing the `Napi::Symbol` to retrieve.

Returns a `Napi::Symbol` representing a well-known `Symbol` from the
`Symbol` registry.

### For
```cpp
static Napi::Symbol Napi::Symbol::For(napi_env env, const std::string& description);
static Napi::Symbol Napi::Symbol::For(napi_env env, const char* description);
static Napi::Symbol Napi::Symbol::For(napi_env env, String description);
static Napi::Symbol Napi::Symbol::For(napi_env env, napi_value description);
```

- `[in] env`: The `napi_env` environment in which to construct the `Napi::Symbol` object.
- `[in] description`: The C++ string representing the `Napi::Symbol` in the global registry to retrieve.

Searches in the global registry for existing symbol with the given name. If the symbol already exist it will be returned, otherwise a new symbol will be created in the registry. It's equivalent to Symbol.for() called from JavaScript.

[`Napi::Name`]: ./name.md