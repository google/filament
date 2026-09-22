//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[cancelAsyncCall](cancel-async-call.md)

# cancelAsyncCall

[main]\
open fun [cancelAsyncCall](cancel-async-call.md)(id: Int): Boolean

Cancel the pending asynchronous call pointed to by `id`, which is retrieved whenever you invoke a non-blocking version of method on an object, such as `Texture::setImageAsync` or `BufferObject::setBufferAsync`. 

Canceling does not suppress the completion callback of the call. The callback always runs exactly once, on the handler the call was given: with `backend::AsyncCallStatus::CANCELED` if the operation never ran, and with `COMPLETED` if it did. So a caller counting outstanding operations always balances out, and whatever the callback owns is still released.

#### Return

Returns true upon successful cancellation. It returns false if the asynchronous operation cannot be canceled because it is currently running, has finished, or has previously been canceled.

#### Parameters

main

| | |
|---|---|
| id | The unique identifier for the asynchronous call to be canceled. |
