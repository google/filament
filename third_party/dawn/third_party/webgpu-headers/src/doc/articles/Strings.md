# Strings {#Strings}

Strings are represented in UTF-8, using the \ref WGPUStringView struct:

> \copydoc WGPUStringView

## Nullable Input String {#NullableInputString}

An input string where the null value may be treated separately from the empty string (e.g. to indicate the absence of a string, or to run some non-trivial defaulting algorithm).

## Non-Null Input String {#NonNullInputString}

This input string is non-nullable.
If the null value is passed, it is treated as the empty string.

## Output String {#OutputString}

This output string is always explicitly sized and never the null value. There is no null terminator inside the string; there may or may not be one after the end. If empty, the data pointer may or may not be null.

To format explicitly-sized strings with `printf`, use `%.*s` (`%s` with a "precision" argument `.*` specifying the max number of bytes to read from the pointer).

### Localizable Human-Readable Message String {#LocalizableHumanReadableMessageString}

This is a @ref OutputString which is human-readable and implementation-dependent, and may be locale-dependent.
It should not be parsed or otherwise treated as machine-readable.
