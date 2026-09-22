//[filament-android](../../../../index.md)/[com.google.android.filament](../../index.md)/[VertexBuffer](../index.md)/[AsyncCallStatus](index.md)

# AsyncCallStatus

enum [AsyncCallStatus](index.md)

Outcome of an asynchronous operation, reported to its completion callback. 

A completion callback that cannot say why it fired is ambiguous: chaining another operation from a callback that fired because the operation was canceled would proceed on a resource that was never populated. The caller cannot reconstruct the answer out of band either, because an operation can be canceled without anyone asking for it (the driver dropping queued work while shutting down).

#### See also

| |
|---|
| AsyncCallback |
| cancelAsyncJob |

## Entries

| | |
|---|---|
| [COMPLETED](-c-o-m-p-l-e-t-e-d/index.md) | [main]<br>[COMPLETED](-c-o-m-p-l-e-t-e-d/index.md)<br>The operation ran to completion. |
| [CANCELED](-c-a-n-c-e-l-e-d/index.md) | [main]<br>[CANCELED](-c-a-n-c-e-l-e-d/index.md)<br>The operation never ran: it was canceled, or dropped because the driver is shutting down. |

## Functions

| Name | Summary |
|---|---|
| [toFilamentNative](to-filament-native.md) | [main]<br>open fun [toFilamentNative](to-filament-native.md)(): Int |
| [valueOf](value-of.md) | [main]<br>open fun [valueOf](value-of.md)(name: [String](https://developer.android.com/reference/kotlin/java/lang/String.html)): [VertexBuffer.AsyncCallStatus](index.md)<br>Returns the enum constant of this type with the specified name. The string must match exactly an identifier used to declare an enum constant in this type. (Extraneous whitespace characters are not permitted.) |
| [values](values.md) | [main]<br>open fun [values](values.md)(): Array&lt;[VertexBuffer.AsyncCallStatus](index.md)&gt;<br>Returns an array containing the constants of this enum type, in the order they're declared. This method may be used to iterate over the constants. |
