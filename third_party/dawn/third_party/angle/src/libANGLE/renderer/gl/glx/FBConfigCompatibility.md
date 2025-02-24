GLX Framebuffer Compatibility investigation
===========================================

In GLX and EGL, contexts are created with respect to a config that
describes the type of surfaces they will be used to render to.
Likewise surfaces are created with respect to a config and for
a context to be able to render to a surface, both their configs
must be compatible. Compatibility is losely described in both
the GLX and EGL specs but the following is clear:
 * In GLX the config's color buffer must have the same type, including
RGBA vs. ColorIndex and the buffers must have the same depth, if they
exist.
 * In EGL the config's color buffer must have the same type and the
buffers must have the same depth (not clear if it is only if they exist)

Obviously the EGLconfig we will expose will have a one-to-one
correspondance with GLXFBConfigs.

Our EGL implementation uses a single OpenGL context to back all
the EGLcontexts created by the application. Since our GL context
and GLXContext are the same object but in two APIs, we will make
the confusion and call the GLX context our backing context.

The problem we have is that the the GLX context is created before
the application can choose what type of context it wants to use,
that means we have to expose EGLconfigs whose respective GLXFBConfigs
are compatible with the GLXFBConfig of our GLX context; we also need
to choose the GLXFBConfig of our GLX context so that it matches the
most common needs of application.

Choice of the GLX context GLXFBConfig
-------------------------------------

We decided that our GLX context's configuration must satisfy the following:
 * Have a RGBA8 color buffer and D24S8 depth-stencil buffer which is what
the vast majority of applications use.
 * It must render in direct colors, i.e. not in a color indexed format.
 * It must be double-buffered (see later)
 * It must support rendering to all the types of GLX surfaces so that we can
use it for all types of EGL surfaces
 * It must have an associated visual ID so that we can use it with X, it seems
like this would be strongly tied to it having the ```WINDOW_BIT``` set.
 * We would like a conformant context.

Study of compatible GLXFBConfigs
--------------------------------

When using the condition of compatibility defined in the GLX spec and filtering
out the non-conformant GLXFBConfig we got the following list (see function
```print_visual_attribs_short``` in [glxinfo's source code](http://cgit.freedesktop.org/mesa/demos/tree/src/xdemos/glxinfo.c)
to understand how to read the table):

```
    visual  x   bf lv rg d st  colorbuffer  sr ax dp st accumbuffer  ms  cav
  id dep cl sp  sz l  ci b ro  r  g  b  a F gb bf th cl  r  g  b  a ns b eat  Result
----------------------------------------------------------------------------
0x02e 24 tc  0  32  0 r  . .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None Fail
0x0e4 32 tc  0  32  0 r  . .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None BadMatch
0x02c 24 tc  0  32  0 r  y .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None Pass
0x0e2 32 tc  0  32  0 r  y .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None BadMatch
0x089 24 dc  0  32  0 r  . .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None Fail
0x087 24 dc  0  32  0 r  y .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None Pass
0x026 24 tc  0  32  0 r  . .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None Fail
0x0dc 32 tc  0  32  0 r  . .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None BadMatch
0x024 24 tc  0  32  0 r  y .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None Pass
0x0da 32 tc  0  32  0 r  y .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None BadMatch
0x081 24 dc  0  32  0 r  . .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None Fail
0x07f 24 dc  0  32  0 r  y .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None Pass
```

The last column shows the result of trying to render on a window using the config,
with a GLX context using config 0x024. The first thing we see is that BadMatch is
thrown by the X server when creating the subwindow for rendering. This was because
we didn't set the border pixel of the subwindow *shake fist at X11* (see this [StackOverflow question](http://stackoverflow.com/questions/3645632/how-to-create-a-window-with-a-bit-depth-of-32)).
The result updated with this fix give:

```
    visual  x   bf lv rg d st  colorbuffer  sr ax dp st accumbuffer  ms  cav
  id dep cl sp  sz l  ci b ro  r  g  b  a F gb bf th cl  r  g  b  a ns b eat
----------------------------------------------------------------------------
0x02e 24 tc  0  32  0 r  . .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None Fail
0x0e4 32 tc  0  32  0 r  . .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None Fail
0x02c 24 tc  0  32  0 r  y .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None Pass
0x0e2 32 tc  0  32  0 r  y .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None Pass
0x089 24 dc  0  32  0 r  . .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None Fail
0x087 24 dc  0  32  0 r  y .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None Pass
0x026 24 tc  0  32  0 r  . .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None Fail
0x0dc 32 tc  0  32  0 r  . .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None Fail
0x024 24 tc  0  32  0 r  y .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None Pass
0x0da 32 tc  0  32  0 r  y .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None Pass
0x081 24 dc  0  32  0 r  . .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None Fail
0x07f 24 dc  0  32  0 r  y .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None Pass
```

From this we see that our rendering test passed if and only if the config was double
buffered like 0x024 which is our GLX context config. The compatible configs are then:

```
    visual  x   bf lv rg d st  colorbuffer  sr ax dp st accumbuffer  ms  cav
  id dep cl sp  sz l  ci b ro  r  g  b  a F gb bf th cl  r  g  b  a ns b eat
----------------------------------------------------------------------------
0x02c 24 tc  0  32  0 r  y .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None
0x0e2 32 tc  0  32  0 r  y .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None
0x087 24 dc  0  32  0 r  y .   8  8  8  8 .  s  4  0  0 16 16 16 16  0 0 None
0x024 24 tc  0  32  0 r  y .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None
0x0da 32 tc  0  32  0 r  y .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None
0x07f 24 dc  0  32  0 r  y .   8  8  8  8 .  s  4 24  8 16 16 16 16  0 0 None
```

We can see two dimensions, with our without a depth-stencil buffer and with TrueColor
or DirectColor. The depth-stencil will be useful to expose to application.

More on double buffering
------------------------
The tests above show that double-buffered contexts are not compatible with single-
buffered surfaces; however other tests show that single-buffered contexts are
compatible with both single and double-buffered surfaces. The problem is that in
that case, we can see some flickering even with double-buffered surfaces. If we
can find a trick to avoid that flicker, then we would be able to expose single
and double-buffered surfaces at the EGL level. Not exposing them isn't too much
of a problem though as the vast majority of application want double-buffering.

AMD and extra buffers
---------------------
As can be seen above, NVIDIA does not expose conformant context with multisampled
buffers or non RGBA16 accumulation buffers. The behavior is different on AMD that
exposes them as conformant, which gives the following list after filtering as
explained above:

```
    visual  x   bf lv rg d st  colorbuffer  sr ax dp st accumbuffer  ms  cav
  id dep cl sp  sz l  ci b ro  r  g  b  a F gb bf th cl  r  g  b  a ns b eat
----------------------------------------------------------------------------
0x023 24 tc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8 16 16 16 16  0 0 None
0x027 24 tc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
0x02b 24 tc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  2 1 None
0x02f 24 tc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  4 1 None
0x03b 24 dc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8 16 16 16 16  0 0 None
0x03f 24 dc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
0x043 24 dc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  2 1 None
0x047 24 dc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  4 1 None
```

ANGLE's context is created using 0x027 and experimentation shows it is only compatible
with 0x03f which is the only other config lacking both an accumulation buffer and a
multisample buffer. The GLX spec seems to hint it should still work ("should have the
same size, if they exist") but it doesn't work in this case. Filtering the configs to
have the same multisample and accumulation buffers gives the following:

```
    visual  x   bf lv rg d st  colorbuffer  sr ax dp st accumbuffer  ms  cav
  id dep cl sp  sz l  ci b ro  r  g  b  a F gb bf th cl  r  g  b  a ns b eat
----------------------------------------------------------------------------
0x027 24 tc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
0x03f 24 dc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
```

Mesa Intel driver
-----------------
In GLX, a criterium for context and surface compatibility is that buffers
should have the same depth, if they exist at all in the surface. This means
that it should be possible to make a context with a D24S8 depth-stencil
buffer to a surface without a depth-stencil buffer. This doesn't work on the
Mesa Intel driver. The list before the workaround was the following, with
0x020 being the fbconfig chosen for the context:

```
    visual  x   bf lv rg d st  colorbuffer  sr ax dp st accumbuffer  ms  cav
  id dep cl sp  sz l  ci b ro  r  g  b  a F gb bf th cl  r  g  b  a ns b eat
----------------------------------------------------------------------------
0x020 24 tc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
0x021 24 dc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
0x08f 32 tc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
0x0d0 24 tc  0  32  0 r  y .   8  8  8  8 .  .  0  0  0  0  0  0  0  0 0 None
0x0e2 24 dc  0  32  0 r  y .   8  8  8  8 .  .  0  0  0  0  0  0  0  0 0 None
0x0e9 24 dc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
```

After the workaround that list becomes the following:

```
    visual  x   bf lv rg d st  colorbuffer  sr ax dp st accumbuffer  ms  cav
  id dep cl sp  sz l  ci b ro  r  g  b  a F gb bf th cl  r  g  b  a ns b eat
----------------------------------------------------------------------------
0x020 24 tc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
0x021 24 dc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
0x08f 32 tc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
0x0e9 24 dc  0  32  0 r  y .   8  8  8  8 .  .  0 24  8  0  0  0  0  0 0 None
```

Future investigation
--------------------
All the non-conformant configs have a multisampled buffer, so it could be interesting
to see if we can use them to expose another EGL extension.

Finally this document is written with respect to a small number of drivers, before
using the GLX EGL implementation in the wild it would be good to test it on other
drivers and hardware.

The drivers tested were:

 - the proprietary NVIDIA driver
 - the proprietary AMD driver
 - the open source Intel (Broadwell) Mesa driver
