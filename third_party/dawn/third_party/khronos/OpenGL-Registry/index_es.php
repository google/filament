<?php
$static_title = 'Khronos OpenGL ES Registry';

include_once("../../assets/static_pages/khr_page_top.php");
?>

<p> The OpenGL ES Registry contains specifications of the core API and
    shading language; specifications of Khronos- and vendor-approved
    OpenGL ES extensions; header files corresponding to the
    specifications; and related documentation. </p>

<p> The OpenGL ES Registry is part of the <a
    href="http://www.khronos.org/registry/OpenGL/"> Combined OpenGL Registry </a>
    for OpenGL, OpenGL ES, and OpenGL SC, which includes the <a
    href="xml/README.adoc">XML API registry</a> of reserved enumerants and
    functions. </p>

<p> <b>Table of Contents</b>
<ul>
<li> <b><a href="docs/update_policy.php">Working Group Policy</a></b> for
     when Specifications and extensions will be updated. </li>
<li> <b><a href="#specs">Current OpenGL ES API and Shading Language
     Specifications and Reference Pages</a></b>
     <ul>
     <li> <a href="#specs32">OpenGL ES 3.2 Specifications</a> </li>
     <li> <a href="#specs31">OpenGL ES 3.1 Specifications</a> </li>
     <li> <a href="#specs3">OpenGL ES 3.0 Specifications</a> </li>
     <li> <a href="#specs2">OpenGL ES 2.0 Specifications</a> </li>
     <li> <a href="#specs11">OpenGL ES 1.1 Specifications</a> </li>
     <li> <a href="#specs10">OpenGL ES 1.0 Specification</a> </li>
     </ul> </li>
<li> <b><a href="#headers">Core API and Extension Header Files</a></b>
     <ul>
     <li> <a href="#headers32">OpenGL ES 3.2 Header Files</a> </li>
     <li> <a href="#headers31">OpenGL ES 3.1 Header Files</a> </li>
     <li> <a href="#headers3">OpenGL ES 3.0 Header Files</a> </li>
     <li> <a href="#headers2">OpenGL ES 2.0 Header Files</a> </li>
     <li> <a href="#headers1">OpenGL ES 1.1 Header Files</a> </li>
     <li> <a href="#headerskhr">Shared Khronos Header File</a> </li>
     </ul> </li>
<li> <b><a href="#otherextspecs">OpenGL ES Extension Specifications</a></b>
<li> <b> <a href="https://www.khronos.org/files/ip-disclosures/opengl/">
     IP Disclosures</a> Potentially Affecting OpenGL ES Implementations
     </b></li>
</ul>

<hr>

<h2> <a name="specs"></a> OpenGL ES Core API and Shading Language
     Specifications and Reference Pages </h2>

<p> The current version of OpenGL ES is OpenGL ES 3.2. Specifications for
    older versions 3.1, 3.0, 2.0, 1.1, and 1.0 are also available below. For
    additional specifications, headers, and documentation not listed below,
    see the <a href="http://www.khronos.org/developers/specs/">Khronos.org
    Developer Pages</a>. Header files not labelled with a revision date
    include their last update time in comments near the top of the file.
    </p>

<h2> <a name="specs32"></a> OpenGL ES 3.2 Specifications and
     Documentation </h2>

<ul>
<li> OpenGL ES 3.2 Specification (May 5, 2022)
     <a href="specs/es/3.2/es_spec_3.2.pdf"> without changes marked </a>
     and
     <a href="specs/es/3.2/es_spec_3.2.withchanges.pdf"> with changes marked </a>. </li>
<li> OpenGL ES Shading Language 3.20 Specification (July 10, 2019)
     <a href="specs/es/3.2/GLSL_ES_Specification_3.20.html"> (HTML) </a>
     <a href="specs/es/3.2/GLSL_ES_Specification_3.20.pdf"> (PDF) </a>
<li> <a href="http://www.khronos.org/registry/OpenGL-Refpages/es3/">
     OpenGL ES 3.2 Online Reference Pages.</a> </li>
<li> <a href="https://www.khronos.org/developers/reference-cards">
     OpenGL ES Quick Reference Card </a> (available for different API
     versions). </li>
</ul>

<h2> <a name="specs31"></a> OpenGL ES 3.1 Specifications and
     Documentation </h2>

<ul>
<li> OpenGL ES 3.1 Specification (November 3, 2016),
     <a href="specs/es/3.1/es_spec_3.1.pdf"> without changes marked </a>
     and
     <a href="specs/es/3.1/es_spec_3.1.withchanges.pdf"> with changes marked </a>. </li>
<li> OpenGL ES Shading Language 3.10 Specification (January 29, 2016)
     <a href="specs/es/3.1/GLSL_ES_Specification_3.10.pdf"> without changes marked </a>
     and
     <a href="specs/es/3.1/GLSL_ES_Specification_3.10.withchanges.pdf"> with changes marked </a>. </li>
<li> <a href="http://www.khronos.org/registry/OpenGL-Refpages/es3.1/">
     OpenGL ES 3.1 Online Reference Pages.</a> </li>
</ul>

<h2> <a name="specs3"></a> OpenGL ES 3.0 Specifications and
     Documentation </h2>

<ul>
<li> OpenGL ES 3.0.6 Specification (November 1, 2019),
     <a href="specs/es/3.0/es_spec_3.0.pdf"> without changes marked </a>
     and
     <a href="specs/es/3.0/es_spec_3.0.withchanges.pdf"> with changes marked </a>. </li>
<li> OpenGL ES Shading Language 3.00
     <a href="specs/es/3.0/GLSL_ES_Specification_3.00.pdf">
     Specification </a> (January 29, 2016). </li>
<li> <a href="http://www.khronos.org/registry/OpenGL-Refpages/es3.0/">
     OpenGL ES 3.0 Online Reference Pages.</a> </li>
</ul>

<h2> <a name="specs2"></a> OpenGL ES 2.0 Specifications and
     Documentation </h2>

<ul>
<li> OpenGL ES 2.0
     <a href="specs/es/2.0/es_full_spec_2.0.pdf">
     Full Specification </a>,
     <a href="specs/es/2.0/es_full_spec_2.0.withchanges.pdf">
     Full Specification with changes marked</a>,
     <a href="specs/es/2.0/es_cm_spec_2.0.pdf">
     Difference Specification </a> (November 2, 2010).
     A
     <a href="http://www.cutt.co.jp/book/978-4-87783-267-4.html">
     Japanese translation </a> of the specification is also available.
     </li>
<li> OpenGL ES Shading Language 1.00
     <a href="specs/es/2.0/GLSL_ES_Specification_1.00.pdf">
     Specification </a> (May 12, 2009). </li>
<li> <a href="http://www.khronos.org/registry/OpenGL-Refpages/es2.0/">
     OpenGL ES 2.0 Online Reference Pages.</a> </li>
</ul>


<h2> <a name="specs11"></a> OpenGL ES 1.1 Specifications and
     Documentation </h2>

<ul>
<li> OpenGL ES 1.1
     <a href="specs/es/1.1/es_full_spec_1.1.pdf"> Full Specification </a>
     and
     <a href="specs/es/1.1/es_cm_spec_1.1.pdf"> Difference Specification </a>
     (April 24, 2008). </li>
<li> <a href="specs/es/1.1/opengles_spec_1_1_extension_pack.pdf"> OpenGL ES
     1.1.03 Extension Pack </a> (July 19, 2005). </li>
<li> <a href="http://www.khronos.org/registry/OpenGL-Refpages/es1.1/">
     OpenGL ES 1.1 Online Reference Pages.</a> </li>
</ul>

<h2> <a name="specs10"></a> OpenGL ES 1.0 Specification and
     Documentation </h2>

<ul>
<li> <a href="specs/es/1.0/opengles_spec_1_0.pdf"> OpenGL ES 1.0.02
     Specification </a>. </li>
<li> <tt><a href="api/GLES/1.0/gl.h"> gl.h </a></tt> for OpenGL ES 1.0. </li>
<li> The old <i>OpenGL ES 1.0 and EGL 1.0 Reference Manual</i> is
     obsolete and has been removed from the Registry. Please use the
     <a href="http://www.khronos.org/registry/OpenGL-Refpages/es1.1/">
     OpenGL ES 1.1 Online Reference Pages</a> instead. </li>
</ul>

<hr>

<h2> <a name="headers"></a> API and Extension Header Files </h2>

<p> Because extensions vary from platform to platform and driver to driver,
    OpenGL ES segregates headers for each API version into a header for the
    core API (OpenGL ES 1.0, 1.1, 2.0, 3.0, 3.1 and 3.2) and a separate
    header defining extension interfaces for that core API. These header
    files are supplied here for developers and platform vendors. They define
    interfaces including enumerants, prototypes, and for platforms
    supporting dynamic runtime extension queries, such as Linux and
    Microsoft Windows, function pointer typedefs. Please report problems as
    <a
    href="https://github.com/KhronosGroup/OpenGL-Registry/issues/">Issues</a>
    in the <a href="index.php#repository">OpenGL-Registry</a> repository.
    </p>

<p> In addition to the core API and extension headers, there is also an
    OpenGL ES version-specific platform header file intended to define
    calling conventions and data types specific to a platform. </p>

<p> Almost all of the headers described below depend on a platform header
    file common to multiple Khronos APIs called
    <tt>&lt;KHR/khrplatform.h&gt;</tt>. </p>

<p> Vendors may include modified versions of any or all of these headers
    with their OpenGL ES implementations, but in general only the
    platform-specific OpenGL ES and Khronos headers are likely to be
    modified by the implementation. This makes it possible for
    developers to drop in more recently updated versions of the headers
    obtained here, typically when new extensions are supplied
    on a platform. </p>

<p> <a name="headers32"></a> <b> OpenGL ES 3.2 Headers </b> </p>

<ul>
<li> <tt><a href="api/GLES3/gl32.h"> &lt;GLES3/gl32.h&gt; </a></tt>
     OpenGL ES 3.2 Header File. </li>
<li> <tt><a href="api/GLES2/gl2ext.h"> &lt;GLES2/gl2ext.h&gt; </a></tt>
     OpenGL ES Extension Header File (this header is defined to contain
     all defined extension interfaces for OpenGL ES 2.0 and all later
     versions, since later versions are backwards-compatible with OpenGL
     ES 2.0).
     </li>
<li> <tt><a href="api/GLES3/gl3platform.h"> &lt;GLES3/gl3platform.h&gt; </a></tt>
     OpenGL ES 3.2 Platform-Dependent Macros (this header is shared with
     OpenGL ES 3.0 and 3.1). </li>
</ul>

<p> <a name="headers31"></a> <b> OpenGL ES 3.1 Headers </b> </p>

<ul>
<li> <tt><a href="api/GLES3/gl31.h"> &lt;GLES3/gl31.h&gt; </a></tt>
     OpenGL ES 3.1 Header File. </li>
<li> <tt><a href="api/GLES2/gl2ext.h"> &lt;GLES2/gl2ext.h&gt; </a></tt>
     OpenGL ES Extension Header File. </li>
<li> <tt><a href="api/GLES3/gl3platform.h"> &lt;GLES3/gl3platform.h&gt; </a></tt>
     OpenGL ES 3.1 Platform-Dependent Macros (this header is shared with
     OpenGL ES 3.0). </li>
</ul>

<p> <a name="headers3"></a> <b> OpenGL ES 3.0 Headers </b> </p>

<ul>
<li> <tt><a href="api/GLES3/gl3.h"> &lt;GLES3/gl3.h&gt; </a></tt>
     OpenGL ES 3.0 Header File. </li>
<li> <tt><a href="api/GLES2/gl2ext.h"> &lt;GLES2/gl2ext.h&gt; </a></tt>
     OpenGL ES Extension Header File. </li>
<li> <tt><a href="api/GLES3/gl3platform.h"> &lt;GLES3/gl3platform.h&gt; </a></tt>
     OpenGL ES 3.0 Platform-Dependent Macros. </li>
</ul>

<p> <a name="headers2"></a> <b> OpenGL ES 2.0 Headers </b> </p>

<ul>
<li> <tt><a href="api/GLES2/gl2.h"> &lt;GLES2/gl2.h&gt; </a></tt>
     OpenGL ES 2.0 Header File. </li>
<li> <tt><a href="api/GLES2/gl2ext.h"> &lt;GLES2/gl2ext.h&gt; </a></tt>
     OpenGL ES Extension Header File. </li>
<li> <tt><a href="api/GLES2/gl2platform.h"> &lt;GLES2/gl2platform.h&gt; </a></tt>
     OpenGL ES 2.0 Platform-Dependent Macros. </li>
</ul>

<p> <a name="headers1"></a> <b> OpenGL ES 1.1 Headers </b> </p>

<ul>
<li> <tt><a href="api/GLES/gl.h"> &lt;GLES/gl.h&gt; </a></tt>
     OpenGL ES 1.1 Header File. </li>
<li> <tt><a href="api/GLES/glext.h"> &lt;GLES/glext.h&gt; </a></tt>
     OpenGL ES 1.1 Extension Header File. </li>
<li> <tt><a href="api/GLES/glplatform.h"> &lt;GLES/glplatform.h&gt; </a></tt>
     OpenGL ES 1.1 Platform-Dependent Macros. </li>
<li> <tt><a href="api/GLES/egl.h"> &lt;GLES/egl.h&gt; </a></tt>
     EGL Legacy Header File for OpenGL ES 1.1 (August 6, 2008) - requires
     <a href="https://www.khronos.org/registry/EGL/api/EGL/egl.h">
     <tt>&lt;EGL/egl.h&gt;</tt></a> from the
     <a href="http://www.khronos.org/registry/EGL/"> EGL Registry </a>.
     </li>
</ul>

<p> <a name="headerskhr"></a> <b> Khronos Shared Platform Header
    (<tt>&lt;KHR/khrplatform.h&gt;</tt>) </b> </p>

<ul>
<li> The OpenGL ES 3.0, 2.0, and 1.1 headers all depend on the shared
     <a href="https://www.khronos.org/registry/EGL/api/KHR/khrplatform.h">
     <tt>&lt;KHR/khrplatform.h&gt;</tt></a> header from the
     <a href="http://www.khronos.org/registry/EGL/"> EGL Registry </a>.
</ul>

<hr>

<h2> <a name="otherextspecs"></a>
     Extension Specifications by number</h2>

<?php include("extensions/esext.php"); ?>

<?php include_once("../../assets/static_pages/khr_page_bottom.php"); ?>
</body>
</html>
