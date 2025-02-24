<?php
$static_title = 'Khronos OpenGL Specification Update Policy';

include_once("../../../assets/static_pages/khr_page_top.php");
?>

<p> This page describes policy of the Khronos OpenGL Working Group for
    updating API and Shading Language Specification documents, as well as
    extension documents. </p>

<p> This policy was last updated on 2019-03-13. </p>

<h2> Specification Updates </h2>

<p> The current core (OpenGL 4.6, OpenGL ES 3.2, OpenGL SC 2.0, GLSL 4.60,
    GLSL ES 3.20, and GLX 1.4) documents are updated in response to issues
    and bugs the OpenGL Working Group considers priorities. The update
    frequency is irregular and dependent on the accumulated backlog of
    issues, up to several times per year. </p>

<p> While Khronos publishes older (non-current) Specifications such as
    OpenGL 4.5 or OpenGL ES 3.0, such Specifications are usually <b>not</b>
    updated in response to issues and bugs unless there's a compelling
    reason. One such compelling reason has been to update older OpenGL ES
    Specifications on which WebGL is based, in response to requests from the
    WebGL Working Group. </p>

<h2> Extension Updates </h2>

<p> Khronos-approved (<tt>ARB</tt>, <tt>OES</tt>, or <tt>KHR</tt> vendor
    suffixed) extensions are usually updated when corresponding changes are
    made to a core Specification, but with some time lag. </p>

<p> This is a recent policy. Older bug fixes to core Specifications have not
    always been back-ported to the corresponding extensions, and we are
    unlikely to perform this exercise for closed issues and bugs without
    compelling reason. </p>

<p> We will tag each Khronos-approved extension with a link to this policy
    document and a recommendation to refer to the current core Specification
    for the latest wording. </p>

<?php include_once("../../../assets/static_pages/khr_page_bottom.php"); ?>
</body>
</html>
