Name

    MESA_set_3dfx_mode

Name Strings

    GLX_MESA_set_3dfx_mode

Contact

    Brian Paul (brian 'at' mesa3d.org)

Status

    Shipping since Mesa 2.6 in February, 1998.

Version

    Last Modified Date:  8 June 2000

Number

    218

Dependencies

    OpenGL 1.0 or later is required.
    GLX 1.0 or later is required.

Overview

    The Mesa Glide driver allows full-screen rendering or rendering into
    an X window.  The glXSet3DfxModeMESA() function allows an application
    to switch between full-screen and windowed rendering.

IP Status

    Open-source; freely implementable.

Issues

    None.

New Procedures and Functions

    GLboolean glXSet3DfxModeMESA( GLint mode );

New Tokens

    GLX_3DFX_WINDOW_MODE_MESA	    0x1
    GLX_3DFX_FULLSCREEN_MODE_MESA   0x2

Additions to Chapter 3 of the GLX 1.3 Specification (Functions and Errors)

    The Mesa Glide device driver allows either rendering in full-screen
    mode or rendering into an X window.  An application can switch between
    full-screen and window rendering with the command:

	GLboolean glXSet3DfxModeMESA( GLint mode );

    <mode> may either be GLX_3DFX_WINDOW_MODE_MESA to indicate window
    rendering or GLX_3DFX_FULLSCREEN_MODE_MESA to indicate full-screen mode.

    GL_TRUE is returned if <mode> is valid and the operation completed
    normally.  GL_FALSE is returned if <mode> is invalid or if the Glide
    driver is not being used.

    Note that only one drawable and context can be created at any given
    time with the Mesa Glide driver.

GLX Protocol

    None since this is a client-side extension.

Errors

    None.

New State

    None.

Revision History

    8 June 2000 - initial specification
