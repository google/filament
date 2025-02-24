# Vulkan Timeline Semaphores

[Vulkan Timeline
Semaphores](https://www.khronos.org/blog/vulkan-timeline-semaphores) are a
synchronization primitive accessible both from the device and the host. A
timeline semaphore represents a monotonically increasing 64-bit unsigned
value. Whereas binary Vulkan semaphores are waited on just to become signaled,
timeline semaphores are waited on to reach a specific value. Once a timeline
semaphore reaches a certain value, it is considered signaled for every value
less than or equal to that
value. [`vkWaitSemaphores`](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkWaitSemaphores.html)
is used to wait for semaphores on the host. It can operate in one of two modes:
"wait for all" and "wait for any".

In SwiftShader, Vulkan Timeline Semaphores are implemented as an unsigned 64-bit
integer protected by a mutex with changes signaled by a condition
variable. Waiting for all timeline semaphores in a set is implemented by simply
waiting for each of the semaphores in turn. Waiting for any semaphore in a set
is a bit more complex.

## Wait for any semaphore

A "wait for any" of a set of semaphores is represented by a
`TimelineSemaphore::WaitForAny` object. Additionally, `TimelineSemaphore`
contains an internal list of all `WaitForAny` objects that wait for it, as well
as for which values they wait. When signaled, the timeline semaphore looks
through this list and, in turn, signals any `WaitForAny` objects that are
waiting for a value less than or equal to the timeline semaphore's new value.

A `WaitForAny` object is created from a `VkSemaphoreWaitInfo`. During
construction, it checks the value of each timeline semaphore provided against
the value for which it is waiting. If it has not yet been reached, the wait
object registers itself with the timeline semaphore. If it _has_ been reached,
the wait object is immediately signaled and no further timeline semaphores are
checked.

Once a `WaitForAny` object is signaled, it remains signaled. There is no way to
change what semaphores or values to wait for after construction. Any subsequent
calls to `wait()` will return `VK_SUCCESS` immediately.

When a `WaitForAny` object is destroyed, it unregisters itself from every
`TimelineSemaphore` it was waiting for. It is expected that the number of
concurrent waits are few, and that the wait objects are short-lived, so there
should not be a build-up of wait objects in any timeline semaphore.
