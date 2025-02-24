# TypeTaggable

Class `Napi::TypeTaggable` inherits from class [`Napi::Value`][].

The `Napi::TypeTaggable` class is the base class for [`Napi::Object`][] and
[`Napi::External`][]. It adds type-tagging capabilities to both. It is an
abstract-only base class.

### TypeTag()

```cpp
void Napi::TypeTaggable::TypeTag(const napi_type_tag* type_tag) const;
```

- `[in] type_tag`: The tag with which this object or external is to be marked.

The `Napi::TypeTaggable::TypeTag()` method associates the value of the
`type_tag` pointer with this JavaScript object or external.
`Napi::TypeTaggable::CheckTypeTag()` can then be used to compare the tag that
was attached with one owned by the add-on to ensure that this object or external
has the right type.

### CheckTypeTag()

```cpp
bool Napi::TypeTaggable::CheckTypeTag(const napi_type_tag* type_tag) const;
```

- `[in] type_tag`: The tag with which to compare any tag found on this object or
  external.

The `Napi::TypeTaggable::CheckTypeTag()` method compares the pointer given as
`type_tag` with any that can be found on this JavaScript object or external. If
no tag is found or if a tag is found but it does not match `type_tag`, then the
return value is `false`. If a tag is found and it matches `type_tag`, then the
return value is `true`.

[`Napi::Value`]: ./value.md
[`Napi::Object`]: ./object.md
[`Napi::External`]: ./external.md
