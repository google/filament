<?php
$static_title = 'OpenGL&reg; Application Binary Interface for Linux';
$static_breadcrumb = array(
    '/registry/' => 'Registry',
    '/registry/OpenGL' => 'OpenGL',
    'NOLINK' => 'ABI'
);
include_once("../../../assets/static_pages/khr_page_top.php");
?>


<h1 style="text-align:center">
    OpenGL&reg; Application Binary Interface for Linux <br/>
    <span style="font-size:12px"> (formerly Linux/OpenGL Base) </span>
</h1>

<p style="text-align:center">Version 1.0<br/>
   Approved June 20, 2000<br/>
   Editor: Jon Leech, SGI </p>

<hr/>

<h6>Latest News</h6>

<p> Version 1.0 is complete. It was approved on June 20, 2000; all
    submitted votes were in favor. </p>

<hr/>

<h6>Index</h6>

    <ul>
    <li><a href="#1">1. Overview and Goals </a></li>
    <li><a href="#2">2. Calling Conventions</a></li>
    <li><a href="#3">3. Libraries</a></li>
    <li><a href="#4">4. Header Files</a></li>
    <li><a href="#5">5. Extension Headers</a></li>
    <li><a href="#6">6. Feedback and Mailing Lists</a></li>
    <li><a href="#app">Appendix: Open Issues</a></li>
    <li><a href="#log">Change Log</a></li>
    </ul>

<p> <a name="1"></a></p>
<h6>1. Overview and Goals </h6>

<p> 1.1. This document is intended to solve two related problems. First,
    defining the ABI and runtime environment for applications using
    OpenGL under X11 on Linux. This will enable applications using the
    OpenGL API for rendering to run on a variety of underlying
    implementations transparently. The intent is to address all of open
    source, commercial closed binary, OpenGL SI-based, and Mesa-based
    implementations. </p>

<p> Second, defining the SDK for developing apps using OpenGL. This
    includes header file locations, conventions for use of extensions,
    etc. </p>

<p> It has similar goals to the <a href="http://www.linuxbase.org">Linux
    Standard Base</a>, but focused much
    more narrowly: on the OpenGL API. Representatives from LSB are
    involved and ultimately this effort should be part of LSB. </p>

<p> We do not exactly track all LSB practice (particularly naming
    conventions for libraries) because LSB itself is not complete, and
    because existing practice with other OpenGL implementations suggests
    preferred methods which may differ from LSB. </p>

<p> 1.2. Things we do <b>not</b> attempt to address include: </p>

    <ul>
    <li> Internal implementation dependent issues - details of direct
        rendering, loadable driver modules, etc. Such details are
        hidden from the public interfaces by the implementation,
        and are irrelevant to applications using the ABI. </li>
    <li> Operating systems other than Linux. Other platforms such as BSD
        are welcome to use whatever comes out of this project, but we
        are explicitly not trying to solve this problem for every free
        OS in the world. </li>
    <li> Changes to the OpenGL API. The definition of OpenGL is
        controlled by the OpenGL Architecture Review Board, and we in no
        way challenge this. A single GLX extension is required; this
        extension has already been approved by the ARB. </li>
    <li> Use of 3D outside the X11/GLX context. There are a variety of
        approaches (fxMesa, GGI, etc.) that again are welcome to use
        relevant parts of this project, but whose support is not part of
        its goals. </li>
    </ul>

<p> 1.3. We believe all critical decisions have been made. Some
    remaining comments (previously identified as open issues) of
    interest are identified in the <a href="#app">appendix</a>. We
    recognize that some decisions are largely arbitrary (filenames and
    file locations, for example) and in those cases have been guided by
    existing practice (<i>in other words, complaining about arbitrary
    decisions is unlikely to change them</i>). </p>

<p> 1.4. Participants in this effort to date include people working at
    or involved with the following companies and open source projects
    (as well as a large number of individuals with unknown
    affiliations): </p>

    <blockquote>
    3Dfx, Alias/Wavefront, Apple, Avid, Compaq, Debian, HP, IBM, Intel,
    Linux Standard Base, Loki Games, Mesa, Metro Link, NVIDIA, Nichimen,
    Parametric Technology Corporation, Precision Insight, SGI, Sharp
    Eye, Sun, XFree86, Xi Graphics.</blockquote>

<p> <a name="2"></a></p>
<h6>2. Calling Conventions</h6>

<p> 2.1. OpenGL already includes its own datatypes (<tt>GLint,
    GLshort,</tt> etc.) used in the API. Guaranteed minimum sizes are
    stated (see table 2.2 of the OpenGL 1.2 Specification), but the
    actual choice of C datatype is left to the implementation. For our
    purposes, however, all implementations on a given binary
    architecture must have common definitions of these datatypes. </p>

<p> For the IA32 architecture, the definitions should be: </p>

<table border="1" class="center-table">
    <tr><td>GL datatype</td>
        <td>Description</td>
        <td>gcc equivalent for IA32</td></tr>
    <tr><td><tt>GLboolean</tt></td>
        <td>8-bit boolean</td>
        <td><tt>unsigned char</tt></td></tr>
    <tr><td><tt>GLbyte</tt></td>
        <td>signed 8-bit 2's-complement integer</td>
        <td><tt>signed char</tt></td></tr>
    <tr><td><tt>GLubyte</tt></td>
        <td>unsigned 8-bit integer</td>
        <td><tt>unsigned char</tt></td></tr>
    <tr><td><tt>GLshort</tt></td>
        <td>signed 16-bit 2's-complement integer</td>
        <td><tt>short</tt></td></tr>
    <tr><td><tt>GLushort</tt></td>
        <td>unsigned 16-bit integer</td>
        <td><tt>unsigned short</tt></td></tr>
    <tr><td><tt>GLint</tt></td>
        <td>signed 32-bit 2's-complement integer</td>
        <td><tt>int</tt></td></tr>
    <tr><td><tt>GLuint</tt></td>
        <td>unsigned 32-bit integer</td>
        <td><tt>unsigned int</tt></td></tr>
    <tr><td><tt>GLsizei</tt></td>
        <td>non-negative 32-bit binary integer size</td>
        <td><tt>int</tt></td></tr>
    <tr><td><tt>GLenum</tt></td>
        <td>enumerated 32-bit value</td>
        <td><tt>unsigned int</tt></td></tr>
    <tr><td><tt>GLbitfield</tt></td>
        <td>32 bit bitfield</td>
        <td><tt>unsigned int</tt></td></tr>
    <tr><td><tt>GLfloat</tt></td>
        <td>32-bit IEEE754 floating-point</td>
        <td><tt>float</tt></td></tr>
    <tr><td><tt>GLclampf</tt></td>
        <td>Same as GLfloat, but in range [0, 1]</td>
        <td><tt>float</tt></td></tr>
    <tr><td><tt>GLdouble</tt></td>
        <td>64-bit IEEE754 floating-point</td>
        <td><tt>double</tt></td></tr>
    <tr><td><tt>GLclampd</tt></td>
        <td>Same as GLdouble, but in range [0, 1]</td>
        <td><tt>double</tt></td></tr>
</table>

<p> <a href="#issue2.1">Issues</a></p>

<p> 2.2. Assembly-level call conventions must be shared. Since the
    OpenGL implementation may use C++ code internally (e.g. for GLU),
    this is potentially a serious problem. Static linking of C++
    libraries used by OpenGL libraries may be required of the
    implementation (also see the <a href="#3">Libraries</a> section
    below). </p>

<p> <a href="#issue2.2">Issues</a> </p>

<p> <a name="3"></a></p>
<h6>3. Libraries</h6>

<p> 3.1. There are two link-level libraries. <tt>libGL</tt> includes the
    OpenGL and GLX entry points and in general depends on underlying
    hardware and/or X server dependent code that may or may not be
    incorporated into this library. <tt>libGLU</tt> includes the GLU
    utility routines and should be hardware independent, using only the
    OpenGL API. </p>

<p> Each library has two names: the link name used
    on the ld command line, and the <tt>DT_SONAME</tt> within that
    library (specified by the <i>-soname</i> switch when linking the
    library), defining where it's looked up at runtime. Both forms must
    exist so that both linking and running will operate properly. The
    library names are: </p>

<table cellspacing="1" border="1" class="center-table">
    <tr><td>Link name</td>
        <td>Runtime name (<tt>DT_SONAME</tt>)</td>
    </tr>
    <tr><td><tt>libGL.so<tt></td>
        <td><tt>libGL.so.1<tt></td>
    </tr>
    <tr><td><tt>libGLU.so<tt></td>
        <td><tt>libGLU.so.1<tt></td>
    </tr>
</table>

<p> <tt>libGL.so</tt> and <tt>libGLU.so</tt> should
    be symbolic links pointing to the runtime names, so that
    future versions of the standard can be implemented transparently
    to applications by changing the link. </p>

<p> <a href="#issue3.1">Issues</a> </p>

<p> 3.2. These libraries must be located in <tt>/usr/lib</tt>. The
    X-specific library direction (<tt>/usr/lib/X11</tt>) was also
    considered, but existing practice on Linux and other platforms
    indicates that <tt>/usr/lib</tt> is preferable. </p>

<p> <a href="#issue3.2">Issues</a>

<p> 3.3. C++ runtime environments are likely to be incompatible
    cross-platform, including both the standard C++ library location and
    entry points, and the semantics of issues such as static
    constructors and destructors. The LSB apparently mandates static
    linking of libraries which aren't already in LSB, but this could
    lead to problems with multiple C++ RTLs present in the same app
    using C++. We'll have to tread carefully here until this issue
    is more completely understood. </p>

<p> <a href="#issue3.3">Issues</a> </p>

<p> 3.4. The libraries must export all OpenGL 1.2,
    GLU 1.3, GLX 1.3, and <tt>ARB_multitexture</tt> entry points
    statically. </p>

<p> It's possible (but unlikely) that additional ARB or vendor
    extensions will be mandated before the ABI is finalized.
    Applications should not expect to link statically against any entry
    points not specified here. </p>

<p> 3.5. Because non-ARB extensions vary so widely and are constantly
    increasing in number, it's infeasible to require that they all be
    supported, and extensions can always be added to hardware drivers
    after the base link libraries are released. These drivers are
    dynamically loaded by <tt>libGL</tt>, so extensions not in the base
    library must also be obtained dynamically. </p>

<p> 3.6. To perform the dynamic query,
    <tt>libGL</tt> also must export an entry point called </p>

    <blockquote>
        <tt>void (*glXGetProcAddressARB(const GLubyte *))();</tt>
    </blockquote>

<p> The <a href="http://www.opengl.org/registry/specs/ARB/get_proc_address.txt">full specification</a>
    of this function is available separately. It takes the string name
    of a GL or GLX entry point and returns a pointer to a function
    implementing that entry point. It is functionally identical to the
    <tt>wglGetProcAddress</tt> query defined by the Windows OpenGL
    library, except that the function pointers returned are <i>context
    independent</i>, unlike the WGL query. </p>

<p> All OpenGL and GLX entry points may be queried with this extension;
    GLU extensions cannot, because GLU is a client-side library that
    cannot easily be extended. </p>

<p> <a href="#issue3.6">Issues</a> </p>

<p> 3.7. Thread safety (the ability to issue OpenGL calls to different
    graphics contexts from different application threads) is required.
    Multithreaded applications must use <b>-lpthread</b>. </p>

<p> 3.8. <tt>libGL</tt> and <tt>libGLU</tt> must be
    transitively linked with any libraries they require in their own
    internal implementation, so that applications don't fail on some
    implementations due to not pulling in libraries needed not by the
    app, but by the implementation. </p>

<p> <a name="4"></a></p>
<h6>4. Header Files</h6>

<p> 4.1. The following header files are required: </p>

    <ul>
    <li> <tt>&lt;GL/gl.h&gt;</tt> - OpenGL </li>
    <li> <tt>&lt;GL/glx.h&gt;</tt> - GLX </li>
    <li> <tt>&lt;GL/glu.h&gt;</tt> - GLU </li>
    <li> <tt>&lt;GL/glext.h&gt;</tt> - OpenGL Extensions </li>
    <li> <tt>&lt;GL/glxext.h&gt;</tt> - GLX Extensions </li>
    </ul>

<p> These headers should properly define prototypes and enumerants for
    use by applications written in either C or C++. Other language
    bindings are not addressed at this time. </p>

<p> 4.2. These header files must be located in <tt>/usr/include/GL</tt>.
    <tt>/usr/include/X11/GL</tt> was considered and rejected for the
    same reasons as library locations in section 3.2 above. </p>

<p> 4.3. The required headers must not pull in
    internal headers or headers from other packages where that would
    cause unexpected namespace pollution (for example, on IRIX
    <tt>glx.h</tt> pulls in <tt>&lt;X11/Xmd.h&gt;</tt>). Likewise the
    required headers must be protected against multiple inclusion and
    should not themselves include any headers that are not so protected.
    However, <tt>glx.h</tt> is allowed to include
    <tt>&lt;X11/Xlib.h&gt;</tt> and <tt>&lt;X11/Xutil.h&gt;</tt>. </p>

<p> 4.4. <tt>glx.h</tt> must include the prototype of the
    <tt>glXGetProcAddressARB</tt> extension described above. </p>

<p> 4.5. All OpenGL 1.2 and <tt>ARB_multitexture</tt>, GLU 1.3, and GLX
    1.3 entry points and enumerants must be present in the corresponding
    header files <tt>gl.h</tt>, <tt>glu.h</tt>, and <tt>glx.h</tt>,
    <b>even if</b> only OpenGL 1.1 is implemented at runtime by the
    associated runtime libraries. </p>

<p> <a href="#issue4.5">Issues</a> </p>

<p> 4.6. Non-ARB OpenGL extensions are
    defined in <tt>glext.h</tt>, and non-ARB GLX extensions in
    <tt>glxext.h</tt>. If these extensions are also defined in one of
    the other required headers, this must be done conditionally so that
    multiple definition problems don't occur. </p>

<p> <a href="#issue4.6">Issues</a> </p>

<p> 4.7. Vendor-specific shortcuts, such as macros for higher
    performance GL entry points, are intrinsically unportable. These
    should <b>not</b> be present in the required header files, but
    instead in a vendor-specific header file that requires explicit
    effort to access, such as defining a vendor-specific preprocessor
    symbol. Likewise vendors who are not willing to include their
    extensions in <tt>glext.h</tt> must isolate those extensions in
    vendor-specific headers. </p>

<p> 4.8. <tt>gl.h</tt> must define the symbol
    <tt>GL_OGLBASE_VERSION</tt>. This symbol must be an integer defining
    the version of the ABI supported by the headers. Its value is
    <i>1000 * major_version + minor_version</i> where
    <i>major_version</i> and <i>minor_version</i> are the major and
    minor revision numbers of this ABI standard. The primary purpose of
    the symbol is to provide a compile-time test by which application
    code knows whether the ABI guarantees are in force. </p>

<p> <a href="#issue4.8">Issues</a> </p>

<p> <a name="5"></a></p>
<h6>5. Extension Headers</h6>

<p> 5.1. Providing prototypes and enumerants for OpenGL extensions is
    challenging because of the expected wide variety of hardware
    drivers, continuing creation of extensions, and multiple sources of
    header files on Linux OpenGL implementations. Some extensions will
    be supported only for a specific implementation, and some will be
    supported only for a specific hardware driver within that
    implementation. This situation does not lend itself easily to
    independent maintenance of header files definining the extensions.
    </p>

<p> Instead, we require a single header file defining <b>all</b> OpenGL
    extensions be supplied from a central point and updated on a
    continuing basis as new extensions are added to the OpenGL <a
    href="http://www.opengl.org/registry/">extension registry</a> (which
    is similarly centrally maintained). The central point is in the
    registry at <a href="http://www.opengl.org/registry/">
    http://www.opengl.org/registry/</a>. </p>

<p> The <a href="../api/GL/glext.h">latest version of
    <tt>glext.h</tt></a> is available in the registry. It is
    automatically generated from the master OpenGL function and
    enumerant registries, and is updated as new extensions are
    registered. The header is intended to be useful on other platforms
    than Linux, particularly Windows; please let us know (via feedback
    to OpenGL.org forums) if it needs enhancement for use on another
    platform. The generator scripts and &quot;.spec&quot; files used in
    generating glext.h are also available. </p>

<p> Likewise for GLX, a single header defining
    all GLX extensions, <a href="../api/GL/glxext.h"><tt>glxext.h</tt></a>,
    is required and is maintained centrally. </p>

<p> The registry also contains a header defining WGL
    extensions, <a href="../api/GL/wglext.h"><tt>wglext.h</tt></a>, but this is
    only for use on Windows; <tt>wglext.h</tt> is <b>not</b> required by
    or useful for the Linux ABI. </p>

<p> <a href="#issue5.1">Issues</a> </p>

<p> 5.2. The centrally maintained <tt>glext.h</tt> will be continually
    updated, so version creep is expected. This could cause problems for
    open source projects distributing source code. The proper solution
    is for users to update glext.h to the latest version, but versioning
    has proven helpful with other extensible aspects of OpenGL.
    Therefore <tt>glext.h</tt> must include a preprocessor version
    symbol <tt>GL_GLEXT_VERSION</tt>, enabling apps to do something
    like: </p>

<blockquote>
    <tt>
    #include &lt;GL/glext.h&gt;<br>
    #if GL_GLEXT_VERSION &lt; 42<br>
    #error "I need a newer &lt;GL/glext.h&gt;. Please download it from http://www.opengl.org/registry/ABI/"<br>
    #endif
    </tt>
</blockquote>

<p> <a href="#issue5.2">Issues</a> </p>

<p> 5.3. Only extensions whose fully documented specifications have been
    made available to the extension registry and whose authors have
    committed to shipping them in their drivers will be included in
    <tt>glext.h</tt> and <tt>glxext.h</tt>. The structure of each
    extension defined in these headers should resemble: </p>

<blockquote>
    <tt>
    #ifndef GL_EXT_<i>extensionname</i><br>
    #define GL_EXT_<i>extensionname</i> 1<br>
    <i> Define enumerants specific to this extension</i><br>
    <i> Typedef function pointers for entry points specifically to
        this extension, dynamically obtained
        with glXGetProcAddressARB</i><br>
    #ifdef GL_GLEXT_PROTOTYPES<br>
    <i> Define prototypes specific to this extension</i><br>
    #endif<br>
    #endif
    </tt>
</blockquote>

<p> Benign redefinition of the enumerants is allowable, so these may be
    outside protective <tt>#ifndef</tt> statements (this structure
    results from the generator scripts used in the OpenGL SI to build
    <tt>glext.h</tt>, and also because some enums may be defined by
    multiple different extensions, so it could make sense to segregate
    them). </p>

<p> Function pointer typedefs will use the Windows convention (e.g. the
    typedef for a function <tt>glFooARB</tt> will be
    <tt>PFNGLFOOARBPROC</tt>) for application source code portability.
    </p>

<p> Normally, prototypes are present in
    the header files, but are not visible due to conditional compilation.
    To define prototypes as well as typedefs, the application must
    <tt>#define GL_GLEXT_PROTOTYPES</tt> prior to including
    <tt>gl.h</tt> or <tt>glx.h</tt>. <i>(Note: consistency suggests
    using <tt>GLX_GLXEXT_PROTOTYPES</tt> for <tt>glxext.h</tt> -
    TBD)</i>. </p>

<p> The preprocessor symbol protecting the extension declaration
    must be the same as the name string identifying the extension at
    runtime and in the extension registry. </p>

<p> <b>All</b> OpenGL and GLX extensions that are shipping should have a
    full extension specification in the master
    <a href="http://www.opengl.org/registry">
    extension registry</a> on www.opengl.org. Vendors failing to document
    and specify their on extensions will not be allowed to incorporate
    the resulting inadequate interfaces into the ABI. </p>

<p> <a href="#issue5.3">Issues</a> </p>

<p> 5.4. <tt>glext.h</tt> is normally
    <tt>#include</tt>ed by <tt>gl.h</tt>. This inclusion can be
    suppressed by the application defining the preprocessor symbol
    <tt>GL_GLEXT_LEGACY</tt> prior to its <tt>#include
    &lt;GL/gl.h&gt;</tt>. </p>

<p> <img src="new.gif">Similarly, <tt>glxext.h</tt> is normally
    <tt>#include</tt>ed by <tt>glx.h</tt>. This inclusion can be
    suppressed by the application defining the preprocessor symbol
    <tt>GLX_GLXEXT_LEGACY</tt> prior to its <tt>#include
    &lt;GL/glx.h&gt;</tt>. </p>

<p> <a href="#issue5.4">Issues</a> </p>

<p> <a name="6"></a></p>
<h6>6. Feedback and Mailing Lists</h6>

<p> Since the ABI has been finalized, we are no longer maintaining the
    oglbase-discuss mailing list used during its development. List
    archives may still be available from
    <a href="http://www.mail-archive.com/oglbase-discuss@corp.sgi.com/">
    http://www.mail-archive.com/oglbase-discuss@corp.sgi.com/</a> </p>

<hr/>

<p> <a name="app"></a></p>
<h6>Appendix: Open Issues</h6>

<p> <a name="issue2.1"></a>
    <b>Section 2.1</b>:
    Define GL datatypes for other supported Linux architectures - Alpha,
    PowerPC, MIPS, etc. (in general these will be identical to the IA32
    types). Note: we may want to suggest <tt>GLlong</tt> and
    <tt>GLulong</tt> as 64-bit datatypes for future OpenGL revisions. </p>

<p> <a name="issue2.2"></a>
    <b>Section 2.2</b>:
    C++ libraries at runtime can be problematic - take the gcc/egcs
    split, for example. Another potential problem area is static
    constructor/destructor issues, e.g. when a C <tt>main()</tt> is
    linked against GLU. Some tweaking may be required as apps running
    against different ABI revisions start appearing. </p>

<p> <a name="issue3.1"></a>
    <b>Section 3.1</b>:
    LSB uses a more complex naming convention for libraries; we're
    avoiding this at least for now, because these conventions disagree
    with common practice on virtually all other Unix OpenGL
    implementations. </p>

<p> <a name="issue3.2"></a>
    <b>Section 3.2 (also Section 4.1)</b>:
    Placing the headers and libraries in non-X11 specific locations
    could impact non-GLX OpenGL implementations resident on the same
    platform. It is also somewhat out of keeping with other X
    extensions. However, this practice is so common on other platforms,
    and non-X based OpenGL implementations are so rarely used, that we
    chose to do so for build portability and "principle of least
    surprise" purposes. </p>

<p> Nothing prohibits the implementation from
    placing the actual library files in other locations and implementing
    the required library paths as links. </p>

<p> <a name="issue3.3"></a>
    <b>Section 3.3</b>:
    The ABI should probably state requirements on GL libraries using C++
    or other auxiliary libraries, such that no conflict will arise with
    apps also using those libraries. </p>

<p> <a name="issue3.6"></a>
    <b>Section 3.6</b>:
    The context-independence requirement was the subject of enormous
    controversy, mostly because the consequences of this requirement on
    the underlying link library and driver implementations can be
    significant. It is impossible to briefly recap the many pro and con
    arguments briefly; refer to the <a href="#6">mailing list
    archive</a> to learn more. </p>

<p> GLU does sometimes need to be extended to
    properly support new GL extensions; in particular, new pixel formats
    and types, or new targets for texture downloads, such as cube
    mapping, should ideally be exposed through the GLU mipmap generation
    routines. This is an unresolved problem, since GLU is client code
    not specific to any GL driver and thus not dynamically loadable. The
    best current option is for driver suppliers to make sure that
    whatever GLU functionality they need is contributed to the OpenGL
    Sample Implementation's GLU library. </p>

<p> Portable applications should treat the pointers
    as context-dependent. </p>

<p> We haven't determined if any non-ARB extensions should be standard
    entry points not requiring this dynamic lookup. As a reference
    point, here are lists of GL, GLX, and GLU extensions supported by a
    variety of OpenGL and Mesa implementations today (please send
    additions for other platforms to the oglbase-discuss mailing list so
    they can be added): </p>

    <ul>
    <li><a href="ext/3dlabs.txt">3Dlabs</a> </li>
    <li><a href="ext/compaq.txt">Compaq</a> </li>
    <li><a href="ext/intergraph.txt">Intergraph/Intense 3D</a> </li>
    <li><a href="ext/mesa.txt">Mesa</a> </li>
    <li><a href="ext/sgi.txt">SGI (multiple platforms)</a> </li>
    <li><a href="ext/sun_ultra.txt">Sun Ultra</a> </li>
    <li><a href="ext/xig.txt">Xi Graphics</a> </li>
    </ul>

<p> <a name="issue4.5"></a>
    <b>Section 4.5</b>:
    Implementations may still implement only OpenGL 1.1 functionality,
    but the 1.2 header and link library material must still be provided.
    Since applications must already check both compile and runtime
    OpenGL version numbers, no problems due to lacking support for 1.2
    are expected. The next version of this standard is anticipated to
    require OpenGL 1.2 support. </p>

<p> <a name="issue4.6"></a>
    <b>Section 4.6</b>:
    It's important that <tt>glext.h</tt> and <tt>glxext.h</tt> can be
    updated from the extension registry without breaking <tt>gl.h</tt>
    and <tt>glx.h</tt>. Making sure that all extension definitions are
    properly protected helps to this end, as well as being good
    programming practice. </p>

<p> <a name="issue4.8"></a>
    <b>Section 4.8</b>:
    <tt>GL_OGLBASE_VERSION</tt> is mostly provided so that apps can
    determine whether to use traditional static linking of extensions,
    or to dynamically query them. Unlike GL/GLX versioning, the ABI
    version is not dynamically queryable at runtime. Historical
    experience suggests that not providing the runtime query to begin
    with is a bad decision. </p>

<p> <a name="issue5.1"></a>
    <b>Section 5.1</b>:
    <tt>glext.h</tt> is an exception to the Linux-centric nature of this
    document, since it is already being used on other platforms. </p>

<p> <a name="issue5.2"></a>
    <b>Section 5.2</b>:
    Applications should <b>not</b> use the version number in
    <tt>glext.h</tt> to test for presence or absence of specific
    extension prototypes; this is extremely unportable and dangerous.
    Always use the extension-specific symbols described in section 5.3.
    </p>

<p> The header version symbol was changed from
    <tt>GL_GLEXT_VERSION_EXT</tt> to <tt>GL_GLEXT_VERSION</tt> for
    consistency with the <tt>GLEXT</tt> namespace the ABI group has
    started using. </p>

<p> <a name="issue5.3"></a>
    <b>Section 5.3</b>:
    Other structures for the extension prototypes have been suggested,
    such as having separate header files for each extension. Having both
    structures may be preferable, but it requires more work. </p>

<p> <a name="issue5.4"></a>
    <b>Section 5.4</b>:
    It's important to be able to suppress automatic inclusion of
    <tt>glext.h</tt> and <tt>glxext.h</tt> in order to support
    compilation of legacy code not written to be ABI-aware (e.g.
    assuming that extensions can be statically linked). </p>

<p> <a name="log"></a></p>
<h6>7. Change Log</h6>

<ul>
<li> 10/9/2006 - updated registry links to the new location on
     opengl.org and cleaned up other dangling wording due to the move
     from oss.sgi.com.
<li> 6/20/2000 (version 1.0) - Linux ABI approved on the oglbase-discuss
     mailing list. Corrected Windows function-pointer typedef convention
     in section 5.3 by appending <tt>PROC</tt>, to match what glext.h
     already does. </li>
<li> 5/29/2000 (version 0.9.8) - <tt>glxext.h</tt> added to section 4.
     Resolution reached on the structure of <tt>glext.h</tt> and
     <tt>glxext.h</tt>, and how they are included from <tt>gl.h</tt> and
     <tt>glx.h</tt>. In particular, <tt>GL_OGLBASE_VERSION</tt> symbol
     defined, default inclusion of extension headers from core headers
     mandated, <tt>GL_GLEXT_PROTOTYPES</tt> may be specified in order to
     get extension prototypes as well as function pointer typedefs.
     Renamed <tt>GL_GLEXT_VERSION_EXT</tt> to <tt>GL_GLEXT_VERSION</tt>.
     </li>
<li> 4/9/2000 (version 0.9.7) - <tt>glext.h</tt> is now available
     together with the ABI specification. </li>
<li> 2/22/2000 (version 0.9.6) - Revised for public comment period.
     Moved open issues to the new appendix. </li>
<li> 2/8/2000 (version 0.9.5) - Removed ellipses from prototype in
     section 3.6, and simplified the lists of SGI supported extensions
     into one file. Mandated threadsafety in section 3.7. Moved
     glXGetProcAddressARB prototype from gl.h to glx.h in section 4.4,
     since the function itself was moved from gl to glX during
     standardization. Restructured the page to fit into the ogl-sample
     site on oss.sgi.com, next to the extension registry. Pointed to the
     updated extension registry on oss.sgi.com in several places. </li>
<li> 12/9/99 (version 0.9.4) - Added Intergraph extension list in
     section 3.6. </li>
<li> 12/6/99 (version 0.9.3) - Added Compaq and 3Dlabs extension
     lists in section 3.6. </li>
<li> 11/23/99 (version 0.9.2) - Refined discussion of
     glXGetProcAddressARB to specify that any GL or GLX function can be
     queried. </li>
<li> 11/23/99 (version 0.9.1) - Summing up lots of email discussion.
     Expanded participant list in section 1.4. Pinned down library
     naming scheme in section 3.1. Changed to require statically
     exporting all GL 1.2 / GLX 1.3 / GLU 1.3 / ARB extension entry
     points in section 3.4. Changed GetProcAddress from EXT to ARB and
     from gl to glX(in anticipation of ARB approval) in section 3.5.
     Does <b>not</b> require a context parameter. Require Windows naming
     convention for <tt>glext.h</tt> function prototypes in section 5.3.
     Added a link to the list archives in section 6. </li>
<li> 9/16/1999 - Added Mesa, Sun, and Xi Graphics extension lists in
     section 3.6. Added section 3.8 on transitive library dependencies
     of the GL libraries. </li>
<li> 9/10/1999 - Added initial list of GL/GLX/GLU extensions
     for existing platforms in section 3.6.<br>
     Specified text/link colors as well as background color. </li>
<li> 9/7/1999 - Initial version. </li>
</ul>

<?php include_once("../../../assets/static_pages/khr_page_bottom.php"); ?>
</body>
</html>
