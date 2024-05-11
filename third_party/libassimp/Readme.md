Open Asset Import Library (assimp)
==================================
A library to import and export various 3d-model-formats including scene-post-processing to generate missing render data.
### Current project status ###
[![Linux Build Status](https://travis-ci.org/assimp/assimp.svg)](https://travis-ci.org/assimp/assimp)
[![Windows Build Status](https://ci.appveyor.com/api/projects/status/tmo433wax6u6cjp4?svg=true)](https://ci.appveyor.com/project/kimkulling/assimp)
<a href="https://scan.coverity.com/projects/5607">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/5607/badge.svg"/>
</a>
[![Coverage Status](https://coveralls.io/repos/github/assimp/assimp/badge.svg?branch=master)](https://coveralls.io/github/assimp/assimp?branch=master)
[![Join the chat at https://gitter.im/assimp/assimp](https://badges.gitter.im/assimp/assimp.svg)](https://gitter.im/assimp/assimp?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Average time to resolve an issue](http://isitmaintained.com/badge/resolution/assimp/assimp.svg)](http://isitmaintained.com/project/assimp/assimp "Average time to resolve an issue")
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/5be56faac64f46fc941ac890fb4febef)](https://www.codacy.com/app/kimkulling/assimp?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=assimp/assimp&amp;utm_campaign=Badge_Grade)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/assimp/assimp.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/assimp/assimp/alerts/)
<br>

APIs are provided for C and C++. There are various bindings to other languages (C#, Java, Python, Delphi, D). Assimp also runs on Android and iOS.

[Check the latest doc](https://assimp-docs.readthedocs.io/en/latest/).

Additionally, assimp features various __mesh post processing tools__: normals and tangent space generation, triangulation, vertex cache locality optimization, removal of degenerate primitives and duplicate vertices, sorting by primitive type, merging of redundant materials and many more.

This is the development repo containing the latest features and bugfixes. For productive use though, we recommend one of the stable releases available from [Github Assimp Releases](https://github.com/assimp/assimp/releases).

Monthly donations via Patreon:
<br>[![Patreon](https://cloud.githubusercontent.com/assets/8225057/5990484/70413560-a9ab-11e4-8942-1a63607c0b00.png)](http://www.patreon.com/assimp)

<br>

One-off donations via PayPal:
<br>[![PayPal](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=4JRJVPXC4QJM4)

<br>

Please check our Wiki as well: https://github.com/assimp/assimp/wiki

If you want to check our Model-Database, use the following repo: https://github.com/assimp/assimp-mdb

#### Supported file formats ####

__Importers__:

- 3D
- [3DS](https://en.wikipedia.org/wiki/.3ds)
- [3MF](https://en.wikipedia.org/wiki/3D_Manufacturing_Format)
- AC
- [AC3D](https://en.wikipedia.org/wiki/AC3D)
- ACC
- AMJ
- ASE
- ASK
- B3D
- [BLEND](https://en.wikipedia.org/wiki/.blend_(file_format))
- [BVH](https://en.wikipedia.org/wiki/Biovision_Hierarchy)
- CMS
- COB
- [DAE/Collada](https://en.wikipedia.org/wiki/COLLADA)
- [DXF](https://en.wikipedia.org/wiki/AutoCAD_DXF)
- ENFF
- [FBX](https://en.wikipedia.org/wiki/FBX)
- [glTF 1.0](https://en.wikipedia.org/wiki/GlTF#glTF_1.0) + GLB
- [glTF 2.0](https://en.wikipedia.org/wiki/GlTF#glTF_2.0)
- HMB
- IFC-STEP
- IRR / IRRMESH
- [LWO](https://en.wikipedia.org/wiki/LightWave_3D)
- LWS
- LXO
- MD2
- MD3
- MD5
- MDC
- MDL
- MESH / MESH.XML
- MOT
- MS3D
- NDO
- NFF
- [OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file)
- [OFF](https://en.wikipedia.org/wiki/OFF_(file_format))
- [OGEX](https://en.wikipedia.org/wiki/Open_Game_Engine_Exchange)
- [PLY](https://en.wikipedia.org/wiki/PLY_(file_format))
- PMX
- PRJ
- Q3O
- Q3S
- RAW
- SCN
- SIB
- SMD
- [STP](https://en.wikipedia.org/wiki/ISO_10303-21)
- [STL](https://en.wikipedia.org/wiki/STL_(file_format))
- TER
- UC
- VTA
- X
- [X3D](https://en.wikipedia.org/wiki/X3D)
- XGL
- ZGL

Additionally, some formats are supported by dependency on non-free code or external SDKs (not built by default):

- [C4D](https://en.wikipedia.org/wiki/Cinema_4D) (https://github.com/assimp/assimp/wiki/Cinema4D-&-Melange) IMporting geometry + node hierarchy are currently supported

__Exporters__:

- DAE (Collada)
- STL
- OBJ
- PLY
- X
- 3DS
- JSON (for WebGl, via https://github.com/acgessler/assimp2json)
- ASSBIN
- STEP
- glTF 1.0 (partial)
- glTF 2.0 (partial)
- 3MF ( experimental )
- FBX ( experimental )

### Building ###
Take a look into the https://github.com/assimp/assimp/blob/master/Build.md file. Our build system is CMake, if you used CMake before there is a good chance you know what to do.

### Ports ###
* [Android](port/AndroidJNI/README.md)
* [Python](port/PyAssimp/README.md)
* [.NET](https://github.com/kebby/assimp-net)
* [Pascal](port/AssimpPascal/Readme.md)
* [Javascript (Alpha)](https://github.com/makc/assimp2json)
* [Unity 3d Plugin](https://www.assetstore.unity3d.com/en/#!/content/91777)
* [JVM](https://github.com/kotlin-graphics/assimp) Full jvm port (current [status](https://github.com/kotlin-graphics/assimp/wiki/Status))
* [HAXE-Port](https://github.com/longde123/assimp-haxe) The Assimp-HAXE-port.

### Other tools ###
[open3mod](https://github.com/acgessler/open3mod) is a powerful 3D model viewer based on Assimp's import and export abilities.

#### Repository structure ####
Open Asset Import Library is implemented in C++. The directory structure looks like:

	/code		Source code
	/contrib	Third-party libraries
	/doc		Documentation (doxysource and pre-compiled docs)
	/include	Public header C and C++ header files
	/scripts 	Scripts used to generate the loading code for some formats
	/port		Ports to other languages and scripts to maintain those.
	/test		Unit- and regression tests, test suite of models
	/tools		Tools (old assimp viewer, command line `assimp`)
	/samples	A small number of samples to illustrate possible
                        use cases for Assimp

The source code is organized in the following way:

	code/Common		The base implementation for importers and the infrastructure
	code/PostProcessing	The post-processing steps
	code/<FormatName>	Implementation for import and export for the format

### Where to get help ###
For more information, visit [our website](http://assimp.org/). Or check out the `./doc`- folder, which contains the official documentation in HTML format.
(CHMs for Windows are included in some release packages and should be located right here in the root folder).

If the docs don't solve your problem, ask on [StackOverflow](http://stackoverflow.com/questions/tagged/assimp?sort=newest). If you think you found a bug, please open an issue on Github.

For development discussions, there is also a (very low-volume) mailing list, _assimp-discussions_
  [(subscribe here)]( https://lists.sourceforge.net/lists/listinfo/assimp-discussions)

Open Asset Import Library is a library to load various 3d file formats into a shared, in-memory format. It supports more than __40 file formats__ for import and a growing selection of file formats for export.

And we also have a Gitter-channel:Gitter [![Join the chat at https://gitter.im/assimp/assimp](https://badges.gitter.im/assimp/assimp.svg)](https://gitter.im/assimp/assimp?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)<br>

### Contributing ###
Contributions to assimp are highly appreciated. The easiest way to get involved is to submit
a pull request with your changes against the main repository's `master` branch.

### License ###
Our license is based on the modified, __3-clause BSD__-License.

An _informal_ summary is: do whatever you want, but include Assimp's license text with your product -
and don't sue us if our code doesn't work. Note that, unlike LGPLed code, you may link statically to Assimp.
For the legal details, see the `LICENSE` file.

### Why this name ###
Sorry, we're germans :-), no english native speakers ...
