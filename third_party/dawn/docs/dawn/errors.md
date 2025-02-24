# Dawn Errors

Dawn produces errors for several reasons. The most common is validation errors, indicating that a
given descriptor, configuration, state, or action is not valid according to the WebGPU spec. Errors
can also be produced during exceptional circumstances such as the system running out of GPU memory
or the device being lost.

The messages attached to these errors will frequently be one of the primary tools developers use to
debug problems their applications, so it is important that the messages Dawn returns are useful.

Following the guidelines in document will help ensure that Dawn's errors are clear, informative, and
consistent.

## Returning Errors

Since errors are expected to be an exceptional case, it's important that code that produces an error
doesn't adversely impact the performance of the error-free path. The best way to ensure that is to
make sure that all errors are returned from within an `if` statement that uses the `DAWN_UNLIKELY()`
macro to indicate that the expression is not expected to evaluate to true. For example:

```C++
if (DAWN_UNLIKELY(offset > buffer.size)) {
  return DAWN_VALIDATION_ERROR("Offset (%u) is larger than the size (%u) of %s."
    offset, buffer.size, buffer);
}
```

To simplify producing validation errors, it's strongly suggested that the `DAWN_INVALID_IF()` macro
is used, which will wrap the expression in the `DAWN_UNLIKELY()` macro for you:

```C++
// This is equivalent to the previous example.
DAWN_INVALID_IF(offset > buffer.size, "Offset (%u) is larger than the size (%u) of %s."
    offset, buffer.size, buffer);
```

// TODO: Cover `MaybeError`, `ResultOrError<T>`, `DAWN_TRY(_ASSIGN)`, `DAWN_TRY_CONTEXT`, etc...

## Error message formatting

Errors returned from `DAWN_INVALID_IF()` or `DAWN_VALIDATION_ERROR()` should follow these guidelines:

**Write error messages as complete sentences. (First word capitalized, ends with a period, etc.)**
 * Example: `Command encoding has already finished.`
 * Instead of: `encoder finished`

**Error messages should be in the present tense.**
 * Example: `Buffer is not large enough...`
 * Instead of: `Buffer was not large enough...`

**When possible any values mentioned should be immediately followed in parentheses by the given value.**
 * Example: `("Array stride (%u) is not...", stride)`
 * Output: `Array stride (16) is not...`

**When possible any object or descriptors should be represented by the object formatted as a string.**
 * Example: `("The %s size (%s) is...", buffer, buffer.size)`
 * Output: `The [Buffer] size (512) is...` or `The [Buffer "Label"] size (512) is...`

**Enum and bitmask values should be formatted as strings rather than integers or hex values.**
 * Example: `("The %s format (%s) is...", texture, texture.format)`
 * Output: `The [Texture "Label"] format (TextureFormat::RGBA8Unorm) is...`

**When possible state both the given value and the expected value or limit.**
 * Example: `("Offset (%u) is larger than the size (%u) of %s.", offset, buffer.size, buffer)`
 * Output: `Offset (256) is larger than the size (144) of [Buffer "Label"].`

**State errors in terms of what failed, rather than how to satisfy the rule.**
 * Example: `Binding size (3) is less than the minimum binding size (32).`
 * Instead of: `Binding size (3) must not be less than the minimum binding size (32).`

**Don't repeat information given in context.**
 * See next section for details

## Error Context

When calling functions that perform validation consider if calling `DAWN_TRY_CONTEXT()` rather than
`DAWN_TRY()` is appropriate. Context messages, when provided, will be appended to any validation
errors as a type of human readable "callstack". An error with context messages appears will be
formatted as:

```
<Primary error message.>
 - While <context message lvl 2>
 - While <context message lvl 1>
 - While <context message lvl 0>
```

For example, if a validation error occurs while validating the creation of a BindGroup, the message
may be:

```
Binding size (256) is larger than the size (80) of [Buffer "View Matrix"].
 - While validating entries[1] as a Buffer
 - While validating [BindGroupDescriptor "Frame Bind Group"] against [BindGroupLayout]
 - While calling CreateBindGroup
```

// TODO: Guidelines about when to include context

## Context message formatting

Context messages should follow these guidelines:

**Begin with the action being taken, starting with a lower case. `- While ` will be appended by Dawn.**
 * Example: `("validating primitive state")`
 * Output: `- While validating primitive state`

**When looping through arrays, indicate the array name and index.**
 * Example: `("validating buffers[%u]", i)`
 * Output: `- While validating buffers[2]`

**Indicate which descriptors or objects are being examined in as high-level a context as possible.**
 * Example: `("validating % against %", descriptor, descriptor->layout)`
 * Output: `- While validating [BindGroupDescriptor "Label"] against [BindGroupLayout]`

**When possible, indicate the function call being made as the top-level context, as well as the parameters passed.**
 * Example: `("calling %s.CreatePipelineLayout(%s).", this, descriptor)`
 * Output: `- While calling [Device].CreatePipelineLayout([PipelineLayoutDescriptor]).`
