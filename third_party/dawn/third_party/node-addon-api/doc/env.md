# Env

The opaque data structure containing the environment in which the request is being run.

The Env object is usually created and passed by the Node.js runtime or node-addon-api infrastructure.

## Methods

### Constructor

```cpp
Napi::Env::Env(napi_env env);
```

- `[in] env`: The `napi_env` environment from which to construct the `Napi::Env` object.

### napi_env

```cpp
operator napi_env() const;
```

Returns the `napi_env` opaque data structure representing the environment.

### Global

```cpp
Napi::Object Napi::Env::Global() const;
```

Returns the `Napi::Object` representing the environment's JavaScript Global Object.

### Undefined

```cpp
Napi::Value Napi::Env::Undefined() const;
```

Returns the `Napi::Value` representing the environment's JavaScript Undefined Object.

### Null

```cpp
Napi::Value Napi::Env::Null() const;
```

Returns the `Napi::Value` representing the environment's JavaScript Null Object.

### IsExceptionPending

```cpp
bool Napi::Env::IsExceptionPending() const;
```

Returns a `bool` indicating if an exception is pending in the environment.

### GetAndClearPendingException

```cpp
Napi::Error Napi::Env::GetAndClearPendingException() const;
```

Returns an `Napi::Error` object representing the environment's pending exception, if any.

### RunScript

```cpp
Napi::Value Napi::Env::RunScript(____ script) const;
```
- `[in] script`: A string containing JavaScript code to execute.

Runs JavaScript code contained in a string and returns its result.

The `script` can be any of the following types:
- [`Napi::String`](string.md)
- `const char *`
- `const std::string &`

### GetInstanceData
```cpp
template <typename T> T* GetInstanceData() const;
```

Returns the instance data that was previously associated with the environment,
or `nullptr` if none was associated.

### SetInstanceData

```cpp
template <typename T> using Finalizer = void (*)(Env, T*);
template <typename T, Finalizer<T> fini = Env::DefaultFini<T>>
void SetInstanceData(T* data) const;
```

- `[template] fini`: A function to call when the instance data is to be deleted.
Accepts a function of the form `void CleanupData(Napi::Env env, T* data)`. If
not given, the default finalizer will be used, which simply uses the `delete`
operator to destroy `T*` when the addon instance is unloaded.
- `[in] data`: A pointer to data that will be associated with the instance of
the addon for the duration of its lifecycle.

Associates a data item stored at `T* data` with the current instance of the
addon. The item will be passed to the function `fini` which gets called when an
instance of the addon is unloaded.

### SetInstanceData

```cpp
template <typename DataType, typename HintType>
using FinalizerWithHint = void (*)(Env, DataType*, HintType*);
template <typename DataType,
          typename HintType,
          FinalizerWithHint<DataType, HintType> fini =
            Env::DefaultFiniWithHint<DataType, HintType>>
void SetInstanceData(DataType* data, HintType* hint) const;
```

- `[template] fini`: A function to call when the instance data is to be deleted.
Accepts a function of the form
`void CleanupData(Napi::Env env, DataType* data, HintType* hint)`. If not given,
the default finalizer will be used, which simply uses the `delete` operator to
destroy `T*` when the addon instance is unloaded.
- `[in] data`: A pointer to data that will be associated with the instance of
the addon for the duration of its lifecycle.
- `[in] hint`: A pointer to data that will be associated with the instance of
the addon for the duration of its lifecycle and will be passed as a hint to
`fini` when the addon instance is unloaded.

Associates a data item stored at `T* data` with the current instance of the
addon. The item will be passed to the function `fini` which gets called when an
instance of the addon is unloaded. This overload accepts an additional hint to
be passed to `fini`.

### GetModuleFileName

```cpp
const char* Napi::Env::GetModuleFileName() const;
```

Returns a A URL containing the absolute path of the location from which the
add-on was loaded. For a file on the local file system it will start with
`file://`. The string is null-terminated and owned by env and must thus not be
modified or freed. It is only valid while the add-on is loaded.

### AddCleanupHook

```cpp
template <typename Hook>
CleanupHook<Hook> AddCleanupHook(Hook hook);
```

- `[in] hook`: A function to call when the environment exits. Accepts a
  function of the form `void ()`.

Registers `hook` as a function to be run once the current Node.js environment
exits. Unlike the underlying C-based Node-API, providing the same `hook`
multiple times **is** allowed. The hooks will be called in reverse order, i.e.
the most recently added one will be called first.

Returns an `Env::CleanupHook` object, which can be used to remove the hook via
its `Remove()` method.

### AddCleanupHook

```cpp
template <typename Hook, typename Arg>
CleanupHook<Hook, Arg> AddCleanupHook(Hook hook, Arg* arg);
```

- `[in] hook`: A function to call when the environment exits. Accepts a
  function of the form `void (Arg* arg)`.
- `[in] arg`: A pointer to data that will be passed as the argument to `hook`.

Registers `hook` as a function to be run with the `arg` parameter once the
current Node.js environment exits. Unlike the underlying C-based Node-API,
providing the same `hook` and `arg` pair multiple times **is** allowed. The
hooks will be called in reverse order, i.e. the most recently added one will be
called first.

Returns an `Env::CleanupHook` object, which can be used to remove the hook via
its `Remove()` method.

# Env::CleanupHook

The `Env::CleanupHook` object allows removal of the hook added via
`Env::AddCleanupHook()`

## Methods

### IsEmpty

```cpp
bool IsEmpty();
```

Returns `true` if the cleanup hook was **not** successfully registered.

### Remove

```cpp
bool Remove(Env env);
```

Unregisters the hook from running once the current Node.js environment exits.

Returns `true` if the hook was successfully removed from the Node.js
environment.
