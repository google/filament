<?php
$static_title = 'Khronos OpenGL&reg; Registry';

include_once("../../assets/static_pages/khr_page_top.php");
?>

<p> The OpenGL Registry contains specifications of the core API and
    shading language; specifications of Khronos- and vendor-approved
    OpenGL extensions; header files corresponding to the specifications; and
    related documentation including specifications, extensions, and headers
    for the GLX, WGL, and GLU APIs. </p>

<p> The OpenGL registry is part of the <a
    href="http://www.khronos.org/registry/OpenGL/"> Combined OpenGL Registry </a>
    for OpenGL, OpenGL ES, and OpenGL SC, which includes the <a
    href="xml/README.adoc">XML API registry</a> of reserved enumerants and
    functions. </p>

<p> <b>Table of Contents</b>
<ul>
<li> <b><a href="docs/update_policy.php">Working Group Policy</a></b> for
     when Specifications and extensions will be updated. </li>
<li> <b><a href="#apispecs">Current OpenGL API, Shading Language, GLX,
     and Related Specifications and Reference Pages</a></b> </li>
<li> <a href="#headers">Core API and Extension Header Files</a> </li>
<li> <b>
     <a href="https://www.khronos.org/files/ip-disclosures/opengl/"> IP
     Disclosures</a> Potentially Affecting OpenGL Implementations
     </b></li>
<li> <b> Extension Specifications </b>
    <ul>
    <li> <a href="#arbextspecs">OpenGL ARB Extensions Specifications</a> </li>
    <li> <a href="#otherextspecs">OpenGL Vendor and EXT Extension Specifications</a>
         </li>
    </ul> </li>
<li> Older Material for Reference
    <ul>
    <li> <b><a href="#oldspecs">Older OpenGL and GLX Specifications and
         Reference Pages</a></b> </li>
    <li> <a href="#abi">OpenGL Application Binary Interface for Linux</a>
         </li>
    </ul> </li>
</ul>

<hr>

<h2> <a name="apispecs"></a>
     Current OpenGL API, OpenGL Shading Language and GLX Specifications and
     Reference Pages </h2>

<ul>
<li> <b> Current Specifications (OpenGL 4.6) </b>
<li> OpenGL 4.6 API Specification (May 5, 2022)
    <ul>
    <li> <a href="specs/gl/glspec46.core.pdf"> Core Profile Specification
         </a> </li>
    <li> <a href="specs/gl/glspec46.core.withchanges.pdf"> Core Profile
         Specification with changes marked </a> </li>
    <li> <a href="specs/gl/glspec46.compatibility.pdf"> Compatibility Profile
         Specification </a> </li>
    <li> <a href="specs/gl/glspec46.compatibility.withchanges.pdf">
         Compatibility Profile Specification with changes marked </a>
         </li>
    </ul> </li>
<li> OpenGL Shading Language 4.60 Specification (August 14, 2023)
     <a href="specs/gl/GLSLangSpec.4.60.html"> (HTML) </a> </li>
     <a href="specs/gl/GLSLangSpec.4.60.pdf"> (PDF) </a> </li>

<li> <a href="http://www.khronos.org/registry/OpenGL-Refpages/gl4/"> OpenGL 4.5 API and
     Shading Language Reference Pages </a> (not yet updated) </li>

<li> <b> OpenGL X Window System Binding (GLX 1.4) Specification </b> </li>
<li> <a href="specs/gl/glx1.4.pdf"> GLX 1.4 Specification </a> </li>

<li> <a href="https://www.khronos.org/developers/reference-cards"> OpenGL
     Quick Reference Card </a> (available for different API versions). </li>
</ul>


<h2> <a name="headers"></a> API and Extension Header Files </h2>

<p> Because extensions vary from platform to platform and driver to
    driver, OpenGL developers can't expect interfaces for all extensions
    to be defined in the standard <tt>gl.h</tt>, <tt>glx.h</tt>, and
    <tt>wgl.h</tt> header files supplied with the OS / graphics drivers.
    Additional header files are provided here, including: </p>

<p> Almost all of the headers described below depend on a platform header
    file common to multiple Khronos APIs called
    <tt>&lt;KHR/khrplatform.h&gt;</tt>. </p>


<ul>
<li> <tt><a href="api/GL/glext.h">&lt;GL/glext.h&gt;</a></tt> - OpenGL
     1.2 and above compatibility profile and extension interfaces. </li>
<li> <tt><a href="api/GL/glcorearb.h">&lt;GL/glcorearb.h&gt;</a></tt> -
     OpenGL core profile and ARB extension interfaces, as described in
     appendix G.2 of the OpenGL 4.3 Specification. Does not include
     interfaces found only in the compatibility profile. </li>
<li> <tt><a href="api/GL/glxext.h">&lt;GL/glxext.h&gt;</a></tt> - GLX
     1.3 and above API and GLX extension interfaces. </li>
<li> <tt><a href="api/GL/wglext.h">&lt;GL/wglext.h&gt;</a></tt> - WGL
     extension interfaces. </li>
</ul>

<p> These headers define interfaces including enumerants; prototypes; and,
    for platforms supporting dynamic runtime extension queries, such as
    Linux and Microsoft Windows, function pointer typedefs. Please report
    problems as <a
    href="https://github.com/KhronosGroup/OpenGL-Registry/issues/">Issues</a>
    in the <a href="index.php#repository">OpenGL-Registry</a> repository.
    </p>

<p> <a name="headerskhr"></a> <b> Khronos Shared Platform Header
    (<tt>&lt;KHR/khrplatform.h&gt;</tt>) </b> </p>

<ul>
<li> The OpenGL headers all depend on the shared
     <a href="https://www.khronos.org/registry/EGL/api/KHR/khrplatform.h">
     <tt>&lt;KHR/khrplatform.h&gt;</tt></a> header from the <a
     href="http://www.khronos.org/registry/EGL/"> EGL Registry </a>.
     This is a new dependency, introduced in
     <a href="https://github.com/KhronosGroup/OpenGL-Registry/pull/183">
     OpenGL-Registry pull request 183</a> for increased compatibility
     between OpenGL and OpenGL ES headers. </li>
</ul>


<hr>

<!-- Older Material -->

<h2> <a name="oldspecs"></a>
     Older OpenGL and GLX Specifications and Reference Pages
     </h2>

<ul>

    <!-- Does not link to diff/withchanges specs, yet -->
<li> <b> OpenGL 4.5 </b>
<li> <a href="specs/gl/glspec45.core.pdf"> (API Core Profile) </a> </li>
<li> <a href="specs/gl/glspec45.compatibility.pdf"> (API Compatibility Profile) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.4.50.pdf"> OpenGL Shading Language
     4.50 Specification </a> </li>

<li> <b> OpenGL 4.4 </b>
<li> <a href="specs/gl/glspec44.core.pdf"> (API Core Profile) </a> </li>
<li> <a href="specs/gl/glspec44.compatibility.pdf"> (API Compatibility Profile) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.4.40.pdf"> OpenGL Shading Language
     4.40 Specification </a> </li>

<li> <b> OpenGL 4.3 </b>
<li> <a href="specs/gl/glspec43.core.pdf"> (API Core Profile) </a> </li>
<li> <a href="specs/gl/glspec43.compatibility.pdf"> (API Compatibility Profile) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.4.30.pdf"> OpenGL Shading Language
     4.30 Specification </a> </li>

<li> <b> OpenGL 4.2 </b>
<li> <a href="specs/gl/glspec42.core.pdf"> (API Core Profile) </a> </li>
<li> <a href="specs/gl/glspec42.compatibility.pdf"> (API Compatibility Profile) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.4.20.pdf"> OpenGL Shading Language
     4.20 Specification </a> </li>

<li> <b> OpenGL 4.1 </b>
<li> <a href="specs/gl/glspec41.core.pdf"> (API Core Profile) </a> </li>
<li> <a href="specs/gl/glspec41.compatibility.pdf"> (API Compatibility Profile) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.4.10.pdf"> OpenGL Shading Language 4.10
     Specification </a> </li>

<li> <b> OpenGL 4.0 </b>
<li> <a href="specs/gl/glspec40.core.pdf"> (API Core Profile) </a> </li>
<li> A <a href="http://www.cutt.co.jp/book/978-4-87783-255-1.html"> Japanese
     translation </a> of the API core profile specification is also
     available. </li>
<li> <a href="specs/gl/glspec40.compatibility.pdf"> (API Compatibility Profile) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.4.00.pdf"> OpenGL Shading Language 4.00
     Specification </a> </li>

<li> <b> OpenGL 3.3 </b>
<li> <a href="specs/gl/glspec33.core.pdf"> (API Core Profile) </a> </li>
<li> <a href="specs/gl/glspec33.compatibility.pdf"> (API Compatibility Profile) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.3.30.pdf"> OpenGL Shading Language
     3.30 Specification </a> </li>

<li> <b> OpenGL 3.2 </b>
<li> <a href="specs/gl/glspec32.core.pdf"> (API Core Profile) </a> </li>
<li> <a href="specs/gl/glspec32.compatibility.pdf"> (API Compatibility Profile) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.1.50.pdf"> OpenGL Shading Language
     1.50 Specification </a> </li>

<li> <b> OpenGL 3.1 </b>
<li> <a href="specs/gl/glspec31.pdf"> (API Specification) </a> </li>
<li> <a href="specs/gl/glspec31undep.pdf">
        (with GL_ARB_compatibility extension) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.1.40.pdf"> OpenGL Shading Language
     1.40 Specification </a> </li>

<li> <b> OpenGL 3.0 </b>
<li> <a href="specs/gl/glspec30.pdf"> (API Specification) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.1.30.pdf"> OpenGL Shading Language
     1.30 Specification </a> </li>

<li> <b> OpenGL 2.1 </b>
<li> <a href="specs/gl/glspec21.pdf"> (API Specification) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.1.20.pdf"> OpenGL Shading Language
     1.20 Specification </a> </li>

<li> <b> OpenGL 2.1 Reference Pages </b>
<li> <a href="http://www.khronos.org/registry/OpenGL-Refpages/gl2.1/"> OpenGL 2.1 Reference
     Pages </a> </li>

<li> <b> OpenGL 2.0 </b>

<li> <a href="specs/gl/glspec20.pdf"> (API Specification) </a> </li>
<li> <a href="specs/gl/GLSLangSpec.1.10.pdf"> OpenGL Shading Language
     1.10 Specification </a> </li>

<li> <b> OpenGL 1.x </b>

<li> <a href="specs/gl/glspec15.pdf"> OpenGL 1.5 API Specification </a>
<li> <a href="specs/gl/glspec14.pdf"> OpenGL 1.4 API Specification </a>
<li> <a href="specs/gl/glspec13.pdf"> OpenGL 1.3 API Specification </a>
<li> <a href="specs/gl/glspec121.pdf"> OpenGL 1.2.1 API Specification </a>
<li> <a href="specs/gl/glspec11.pdf"> OpenGL 1.1 API Specification </a>
<li> <a href="specs/gl/glspec10.pdf"> OpenGL 1.0 API Specification </a>

<li> <b> Older GLX Specifications </b>

<li> <a href="specs/gl/glx1.3.pdf"> GLX 1.3 Specification </a>
<li> <a href="specs/gl/glxencode1.3.pdf"> GLX 1.3 Protocol Encoding
     Specification </a>
<li> <a href="specs/gl/glx1.2.ps"> GLX 1.2 Specification (PostScript format) </a>
<li> <a href="specs/gl/GLXprotocol.ps"> GLX Protocol Slides (PostScript
     format; only of historical interest) </a>

<li> <b> OpenGL Utility Library (GLU) Specification </b>
<li> <a href="specs/gl/glu1.3.pdf"> GLU 1.3 Specification (November 4, 1998) </a>

</ul>

<h2> <a name="abi"></a>
    OpenGL Application Binary Interface for Linux </h2>

<p> The <a href="ABI/">OpenGL Application Binary Interface for Linux</a> is
    also available. <b>NOTE:</b> this document is extremely old and of no
    relevance to modern Linux systems, where the ABI is de-facto defined by
    the <a href="https://docs.mesa3d.org/precompiled.html">Mesa
    libraries</a> as shipped by distribution vendors. </p>

<h2> <a name="arbextspecs"></a>
     ARB and KHR Extensions by number</h2>

<?php include("extensions/arbext.php"); ?>

<h2> <a name="otherextspecs"></a>
     Vendor and EXT Extensions by number</h2>

<?php include("extensions/glext.php"); ?>

<?php include_once("../../assets/static_pages/khr_page_bottom.php"); ?>
</body>
</html>
