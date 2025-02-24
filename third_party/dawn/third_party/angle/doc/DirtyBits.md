# Dirty Bits and State Changes

OpenGL render loops typically involve changing some render states followed by
a draw call. For instance the app might change a few uniforms and invoke
`glDrawElements`:

```
for (const auto &obj : scene) {
    for (const auto &uni : obj.uniforms) {
        glUniform4fv(uni.loc, uni.data);
    }
    glDrawElements(GL_TRIANGLES, obj.eleCount, GL_UNSIGNED_SHORT, obj.eleOffset);
}
```

Another update loop may change Texture and Vertex Array state before the draw:

```
for (const auto &obj : scene) {
    glBindBuffer(GL_ARRAY_BUFFER, obj.arrayBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, obj.bufferOffset, obj.bufferSize, obj.bufferData);
    glVertexAttribPointer(obj.arrayIndex, obj.arraySize, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindTexture(GL_TEXTURE_2D, obj.texture);
    glDrawElements(GL_TRIANGLES, obj.eleCount, GL_UNSIGNED_SHORT, obj.eleOffset);
}
```

Other update loops may change render states like the blending modes, the depth test, or Framebuffer
attachments. In each case ANGLE needs to validate, track, and translate these state changes to the
back-end as efficiently as possible.

## Dirty Bits

Each OpenGL Context state value is stored in [`gl::State`](../src/libANGLE/State.h). For instance
the blending state, depth/stencil state, and current object bindings. Our problem is deciding how to
notify the back-end when app changes front-end state. We decided to bundle changed state into
bitsets. Each 1 bit indicates a specific changed state value. We call these bitsets "*dirty bits*".
See [`gl::State::DirtyBitType`][DirtyBitType].

Each back-end handles state changes in a `syncState` implementation function that takes a dirty
bitset. See examples in the [GL back-end][GLSyncState], [D3D11 back-end][D3D11SyncState] and
[Vulkan back-end][VulkanSyncState].

Container objects such as Vertex Array Objects and Framebuffers also have their own OpenGL front-end
state. [VAOs][VAOState] store vertex arrays and array buffer bindings. [Framebuffers][FBOState]
store attachment state and the active read and draw buffers. These containers also have internal
dirty bits and `syncState` methods. See [`gl::Framebuffer::DirtyBitType`][FBODirtyBits] and
[`rx::FramebufferVk::syncState`][FBOVkSyncState] for example.

Dirty bits allow us to efficiently process groups of state updates. We use fast instrinsic functions
to scan the bitsets for 1 bits. See [`bitset_utils.h`](../src/common/bitset_utils.h) for more
information.

## Cached Validation and State Change Notifications

To optimize validation we cache many checks. See [`gl::StateCache`][StateCache] for examples. We
need to refresh cached values on state changes. For instance, enabling a generic vertex array
changes a cached mask of active vertex arrays. Changes to a texture's images could change a cached
framebuffer's completeness when the texture is bound as an attachment. And if the draw framebuffer
becomes incomplete it changes a cached draw call validation check.

See a below example of a call to `glTexImage2D` that can affect draw call validation:

<!-- Generated from https://bramp.github.io/js-sequence-diagrams/
participant App
participant Context
participant Framebuffer
participant Texture
App->Context: glTexImage2D
Context->Texture: setImage
Texture- ->Framebuffer: onSubjectStateChange
Note over Framebuffer: cache update
Framebuffer- ->Context: onSubjectStateChange
Note over Context: cache update
-->

![State Change Example](https://raw.githubusercontent.com/google/angle/main/doc/img/StateNotificationExample.svg?sanitize=true)

We use the [Observer pattern](https://en.wikipedia.org/wiki/Observer_pattern) to implement cache
invalidation notifications. See [`Observer.h`](../src/libANGLE/Observer.h). In the example the
`Framebuffer` observes `Texture` attachments via [`angle::ObserverBinding`][ObserverBinding].
`Framebuffer` implements [`angle::ObserverInterface::onSubjectStateChange`][FBOStateChange] to
receive a notification to update its completeness cache. The `STORAGE_CHANGED` message triggers a
call to [`gl::Context::onSubjectStateChange`][ContextStateChange] which in turn calls
[`gl::StateCache::updateBasicDrawStatesError`][StateCacheUpdate] to re-validate the draw
framebuffer's completeness. On subsequent draw calls we skip re-validation at minimal cost.

See the below diagram for the dependency relations between Subjects and Observers.

![State Change Notification Flow](https://raw.githubusercontent.com/google/angle/main/doc/img/StateChangeNotificationFlow.svg?sanitize=true)

## Back-end specific Optimizations

See [Fast OpenGL State Transitions][FastStateTransitions] in [Vulkan documents][VulkanREADME] for
additional information for how we implement state change optimization on the Vulkan back-end.

[DirtyBitType]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/State.h#483
[GLSyncState]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/renderer/gl/StateManagerGL.cpp#1576
[D3D11SyncState]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/renderer/d3d/d3d11/StateManager11.cpp#852
[VulkanSyncState]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/renderer/vulkan/ContextVk.cpp#642
[VAOState]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/VertexArray.h#35
[FBOState]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/Framebuffer.h#52
[FBODirtyBits]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/Framebuffer.h#319
[FBOVkSyncState]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/renderer/vulkan/FramebufferVk.cpp#726
[StateCache]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/Context.h#98
[ObserverBinding]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/Observer.h#103
[FBOStateChange]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/Framebuffer.cpp#1811
[ContextStateChange]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/Context.cpp#7981
[StateCacheUpdate]: https://chromium.googlesource.com/angle/angle/+/5f662c0042703344eb0eef6d1c123e902e3aefbf/src/libANGLE/Context.cpp#8190
[FastStateTransitions]: ../src/libANGLE/renderer/vulkan/doc/FastOpenGLStateTransitions.md
[VulkanREADME]: ../src/libANGLE/renderer/vulkan/README.md
