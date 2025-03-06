<?php
$static_title = 'Khronos Combined OpenGL Registry';

include_once("../../assets/static_pages/khr_page_top.php");
?>

<p> This site contains the API and Extension registries for the OpenGL
    family APIs - OpenGL, OpenGL ES, and OpenGL SC. It includes API
    specifications; specifications of Khronos- and vendor-approved
    extensions; header files corresponding to the specifications; the XML
    API Registry definining each API; and related tools and scripts. </p>

<p> Each API has its own index page linking to the files specific to that
API: </p>

<ul>
<li> <a href="index_es.php">OpenGL ES Registry Index</a> </li>
<li> <a href="index_gl.php">OpenGL Registry Index</a> </li>
<li> <a href="index_sc.php">OpenGL SC Registry Index</a> </li>
</ul>

<h2> <a name="repository"></a>
     OpenGL-Registry Repository </h2>

<p> The web registry is backed by a <a
    href="https://github.com/KhronosGroup/OpenGL-Registry"> github
    repository</a>. Changes committed to the <b>main</b> branch of the
    repository are reflected on the website. The repository includes
    everything visible on the registry website - specifications, extensions,
    headers, XML, and the index pages such as this one - as well as
    additional documentation and scripts. Problems with the registry or with
    the underlying specifications may be reported as github Issues. </b>

<p> In particular, the registry repository includes formal XML documents
    defining the APIs and enumerants used in OpenGL, OpenGL ES, OpenGL SC,
    GLX, and WGL. Changes to these files can be proposed to reserved
    enumerant ranges to vendors for future use, as well as to add interfaces
    for new extension APIs. A set of Python scripts are provided to load the
    XML files and process them into header files. </p>

<p> See <a href="xml/README.adoc"> xml/README.adoc </a> for more details of the
    API XML registry, how to obtain enumerant allocations, create extension
    specifications and register them, and other related topics.

<?php include_once("../../assets/static_pages/khr_page_bottom.php"); ?>
</body>
</html>
