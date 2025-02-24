# External Buffer

**Some runtimes other than Node.js have dropped support for external buffers**.
On runtimes other than Node.js, node-api methods may return
`napi_no_external_buffers_allowed` to indicate that external
buffers are not supported. One such runtime is Electron as
described in this issue
[electron/issues/35801](https://github.com/electron/electron/issues/35801).

In order to maintain broadest compatibility with all runtimes,
you may define `NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED` in your addon before
includes for the node-api and node-addon-api headers. Doing so will hide the
functions that create external buffers. This will ensure a compilation error
occurs if you accidentally use one of these methods.

In node-addon-api, the `Napi::Buffer::NewOrCopy` provides a convenient way to
create an external buffer, or allocate a new buffer and copy the data when the
external buffer is not supported.
