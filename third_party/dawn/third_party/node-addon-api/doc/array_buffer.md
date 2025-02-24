# ArrayBuffer

Class `Napi::ArrayBuffer` inherits from class [`Napi::Object`][].

The `Napi::ArrayBuffer` class corresponds to the
[JavaScript `ArrayBuffer`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer)
class.

## Methods

### New

Allocates a new `Napi::ArrayBuffer` instance with a given length.

```cpp
static Napi::ArrayBuffer Napi::ArrayBuffer::New(napi_env env, size_t byteLength);
```

- `[in] env`: The environment in which to create the `Napi::ArrayBuffer` instance.
- `[in] byteLength`: The length to be allocated, in bytes.

Returns a new `Napi::ArrayBuffer` instance.

### New

> When `NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED` is defined, this method is not available.
> See [External Buffer][] for more information.

Wraps the provided external data into a new `Napi::ArrayBuffer` instance.

The `Napi::ArrayBuffer` instance does not assume ownership for the data and
expects it to be valid for the lifetime of the instance. Since the
`Napi::ArrayBuffer` is subject to garbage collection this overload is only
suitable for data which is static and never needs to be freed.
This factory method will not provide the caller with an opportunity to free the
data when the `Napi::ArrayBuffer` gets garbage-collected. If you need to free
the data retained by the `Napi::ArrayBuffer` object please use other
variants of the `Napi::ArrayBuffer::New` factory method that accept
`Napi::Finalizer`, which is a function that will be invoked when the
`Napi::ArrayBuffer` object has been destroyed.

```cpp
static Napi::ArrayBuffer Napi::ArrayBuffer::New(napi_env env, void* externalData, size_t byteLength);
```

- `[in] env`: The environment in which to create the `Napi::ArrayBuffer` instance.
- `[in] externalData`: The pointer to the external data to wrap.
- `[in] byteLength`: The length of the `externalData`, in bytes.

Returns a new `Napi::ArrayBuffer` instance.

### New

> When `NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED` is defined, this method is not available.
> See [External Buffer][] for more information.

Wraps the provided external data into a new `Napi::ArrayBuffer` instance.

The `Napi::ArrayBuffer` instance does not assume ownership for the data and
expects it to be valid for the lifetime of the instance. The data can only be
freed once the `finalizeCallback` is invoked to indicate that the
`Napi::ArrayBuffer` has been released.

```cpp
template <typename Finalizer>
static Napi::ArrayBuffer Napi::ArrayBuffer::New(napi_env env,
                       void* externalData,
                       size_t byteLength,
                       Finalizer finalizeCallback);
```

- `[in] env`: The environment in which to create the `Napi::ArrayBuffer` instance.
- `[in] externalData`: The pointer to the external data to wrap.
- `[in] byteLength`: The length of the `externalData`, in bytes.
- `[in] finalizeCallback`: A function to be called when the `Napi::ArrayBuffer` is
  destroyed. It must implement `operator()`, accept an Napi::Env, a `void*` (which is the
  `externalData` pointer), and return `void`.

Returns a new `Napi::ArrayBuffer` instance.

### New

> When `NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED` is defined, this method is not available.
> See [External Buffer][] for more information.

Wraps the provided external data into a new `Napi::ArrayBuffer` instance.

The `Napi::ArrayBuffer` instance does not assume ownership for the data and expects it
to be valid for the lifetime of the instance. The data can only be freed once
the `finalizeCallback` is invoked to indicate that the `Napi::ArrayBuffer` has been
released.

```cpp
template <typename Finalizer, typename Hint>
static Napi::ArrayBuffer Napi::ArrayBuffer::New(napi_env env,
                       void* externalData,
                       size_t byteLength,
                       Finalizer finalizeCallback,
                       Hint* finalizeHint);
```

- `[in] env`: The environment in which to create the `Napi::ArrayBuffer` instance.
- `[in] externalData`: The pointer to the external data to wrap.
- `[in] byteLength`: The length of the `externalData`, in bytes.
- `[in] finalizeCallback`: The function to be called when the `Napi::ArrayBuffer` is
  destroyed. It must implement `operator()`, accept an Napi::Env, a `void*` (which is the
  `externalData` pointer) and `Hint*`, and return `void`.
- `[in] finalizeHint`: The hint to be passed as the second parameter of the
  finalize callback.

Returns a new `Napi::ArrayBuffer` instance.

### Constructor

Initializes an empty instance of the `Napi::ArrayBuffer` class.

```cpp
Napi::ArrayBuffer::ArrayBuffer();
```

### Constructor

Initializes a wrapper instance of an existing `Napi::ArrayBuffer` object.

```cpp
Napi::ArrayBuffer::ArrayBuffer(napi_env env, napi_value value);
```

- `[in] env`: The environment in which to create the `Napi::ArrayBuffer` instance.
- `[in] value`: The `Napi::ArrayBuffer` reference to wrap.

### ByteLength

```cpp
size_t Napi::ArrayBuffer::ByteLength() const;
```

Returns the length of the wrapped data, in bytes.

### Data

```cpp
void* Napi::ArrayBuffer::Data() const;
```

Returns a pointer the wrapped data.

### Detach

```cpp
void Napi::ArrayBuffer::Detach();
```

Invokes the `ArrayBuffer` detach operation on a detachable `ArrayBuffer`.

### IsDetached

```cpp
bool Napi::ArrayBuffer::IsDetached() const;
```

Returns `true` if this `ArrayBuffer` has been detached.

[`Napi::Object`]: ./object.md
[External Buffer]: ./external_buffer.md
