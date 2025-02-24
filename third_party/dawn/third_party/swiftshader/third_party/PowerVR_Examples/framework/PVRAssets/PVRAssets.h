/*!
\brief Include this file if you wish to use the PVRAssets functionality.
\file PVRAssets/PVRAssets.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRAssets/Model.h"
#include "PVRAssets/fileio/PODReader.h"
#include "PVRAssets/fileio/GltfReader.h"
#include "PVRAssets/BoundingBox.h"
#include "PVRAssets/Geometry.h"
#include "PVRAssets/Helper.h"

/*****************************************************************************/
/*! \mainpage PVRAssets
******************************************************************************

\tableofcontents

\section overview Overview
*****************************

PVRAssets provides the necessary classes to work with assets and resources, like Model, Mesh, Texture, Effect, Animation, Camera and others. PVRAssets provides built-in support for
reading all PowerVR deployment formats (PVR textures, POD models and PFX effects) into these classes, usually with a single line of code. PVRAssets classes are very flexible and
can be adapted for a wide variety of uses.

The Model and Mesh classes along with Texture will very commonly be encountered in user code. The Model and Mesh codes are especially very well suited for extracting data to
simplify usual graphics tasks such as building Vertex Buffer Objects (VBOs), automating rendering, etc. PVRAssets is heavily used by PVRUtils and is additionally very commonly used
directly.

PVRAssets source can be found in the <a href="../../">PVRAssets</a> folder in the SDK package.

\section usage Using PVRAssets
*****************************

To use PVRAssets:
<ul>
  <li>Depending on the platform, add the module in by adding either:<br/>
	<ol>
	  <li>A project dependency (windows/macOS/ios/android/...) (project file in <span class="code">Framework/PVRCamera/Build/[Platform]/...</span>)</li>
	  <li>The library to link against directly (windows/android/linux makefiles) (.so etc. in <span class="code">Framework/Bin/[Platform]/[lib?]PVRCamera[.lib/.so]</span>)</li>
	</ol>
  </li>
  <li>Include the relevant header files (usually <span class="code">PVRAssets/PVRAssets.h</span>)</li>
  <li>Use the code (load .pod files into models, modify meshes, load .pvr textures, inspect texture metadata, extract attribute information from meshes, etc.)</li>
</ul>

\section code Code Examples
*****************************

\code
pvr::assets::PODReader podReader(myPODFileStream);
pvr::assets::Model::createWithReader(podReader, mySceneSmartPointer);
\endcode

\code
pvr:::assets::WaveFrontReader objReader(myObjFileStream);
pvr::assets::Model::createWithReader(objReader, mySceneSmartPointer);
\endcode
*/
