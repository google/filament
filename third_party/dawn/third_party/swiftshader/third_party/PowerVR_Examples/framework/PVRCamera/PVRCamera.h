/*!
\brief Include this file if you wish to use the PVRCamera functionality
\file PVRCamera/PVRCamera.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRCamera/CameraInterface.h"

/*****************************************************************************/
/*! \mainpage PVRCamera
******************************************************************************

\tableofcontents
 

\section overview Overview
*****************************
PVRCamera is unique among the PowerVR Framework modules in that it does not only contain native code. PVRCamera provides an easy-to-use interface between the platform's Camera of Android or iOS and the rest of the Framework. Please refer to the relevant examples (e.g. TextureStreaming) for its use.

Except for linking to the native library, PVRCamera requires a few lines of Java code to be added (for Android applications), and to be available for linking.

PVRCamera source can be found in the <a href="../../">PVRCamera</a> folder in the SDK package.

\section usage Using PVRCamera
*****************************

To use PVRCamera:
<ul>
<li>
	<li>Depending on the platform, add the module in by adding either:</li>
	<li>
		<ol>
			<li>A project dependency (windows/macOS/ios/android/...) (project file in <span class="code">Framework/PVRCamera/Build/[Platform]/...</span>)</li>
			<li>The library to link against directly (windows/android/linux makefiles) (.so etc. in <span class="code">Framework/Bin/[Platform]/[lib?]PVRCamera[.lib/.so]</span>)</li>
		</ol>
	</li>
	<li>Include the header file (most commonly this file, <span class="code">PVRCamera/PVRCamera.h</span>)</li>
<li>Create and use a pvr::CameraInterface class. For environments without a built-in camera, the camerainterface will provide a dummy static image</li>
</ul>
*/