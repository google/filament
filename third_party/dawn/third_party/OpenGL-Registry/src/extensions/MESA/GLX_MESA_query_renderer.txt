Name

    MESA_query_renderer

Name Strings

    GLX_MESA_query_renderer

Contact

    Ian Romanick <ian.d.romanick@intel.com>

IP Status

    No known IP claims.

Status

    Shipping as of Mesa 10.0

Version

    Version 9, 09 November 2018

Number

    OpenGL Extension #446

Dependencies

    GLX 1.4 is required.

    GLX_ARB_create_context and GLX_ARB_create_context_profile are required.

Overview

    In many situations, applications want to detect characteristics of a
    rendering device before creating a context for that device.  Information
    gathered at this stage may guide choices the application makes about
    color depth, number of samples per-pixel, texture quality, and so on.
    In addition, versions of supported APIs and implementation API
    preference may also guide start-up decisions made by the application.
    For example, one implementation may prefer vertex data be supplied using
    methods only available in a compatibility profile, but another
    implementation may only support the desired version in a core profile.

    There are also cases where more than one renderer may be available per
    display.  For example, there is typically a hardware implementation and
    a software based implementation.  There are cases where an application
    may want to pick one over the other.  One such situation is when the
    software implementation supports more features than the hardware
    implementation.  Another situation is when a particular version of the
    hardware implementation is blacklisted due to known bugs.

    This extension provides a mechanism for the application to query all of
    the available renderers for a particular display and screen.  In
    addition, this extension provides a mechanism for applications to create
    contexts with respect to a specific renderer.

New Procedures and Functions

    Bool glXQueryRendererIntegerMESA(Display *dpy, int screen,
                                     int renderer, int attribute,
                                     unsigned int *value);
    Bool glXQueryCurrentRendererIntegerMESA(int attribute, unsigned int *value);

    const char *glXQueryRendererStringMESA(Display *dpy, int screen,
                                           int renderer, int attribute);

    const char *glXQueryCurrentRendererStringMESA(int attribute);

New Tokens

    Accepted as an <attribute> in glXQueryRendererIntegerMESA and
    glXQueryCurrentRendererIntegerMESA:

        GLX_RENDERER_VENDOR_ID_MESA                      0x8183
        GLX_RENDERER_DEVICE_ID_MESA                      0x8184
        GLX_RENDERER_VERSION_MESA                        0x8185
        GLX_RENDERER_ACCELERATED_MESA                    0x8186
        GLX_RENDERER_VIDEO_MEMORY_MESA                   0x8187
        GLX_RENDERER_UNIFIED_MEMORY_ARCHITECTURE_MESA    0x8188
        GLX_RENDERER_PREFERRED_PROFILE_MESA              0x8189
        GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA    0x818A
        GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA    0x818B
        GLX_RENDERER_OPENGL_ES_PROFILE_VERSION_MESA      0x818C
        GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA     0x818D

    Accepted as an <attribute> in glXQueryRendererStringMESA and
    glXQueryCurrentRendererStringMESA:

        GLX_RENDERER_VENDOR_ID_MESA
        GLX_RENDERER_DEVICE_ID_MESA

Additions to the OpenGL / WGL Specifications

    None. This specification is written for GLX.

Additions to the GLX 1.4 Specification

    [Add to Section 3.3.2 "GLX Versioning" of the GLX Specification]

    To obtain information about the available renderers for a particular
    display and screen,

        Bool glXQueryRendererIntegerMESA(Display *dpy, int screen, int renderer,
                                         int attribute, unsigned int *value);

    can be used.  The value for <attribute> will be returned in one or more
    integers specified by <value>.  The values, data sizes, and descriptions
    of each renderer attribute are listed in the table below.

    GLX renderer attribute         number     description
                                  of values
    ----------------------        ---------   -----------
    GLX_RENDERER_VENDOR_ID_MESA   1           PCI ID of the device vendor
    GLX_RENDERER_DEVICE_ID_MESA   1           PCI ID of the device
    GLX_RENDERER_VERSION_MESA     3           Major, minor, and patch level of
                                              the renderer implementation
    GLX_RENDERER_ACCELERATED_MESA 1           Boolean indicating whether or
                                              not the renderer is hardware
                                              accelerated
    GLX_RENDERER_VIDEO_MEMORY_MESA 1          Number of megabytes of video
                                              memory available to the renderer
    GLX_RENDERER_UNIFIED_MEMORY_ARCHITECTURE_MESA
                                  1           Boolean indicating whether or
                                              not the renderer uses a unified
                                              memory architecture or has
                                              separate "on-card" and GART
                                              memory.
    GLX_RENDERER_PREFERRED_PROFILE_MESA
                                  1           Bitmask of the preferred context
                                              profile for this renderer.  This
                                              value is suitable to be supplied
                                              with the
                                              GLX_CONTEXT_PROFILE_MASK_ARB
                                              attribute to
                                              glXCreateContextAttribsARB
    GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA
                                  2           Maximum core profile major and
                                              minor version supported by the
                                              renderer
    GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA
                                  2           Maximum compatibility profile
                                              major and minor version
                                              supported by the renderer
    GLX_RENDERER_OPENGL_ES_PROFILE_VERSION_MESA
                                  2           Maximum OpenGL ES 1.x
                                              major and minor version
                                              supported by the renderer
    GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA
                                  2           Maximum OpenGL ES 2.x or 3.x
                                              major and minor version
                                              supported by the renderer

    In the table, boolean attributes will have either the value 0 or 1.

    GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA,
    GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA,
    GLX_RENDERER_OPENGL_ES_PROFILE_VERSION_MESA, and
    GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA each return <0, 0> in
    *value if no version of that profile is supported.

    GLX_RENDERER_VENDOR_ID_MESA and GLX_RENDERER_DEVICE_ID_MESA may return
    0xFFFFFFFF if the device does not have a PCI ID (because it is not a PCI
    device) or if the PCI ID is not available.  In this case the application
    should rely on the string query instead.

    If <attribute> is not a recognized value, False is returned, but no GLX
    error is generated.  Otherwise, True is returned.

    String versions of some attributes may also be queried using

        const char *glXQueryRendererStringMESA(Display *dpy, int screen,
                                               int renderer, int attribute);

    The value for <attribute> will be returned in one or more
    integers specified by <value>.  The values, data sizes, and descriptions
    of each renderer attribute are listed in the table below.

    GLX renderer attribute        description
    ----------------------        -----------
    GLX_RENDERER_VENDOR_ID_MESA   Name of the renderer provider.  This may
                                  differ from the vendor name of the
                                  underlying hardware.
    GLX_RENDERER_DEVICE_ID_MESA   Name of the renderer.  This may differ from
                                  the name of the underlying hardware (e.g.,
                                  for a software renderer).

    If <attribute> is not a recognized value, NULL is returned, but no GLX
    error is generated.

    The string returned for GLX_RENDERER_VENDOR_ID_MESA will have the same
    format as the string that would be returned by glGetString of GL_VENDOR.
    It may, however, have a different value.

    The string returned for GLX_RENDERER_DEVICE_ID_MESA will have the same
    format as the string that would be returned by glGetString of GL_RENDERER.
    It may, however, have a different value.

Issues

    1) How should the difference between on-card and GART memory be exposed?

        UNRESOLVED.

    2) How should memory limitations of unified memory architecture (UMA)
    systems be exposed?

        UNRESOLVED.  Some hardware has different per-process and global
        limits for memory that can be accessed within a single draw call.

    3) How should the renderer's API preference be advertised?

        UNRESOLVED.  The common case for desktop renderers is to prefer
        either core or compatibility.  However, some renderers may actually
        prefer an ES context.  This leaves the application in a tough spot
        if it can only support core or compatibility and the renderer says it
        wants ES.

    4) Should OpenGL ES 2.0 and OpenGL ES 3.0 be treated separately?

        RESOLVED.  No.  OpenGL ES 3.0 is backwards compatible with OpenGL ES
        2.0.  Applications can detect OpenGL ES 3.0 support by querying
        GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA.

    5) How can applications tell the difference between different hardware
    renderers for the same device?  For example, whether the renderer is the
    open-source driver or the closed-source driver.

        RESOLVED.  Assuming this extension is ever implemented outside Mesa,
        applications can query GLX_RENDERER_VENDOR_ID_MESA from
        glXQueryRendererStringMESA.  This will almost certainly return
        different strings for open-source and closed-source drivers.

    6) What is the value of GLX_RENDERER_UNIFIED_MEMORY_ARCHITECTURE_MESA for
    software renderers?

        UNRESOLVED.  Video (display) memory and texture memory is not unified
        for software implementations, so it seems reasonable for this to be
        False.

    7) How does an application determine the number of available renderers?

        UNRESOLVED.

    8) What happens if a fbconfig is used to create context on a renderer
    that cannot support it?  For example, if a multisampled config is used
    with a software renderer that does not support multisampling.

        RESOLVED.  The language for glXCreateContextAttribsARB already covers
        this case.  Context creation will fail, and BadMatch is generated.

    9) In addition to being able to query the supported versions, should
    applications also be able to query the supported extensions?

        RESOLVED.  No.  Desktop OpenGL core profiles and OpenGL ES 3.0 have
        moved away from the monolithic string returned by glGetString of
        GL_EXTENSIONS.  Providing the newer indexed query would require adding
        a lot of extra infrastructure, and it would probably provide little
        benefit to applications.

    10) What combination of values for GLX_RENDERER_PREFERRED_PROFILE_MESA,
    GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA, and
    GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA should be returned
    for a renderer that only supports OpenGL 3.1 without the
    GL_ARB_compatibility extension?

        RESOLVED.  The renderer will return GLX_CONTEXT_CORE_PROFILE_BIT_ARB
        for GLX_RENDERER_PREFERRED_PROFILE_MESA.

        Further, the renderer will return <3,0> for
        GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA because OpenGL
        3.1 without GL_ARB_compatibility is not backwards compatible with
        previous versions of OpenGL.  The render will return <3,1> for
        GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA indicating that support
        for OpenGL 3.1 is available.

        Even though there is no OpenGL 3.1 core profile, the values
        returned for GLX_RENDERER_PREFERRED_PROFILE_MESA and
        GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA can be supplied
        with the GLX_CONTEXT_PROFILE_MASK_ARB and
        GLX_CONTEXT_{MAJOR,MINOR}_VERSION_ARB attributes of
        glXCreateContextAttribsARB without error.  If the requested
        OpenGL version is less than 3.2, the
        GLX_CONTEXT_PROFILE_MASK_ARB attribute is ignored by
        glXCreateContextAttribsARB.

    11) How can application learn about multi-GPU (e.g., SLI, CrossFireX,
    etc.) configurations?

        UNRESOLVED.  Based on ISV feedback, this is important information to
        provide to the application.  Given the variety of possible hardware
        configurations (e.g., Hybrid CrossFireX) and different rendering
        modes (e.g., split-frame rendering vs. alternate-frame rendering),
        it's not clear how this information can be communicated.

        It is likely that this will be left to a layered extension.

    12) Should capability queries similar to those in
    GL_ARB_internalformat_query or GL_ARB_internalformat_query2 be added?

        RESOLVED.  No.  With the possible exception of the texture size
        queries, it seems unlikely that applications would ever use this
        information before creating a context.

    13) Existing GL extensions (e.g., GL_ATI_meminfo and
    GL_NVX_gpu_memory_info) allow easy queries after context creation.  With
    this extension it is a bit of a pain for a portable application to query
    the information after context creation.

        RESOLVED.  Add versions of the queries that implicitly take the
        display, screen, and renderer from the currently bound context.

    14) Why not make the queries from issue #13 GL functions (instead of GLX)?

        RESOLVED.  It is fairly compelling for the post-creation queries to
        just use glGetInteger and glGetString.  However, the GL enums and
        the GLX enums would have different names and would almost certainly
        have different values.  It seems like this would cause more problems
        than it would solve.

    15) Should the string queries be required to return the same values as
    glGetString(GL_VENDOR) and glGetString(GL_RENDERER)?

        UNRESOLVED.  This may be useful for applications that already do
        device detection based on these strings.

    16) What type should the value parameter of glXQueryRendererIntegerMESA
        and glXQueryCurrentRendererIntegerMESA be?

        UNRESOLVED.  Other similar GLX query functions just use int or
        unsigned int, so that's what this extension uses for now.  However,
        an expeclitly sized value, such as uint32_t or uint64_t, seems
        preferable.

    17) What about SoCs and other systems that don't have PCI?

        RESOLVED. The GLX_RENDERER_VENDOR_ID_MESA and
        GLX_RENDERER_DEVICE_ID_MESA integer queries may return 0xFFFFFFFF if a
        PCI ID either does not exist or is not available.  Implementations
        should make every attempt to return as much information as is
        possible.  For example, if the implementation is running on a non-PCI
        SoC with a Qualcomm GPU, GLX_RENDERER_VENDOR_ID_MESA should return
        0x5143, but GLX_RENDERER_DEVICE_ID_MESA will return 0xFFFFFFFF.

Revision History

    Version 1, 2012/08/27 - Initial version

    Version 2, 2012/09/04 - Specify behavior of implementations that
                            do not support certain profiles.
                            Change wording of issue #8 to be more
                            clear.
                            Make some wording changes to issue #10 to
                            clarify the resolution a bit.

    Version 3, 2012/09/23 - Add issue #11 regarding multi-GPU systems.

    Version 4, 2013/02/01 - Add issue #12 regarding texture / renderbuffer
                            format queries.

    Version 5, 2013/02/14 - Add issues #13 and #14 regarding simpler queires
                            after the context is created and made current.
                            Add issue #15 regarding the string query.
                            Add issue #16 regarding the value type returned
                            by the Integer functions.

    Version 6, 2013/10/25 - Fix a typo.  Update the list of functions to
                            which the new enums can be passed.  The "Current"
                            versions were previously missing.

    Version 7, 2013/11/07 - Fix a couple more typos.  Add issue #17 regarding
                            the PCI queries on systems that don't have PCI.

    Version 8, 2014/02/14 - Fix a couple typos. GLX_RENDER_ID_MESA should
                            read GLX_RENDERER_ID_MESA. The VENDOR/DEVICE_ID
                            example given in issue #17 should be 0x5143 and
                            0xFFFFFFFF respectively.

    Version 9, 2018/11/09 - Remove GLX_RENDERER_ID_MESA, which has never been
                            implemented. Remove the unnecessary interactions
                            with the GLX GLES profile extensions. Note the
                            official GL extension number. Specify the section
                            of the GLX spec to modify.
