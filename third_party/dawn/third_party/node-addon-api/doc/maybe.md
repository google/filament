# Maybe (template)

Class `Napi::Maybe<T>` represents a value that may be empty: every `Maybe` is
either `Just` and contains a value, or `Nothing`, and does not. `Maybe` types
are very common in node-addon-api code, as they represent that the function may
throw a JavaScript exception and cause the program to be unable to evaluate any
JavaScript code until the exception has been handled.

Typically, the value wrapped in `Napi::Maybe<T>` is [`Napi::Value`] and its
subclasses.

## Methods

### IsNothing

```cpp
template <typename T>
bool Napi::Maybe::IsNothing() const;
```

Returns `true` if the `Maybe` is `Nothing` and does not contain a value, and
`false` otherwise.

### IsJust

```cpp
template <typename T>
bool Napi::Maybe::IsJust() const;
```

Returns `true` if the `Maybe` is `Just` and contains a value, and `false`
otherwise.

### Check

```cpp
template <typename T>
void Napi::Maybe::Check() const;
```

Short-hand for `Maybe::Unwrap()`, which doesn't return a value. Could be used
where the actual value of the Maybe is not needed like `Object::Set`.
If this Maybe is nothing (empty), node-addon-api will crash the
process.

### Unwrap

```cpp
template <typename T>
T Napi::Maybe::Unwrap() const;
```

Return the value of type `T` contained in the Maybe. If this Maybe is
nothing (empty), node-addon-api will crash the process.

### UnwrapOr

```cpp
template <typename T>
T Napi::Maybe::UnwrapOr(const T& default_value) const;
```

Return the value of type T contained in the Maybe, or use a default
value if this Maybe is nothing (empty).

### UnwrapTo

```cpp
template <typename T>
bool Napi::Maybe::UnwrapTo(T* result) const;
```

Converts this Maybe to a value of type `T` in the `out`. If this Maybe is
nothing (empty), `false` is returned and `out` is left untouched.

[`Napi::Value`]: ./value.md
