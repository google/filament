# Writing Tests

There are multiple flavors of node-addon-api test builds that cover different
build flags defined in `napi.h`:

1. c++ exceptions enabled,
2. c++ exceptions disabled,
3. c++ exceptions disabled, and `NODE_ADDON_API_ENABLE_MAYBE` defined.

Functions in node-addon-api that call into JavaScript can have different
declared return types to reflect build flavor settings. For example,
`Napi::Object::Set` returns `bool` when `NODE_ADDON_API_ENABLE_MAYBE`
is not defined, and `Napi::Maybe<bool>` when `NODE_ADDON_API_ENABLE_MAYBE`
is defined. In source code, return type variants are defined as
`Napi::MaybeOrValue<>` to prevent the duplication of most of the code base.

To properly test these build flavors, all values returned by a function defined
to return `Napi::MaybeOrValue<>` should be tested by using one of the following
test helpers to handle possible JavaScript exceptions.

There are three test helper functions to conveniently convert
`Napi::MaybeOrValue<>` values to raw values.

## MaybeUnwrap

```cpp
template <typename T>
T MaybeUnwrap(MaybeOrValue<T> maybe);
```

Converts `MaybeOrValue<T>` to `T` by checking that `MaybeOrValue` is NOT an
empty `Maybe`.

Returns the original value if `NODE_ADDON_API_ENABLE_MAYBE` is not defined.

Example:

```cpp
Object obj = info[0].As<Object>();
// we are sure the parameters should not throw
Value value = MaybeUnwrap(obj->Get("foobar"));
```

## MaybeUnwrapOr

```cpp
template <typename T>
T MaybeUnwrapOr(MaybeOrValue<T> maybe, const T& default_value = T());
```

Converts `MaybeOrValue<T>` to `T` by getting the value that wrapped by the
`Maybe` or return the `default_value` if the `Maybe` is empty.

Returns the original value if `NODE_ADDON_API_ENABLE_MAYBE` is not defined.

Example:

```cpp
Value CallWithArgs(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  // We don't care if the operation is throwing or not, just return it back to node-addon-api
  return MaybeUnwrapOr(
      func.Call(std::initializer_list<napi_value>{info[1], info[2], info[3]}));
}
```

## MaybeUnwrapTo

```cpp
template <typename T>
bool MaybeUnwrapTo(MaybeOrValue<T> maybe, T* out);
```

Converts `MaybeOrValue<T>` to `T` by getting the value that wrapped by the
e`Maybe` or return `false` if the Maybe is empty

Copies the `value` to `out` when `NODE_ADDON_API_ENABLE_MAYBE` is not defined

Example:

```cpp
Object opts = info[0].As<Object>();
bool hasProperty = false;
// The check may throw, but we are going to suppress that.
if (MaybeUnwrapTo(opts.Has("blocking"), &hasProperty)) {
  isBlocking = hasProperty &&
                MaybeUnwrap(MaybeUnwrap(opts.Get("blocking")).ToBoolean());
} else {
  env.GetAndClearPendingException();
}
```
