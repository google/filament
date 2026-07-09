# 2D Coordinate Spaces

This page describes Filament's interpretation of 2D coordinate spaces. We address the differences
between graphics APIs and provide explanations for custom logic and client APIs.

## 1. Texture coordinate space

To begin our discussion, we imagine looking at a rectangular image, like a photograph. To be able to
describe to another person where things are on the photograph (say, a **tree**), we need to first
define a consistent coordinate system that both parties understand. A natural choice is to pick one
of the corners as the origin of the coordinate system (that is, corners are easy to identify on a
rectangle, and do not require tools - like a ruler - for positioning). We can further refine this
coordinate system by assigning 2D numbers to the four corners so that we can accurately
describe the positions on the photograph.

So for example, we assign the corners in clockwise order, starting from the top-left corner:
\\((0, 0)\\), \\((1, 0)\\), \\((1, 1)\\), \\((0, 1)\\).

Assuming we are sampling from a texture in a shader, we sample by using a 2D coordinate (UV)
to identify a point on the rectangular space of the texture. We can make a statement such as, "the
**tree** is near the point of \\(((0.65, 0.35)\\))", to communicate the position of a tree in
the picture to another person.

Given the description above, we conclude this section with the following statements
 - Programmers communicate with computers using coordinate systems like the above to draw 2D images
   onto a display.
 - In 3D graphics, similar coordinate systems are used to texturize 3D models with 2D images. The
   2D images are called *textures*.
 - Filament defines its coordinate system using the *top-left* of the image as \\((0, 0)\\) and
   the *bottom-right* is \\((1, 1)\\).

### 1.1. OpenGL (The Bottom-Left API)

OpenGL has its texture coordinate origin \\((0, 0)\\) at the **bottom-left**, and
\\((1, 1)\\) is at the top-right. Note that this differs from Filament's assumed texture
coordinate system.

The GL function for uploading texture is `glTexImage2D`. Even though GL interprets \\((0, 0)\\) as
the bottom-left of a 2D image, `glTexImage2D` uploads the lower-bits as the *bottom* of the image
as opposed to the *top*. (This behavior is opposite of the other backends' texture upload
APIs). The two differences cancel each other, meaning there is no difference between the different
backends in upload and sampling of textures.

We can restate the above as

| |
|---|
| *Filament's internal texture coordinate space defines the **top-left** corner of the image as the origin.* |

### 1.2. Readback of rendertarget

While we've established that the APIs are not any different in the *uploading* and *sampling* of
textures for Filament's backends, there are, unfortunately, cases where special handling of
texture coordinate is necessary.

Reading the result of rendering into CPU memory can be very useful; for example, one might want to
take snapshots of the rendering and output to image formats such as PNG or JPEG. Another use is to
perform image analysis or processing over a rendered image. In Filament, the readback path
corresponds to the `readPixels`. In the OpenGL backend, this is implemented using the `glReadPixels`
function. GL's interpretation of texture coordinate space holds true for `glReadPixels`; this means
that it treats the bottom-left of the image as the origin and the readback output *starts* from the
origin. Hence, the first row of the image in CPU memory is the bottom-most row of the image (when
displayed).

This creates two related inconsistencies:
 - First, (not a real use case), if one were to upload a texture, attach it as an attachment, and
   read the attachment, the GL backend would place the bottom of the image as the first row of
   pixels in CPU memory.
 - Second, the other backends consider the top-left corner as the origin, which means their
   readbacks would place the top of the image as the first row of the image in CPU memory.

To account for these inconsistencies, *the GL backend vertically flips the rows of the readback
image*.

### 1.3. `flipUV` in Materials

In order to work with textures and associated primitives from different sources, Filament provides
a simple, material-level client-set flag to flip the v-direction.

For example, if a 3D modeler
exported a mesh with textures such that the bottom-left corner of the texture is \\(u=0, v=0\\) in
texture space, then this is not consistent with Filament's assumption. Luckily, modern modelers will
only choose either the top-left corner or the bottom-right corner as the origin. To address this
inconsistency, Filament provides `flipUV` in the material definition. Setting this flag to true will
flip the UV coordinate in the vertex attributes so that \\(v' = 1 - v\\), and the texture
coordinate interpretation is then consistent with Filament's internal coordinates.

## 2. Clip-space

The clip-space is the bounded 3D space that a primitive vertex is transformed into at the end of a
vertex shader. Abstractly, the XY-plane of the clip-space is parallel to the display, and its
bounds correspond to the bounds of the window, surface, or buffer that we are rendering into. (This
API-specific bounded space is also known as Normalized Device Coordinates or NDC).
In different graphics API, the bounds and orientation of the NDC is also inconsistent. For all of
the backends, the ranges of the XY-plane \\([-1, 1]\\) and the origin (\\((0, 0)\\)) are the
same. However, the depth range and the Y-axis directions are not the same.

| Graphics API   | Depth Range (Z) | Y-Axis Direction | Frame Buffer Origin |
| -------------- | --------------- | ---------------- | ------------- |
| OpenGL / WebGL | \\([−1, 1]\\)   | Up               | bottom-left |
| Metal          | \\([0, 1]\\)    | Up               | top-left |
| WebGPU         | \\([0, 1]\\)    | Up               | top-left |
| Vulkan         | \\([0, 1]\\)    | Down             | top-left |

(We will defer the discussion of GL's depth range difference to section 2.3.)

### 2.1. Vulkan y-axis correction

The XY ranges for all backends are the same, but Vulkan interprets the Y-Axis direction as
"Down", meaning that \\(y=1\\) is at the bottom of the image. To account for this difference,
Filament flips the projected \\(y\\) coordinate in the vertex shader for Vulkan

```c++
#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip space is Y-down. In OpenGL and Metal, clip space is Y-up.
    position.y = -position.y;
#endif
```

### 2.2 Fix Frame buffer origin difference with `uvToRenderTargetUV`

After the workaround in the above section, we can rewrite the table thus

| Graphics API   | Y-Axis Direction | Frame Buffer Origin |
| -------------- | ---------------- | ------------------- |
| OpenGL / WebGL | Up               | bottom-left |
| Metal          | Up               | top-left |
| WebGPU         | Up               | top-left |
| Vulkan         | Up (corrected)   | top-left |

We can rephrase the Y-Axis Direction column to say that

| |
|---|
| *Filament's internal clip-space defines the Y-axis direction as **UP** (i.e. the physical world up direction pointing to \\(+1\\)).* |

Now consider the "Frame Buffer Origin" column. This column identifies the origin of the rendered
image in the context of sampling by subsequent passes.

Assume
 - We have two passes \\(A\\) and \\(B\\)
 - \\(A\\) renders to a render target that attaches texture \\(T\\)
 - \\(B\\) renders a full-screen quad
 - In \\(B\\), top-left and bottom-right corners of the quad are assigned texturing UV coordinates
   \\((0, 0)\\) and \\((1, 1)\\), respectively.
 - The quad in \\(B\\) samples texture \\(T\\).

For GL, having the *bottom-left* corner as the origin means that rendering a pixel to clip-space
\\(x=-1, y=-1\\) in \\(A\\) will output to pixel at \\(u=0, v=0\\) of \\(T\\). Note that GL
clip-space's y-axis direction is consistent with the texture space v-axis direction. That is, in
pass \\(B\\), sampling from \\(v=0\\) to \\(v=1\\) (moving in the positive direction) maps to going
from \\(y=-1\\) to \\(y=1\\) in clip-space (also moving in positive direction).

This behavior is opposite for all other backends; sampling from \\(v=0\\) to \\(v=1\\) will map to
clip-space movement from \\(y=1\\) to \\(y=-1\\) (going in the negative direction).

To account for this, Filament provides the following shader method that clients can call to
correctly sample from a texture that was the rendertarget of a previous pass.

```c++
highp vec2 uvToRenderTargetUV(const highp vec2 uv) {
#if defined(TARGET_METAL_ENVIRONMENT) || defined(TARGET_VULKAN_ENVIRONMENT) || defined(TARGET_WEBGPU_ENVIRONMENT)
    return vec2(uv.x, 1.0 - uv.y);
#else
    return uv;
#endif
}
```

It's meant to be used like so

```c++
vec2 uv0 = uvToRenderTargetUV(getUV0());
material.baseColor = texture(materialParams_baseColor, uv0);
```

### 2.3. Clip-space depth adjustments for OpenGL

While the clip-space depth (or Z) dimension does not pertain to the 2D coordinate spaces that we have
been discussing, the concept in this section parallels the space adjustment logic in the other
sections. Hence, we have included it here.

Consider again this table

| Graphics API   | Depth Range (Z) |
| -------------- | --------------- |
| OpenGL / WebGL | \\([−1, 1]\\)   |
| Metal          | \\([0, 1]\\)    |
| WebGPU         | \\([0, 1]\\)    |
| Vulkan         | \\([0, 1]\\)    |

For OpenGL, the NDC depth dimension ranges from \\(-1\\) to \\(1\\). Filament implements two
paths to adjust this so that the internal clip-space definition remains consistent:

1. Use the `EXT_clip_control` extension to change the depth range to \\([0, 1]\\).

   ```c++
   #if !defined(__EMSCRIPTEN__)
       if (ext.EXT_clip_control) {
       #if defined(BACKEND_OPENGL_VERSION_GL)
           glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
       #elif defined(GL_EXT_clip_control)
           glClipControlEXT(GL_LOWER_LEFT_EXT, GL_ZERO_TO_ONE_EXT);
       #endif
       }
   #endif
   ```

2. For cases where the extension is not available, the following shader code will convert the
   internal (or noted as virtual in the code) coordinates to GL's expected depth coordinates.

   ```c++
   #if !defined(TARGET_VULKAN_ENVIRONMENT) && !defined(TARGET_METAL_ENVIRONMENT) && !defined(TARGET_WEBGPU_ENVIRONMENT)
       // This is not needed in Vulkan, Metal or WebGPU because clipControl is always (1, 0)
       position.z = position.z * frameUniforms.clipControl.x
                  + position.w * frameUniforms.clipControl.y;
   #endif
   ```

   This code uses a vector in the uniform buffer to scale and translate the resulting depth. This
   vector is written in runtime (so that we'd know whether the extension is present or not).
