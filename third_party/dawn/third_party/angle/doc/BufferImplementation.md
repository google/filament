# Introduction

Since Direct3D 9 only supports buffers that either contain vertex or index data,
and OpenGL buffers can contain both, ANGLE waits till a draw call is issued to
determine which resources to create/update. The generic implementation 'streams'
the data into global vertex and index buffers. This streaming buffer
implementation works in all circumstances, but does not offer optimal
performance. When buffer data isn't updated, there's no reason to copy the data
again. For these cases a 'static' buffer implementation is used.

The OpenGL ES 2.0 glBufferData() function allows to specify a usage hint
parameter (GL\_STREAM\_DRAW, GL\_DYNAMIC\_DRAW or GL\_STATIC\_DRAW). Both
GL\_STREAM\_DRAW and GL\_DYNAMIC\_DRAW use the streaming buffer implementation.
With the GL\_STATIC\_DRAW hint, ANGLE will attempt to use the static buffer
implementation. If you update the buffer data after it has already been used in
a draw call, it falls back to the streaming buffer implementation, because
updating static ones would involve creating new ones, which is slower than
updating streaming ones (more on this later).

Because some applications use GL\_STREAM\_DRAW or GL\_DYNAMIC\_DRAW even when
the data is not or very infrequently updated, ANGLE also has a heuristic to
promote buffers to use the static implementation.

# Streaming buffers

The streaming buffers implementation uses one Context-global vertex buffer
(VertexDataManager::mStreamingBuffer) and two index buffers
(IndexDataManager::mStreamingBufferShort and
IndexDataManager::mStreamingBufferInt). The streaming behavior is achieved by
writing new data behind previously written data (i.e. without overwriting old
data). Direct3D 9 allows to efficiently update vertex and index buffers when
you're not reading or overwriting anything (it won't stall waiting for the GPU
finish using it).

When the end of these streaming buffers is reached, they are 'recycled' by
discarding their content. D3D9 will still keep a copy of the data that's in use,
so this recycling efficiently renames the driver level buffers. ANGLE can then
write new data to the beginning of the vertex or index buffer.

The ArrayVertexBuffer::mWritePosition variable holds the current end position of
the last data that was written. StreamingVertexBuffer::reserveRequiredSpace()
allocates space to write the data, and StreamingVertexBuffer::map() actually
locks the D3D buffer and updates the write position. Similar for index buffers.

# Static buffers

Each GL buffer object can have a corresponding static vertex or index buffer
(Buffer::mVertexBuffer and Buffer::mIndexBuffer). When a GL buffer with static
usage is used in a draw call for the first time, all of its data is converted to
a D3D vertex or index buffer, based on the attribute or index formats
respectively. If a subsequent draw call uses different formats, the static
buffer is invalidated (deleted) and the streaming buffer implementation is used
for this buffer object instead. So for optimal performance it's important to
store only a single format of vertices or indices in a buffer. This is highly
typical, and even when in some cases it falls back to the streaming buffer
implementation the performance isn't bad at all.

The StreamingVertexBuffer and StaticVertexBuffer classes share a common base
class, ArrayVertexBuffer. StaticVertexBuffer also has access to the write
position, but it's used only for the initial conversion of the data. So the
interfaces of both classes are not that different. Static buffers have an exact
size though, and can't be changed afterwards (streaming buffers can grow to
handle draw calls which use more data, and avoid excessive recycling).
StaticVertexBuffer has a lookupAttribute() method to retrieve the location of a
certain attribute (this is also used to verify that the formats haven't changed,
which would result in invalidating the static buffer). The descriptions of all
the attribute formats a static buffer contains are stored in the
StaticVertexBuffer::mCache vector.

StaticIndexBuffer also caches information about what's stored in them, namely
the minimum and maximum value for certain ranges of indices. This information is
required by the Direct3D 9 draw calls, and is also used to know the range of
vertices that need to be copied to the streaming vertex buffer in case it needs
to be used (e.g. it is not uncommon to have a buffer with static vertex position
data and a buffer with streaming texture coordinate data for skinning).

# Constant attributes

Aside from using array buffers to feed attribute data to the vertex shader,
OpenGL also supports attributes which remain constant for all vertices used in a
draw call. Direct3D 9 doesn't have a similar concept, at least not explicitly.

Constant attributes are implemented using separate (static) vertex buffers,
and uses a stride of 0 to ensure that every vertex retrieves the same data.
Using a stride of 0 is not possible with streaming buffers because on some
hardware it is incompatible with the D3DUSAGE\_DYNAMIC flag. We found that with
static usage, all hardware tested so far can handle stride 0 fine.

This functionality was implemented in a ConstantVertexBuffer class, and it
integrates nicely with the rest of the static buffer implementation.

# Line loops

Direct3D 9 does not support the 'line loop' primitive type directly. This is
implemented by drawing the 'closing' line segment separately, constructing a
tiny temporary index buffer connecting the last and first vertex.

# Putting it all together

glDrawElements() calls IndexDataManager::prepareIndexData() to retrieve a
Direct3D index buffer containing the necessary data. If an element array is used
(i.e. a buffer object), it has static usage, and it hasn't been invalidated, the
GL buffer's static D3D index buffer will be returned. Else the updated streaming
index buffer is returned, as well as the index offset (write position) where the
new data is located. When prepareIndexData() does find a static index buffer,
but it's empty, it means the GL buffer's data hasn't been converted and stored
in the D3D index buffer yet. So in the convertIndices() call it will convert the
entire buffer. prepareIndexData() will also look up the min/max value of a range
of indices, or computes it when not already in the static buffer or when a
streaming buffer is used.

Similarly, both glDrawElements() and glDrawArrays() both call
VertexDataManager::prepareVertexData() to retrieve a set of Direct3D vertex
buffers and their translated format and offset information. It's implementation
is more complicated than prepareIndexData() because buffer objects can contain
multiple vertex attributes, and multiple buffers can be used as input to the
vertex shader. So first it accumulates how much storage space is required for
each of the buffers in use. For all static non-empty buffers in use, it
determines whether the stored attributes still match what is required by the
draw call, and invalidates them if not (at which point more space is allocated
in the streaming buffer). Converting the GL buffer object's data into D3D
compatible vertex formats is still done by specialized template functions.
