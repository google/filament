<?php
$static_title = 'Khronos OpenGL SC Registry';

include_once("../../assets/static_pages/khr_page_top.php");
?>

<p> The OpenGL SC Registry contains specifications of the core API;
    specifications of Khronos- and vendor-approved OpenGL SC extensions;
    header files corresponding to the specifications; and related
    documentation. </p>

<p> The OpenGL SC Registry is part of the combined <a
    href="http://www.khronos.org/registry/OpenGL/"> Combined OpenGL Registry </a>
    for OpenGL, OpenGL ES, and OpenGL SC, which includes the <a
    href="xml/README.adoc">XML API registry</a> of reserved enumerants and
    functions. </p>

<h2> OpenGL SC Core API Specification, Headers, and Documentation </h2>

<p> The current version of OpenGL SC is OpenGL SC 2.0.1 </p>

<ul>
<li> <b><a href="docs/update_policy.php">Working Group Policy</a></b> for
     when Specifications and extensions will be updated. </li>
<li> OpenGL SC 2.0.1
     <a href="specs/sc/sc_spec_2.0.1.pdf">
     Full Specification </a> (July 24, 2019). </li>
<li> <tt><a href="api/GLSC2/glsc2.h"> &lt;GLSC2/glsc2.h&gt; </a></tt>
     OpenGL SC 2.0 Header File. </li>
<li> <tt><a href="api/GLSC2/glsc2ext.h"> &lt;GLSC2/glsc2ext.h&gt; </a></tt>
     OpenGL SC 2.0 Extension Header File. </li>
<li> <tt><a href="api/GLSC2/gl2platform.h"> &lt;GLSC2/gl2platform.h&gt; </a></tt>
     OpenGL SC 2.0 Platform-Dependent Macros. </li>
<li> The headers depend on the shared <a
     href="https://www.khronos.org/registry/EGL/api/KHR/khrplatform.h">
     <tt>&lt;KHR/khrplatform.h&gt;</tt></a> header located in the
     <a href="http://www.khronos.org/registry/EGL/"> EGL Registry </a>.
<li> The headers are generated from the <a href="index.php#repository">
     OpenGL-Registry </a> github repository.
<li> <b> Extension Specifications </b>
    <ul>
    <li> <a href="#otherextspecs">OpenGL SC 2.0 Extension Specifications</a> </li>
    </ul> </li>
<li> <a href="https://www.khronos.org/developers/reference-cards">
     OpenGL SC 2.0 Quick Reference Card. </a> </li>
</ul>

<h2> Older versions of OpenGL SC are also available </h2>

<p> OpenGL SC 2.0 </p>

<ul>
<li> OpenGL SC 2.0
     <a href="specs/sc/sc_spec_2.0.pdf">
     Full Specification </a> (April 19, 2016). </li>
</ul>

<p> OpenGL SC 1.0.1 </p>

<ul>
<li> OpenGL SC 1.0.1
     <a href="specs/sc/sc_spec_1_0_1.pdf">
     Difference Specification </a> (March 12, 2009). </li>
<li> <a href="api/GLSC/1.0.1/gl.h"> gl.h </a> -
     OpenGL SC 1.0.1 Header File (March 16, 2009). </li>
<li> <a href="specs/sc/es_sc_philosophy.pdf"> OpenGL SC Philosophy </a>
     (June 6, 2005). </li>
</ul>

<p> OpenGL SC 1.0 </p>

<ul>
<li> OpenGL SC 1.0
     <a href="specs/sc/opengles_sc_spec_1_0.pdf">
     Difference Specification </a> (June 6, 2005). </li>
<li> <a href="api/GLSC/1.0/gl.h"> gl.h </a> -
     OpenGL SC 1.0 Header File. </li>
<li> <a href="specs/sc/es_sc_philosophy.pdf"> OpenGL SC Philosophy </a>
     (June 6, 2005). </li>
</ul>

<h2> <a name="otherextspecs"></a>
   Extension Specifications by number</h2>

<?php include("extensions/scext.php"); ?>

<?php include_once("../../assets/static_pages/khr_page_bottom.php"); ?>
</body>
</html>
