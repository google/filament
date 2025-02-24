# Implicit Device Synchronization

Makes most public API methods implicitly lock the device, making them safe to call in a multi-threaded environment.
All commands for the `wgpu::Device` and child objects are safe, except command encoding which doesn't get implicit synchronization.

Note that this extension is quite heavy-handed in terms of synchronization, and should eventually disappear in favor of proper fine-grained and efficient synchronization in Dawn.
Also there are likely still cases where Dawn's reentrancy is not handled properly and could cause deadlocks with this extension.

The initial tracking bug was https://crbug.com/dawn/1662.
