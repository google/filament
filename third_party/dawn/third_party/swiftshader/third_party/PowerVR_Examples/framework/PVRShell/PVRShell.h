/*!
\brief Include this file if you wish to use the PVRShell functionality.
\file PVRShell/PVRShell.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRShell/Shell.h"

#if defined(__QNXNTO__)
#if not(defined(Screen) || defined(NullWS))
#error Please define a valid window system to compile PVRShell for QNX - Supported window systems are NullWS or Screen. Please pass the desired window system using -DPVR_WINDOW_SYSTEM=[NullWS,Screen].
#endif
#elif defined(__linux__)
#if not defined(__ANDROID__)
#if not(defined(X11) || defined(XCB) || defined(Wayland) || defined(NullWS))
#error Please define a valid window system to compile PVRShell for Linux - Supported window systems are X11, XCB, Wayland, or NullWS. Please pass the desired window system using -DPVR_WINDOW_SYSTEM=[NullWS,X11,XCB,Wayland].
#endif
#endif
#endif

/*****************************************************************************/
/*! \mainpage PVRShell
******************************************************************************

\tableofcontents

\section overview Overview
*****************************

PVRShell will usually be the foundation on top of which an application is written. This library abstracts the system and contains, among others, the application's entry point
(main(), android_main() or other, depending on the platform), command line arguments, events, main loop, etc., effectively abstracting the native platform.

Also, PVRShell will create and teardown the window, initialize and de-initialize the graphics system, swap the buffers at the end of every frame, search platform-specific methods
and file system storage (file system, assets, bundle, etc.).

PVRShell directly depends on PVRCore. Normally, PVRShell is used as the first step of every PowerVR SDK application.

PVRShell source can be found in the <a href="../../">PVRShell</a> folder in the SDK package.

\section usage Using PVRShell
*****************************

To use PVRShell:
<ul>
	<li>Depending on the platform, add the module in by adding either:<br/>
		<ul>
			<li>A project dependency (windows/macOS/ios/android/...) (project file in <span class="code">Framework/PVRShell/Build/[Platform]/...</span>)</li>
			<li>The library to link against directly (windows/android/linux makefiles) (.so etc. in <span
class="code">Framework/Bin/[Platform]/[lib?]PVRShell[.lib/.so]</span>)</li>
		</ul>
	</li>
	<li>In your main code file, include <span class="code">PVRShell/PVRShell.h</span></li>
	<li>Write a function <span class="code">pvr::newDemo()</span> that returns a <span class="code">std::unique_ptr</span> to a created instance of your class (refer to the bottom
of any example)</li> <li>Implement the pure virtual functions of <span class="code">pvr::Shell</span> in your own class (<span class="code">initApplication</span>, <span
class="code">initView</span>, <span class="code">renderFrame</span>, <span class="code">releaseView</span>, <span class="code">quitApplication</span>)</li> <li>Write an application
class inheriting from <span class="code">pvr::Shell</span></li> <li>Implement a simple function <span class="code">pvr::newDemo()</span> returning an <span
class="code">unique_ptr</span> to an instance of your application class</li>
</ul>
<ul>
<li>Implement <span class="code">pvr::Shell::initApplication()</span></li>
<p>This is the first initialization step. It is called once, after PVRShell is initialized, but before the Device/Graphics Context have been initialized. Initialization steps that
do not require a graphical context can go here (Model file loading, etc.). Any preferences that will influence context creation should be here.</p>
</ul>
<ul>
<li>Implement <span class="code">pvr::Shell::initView()</span></li>
<p>This is called right after a Device/Graphics Context is created and bound. Additionally, if the context is lost, this function is called when it is recreated. Implement any
initialization operations that would require a graphics context initialized (texture loading, buffer creation etc.).</p>
</ul>
<ul>
<li>Implement <span class="code">pvr::Shell::releaseView()</span></li>
<p><span class="code">releaseView()</span> is called just before a Graphics Context is released at teardown, or if it is lost (for example, minimizing the application on Android).
Release API objects (textures, buffers) should be here.</p>
</ul>
<ul>
<li>Implement <span class="code">pvr::Shell::quitApplication()</span></li>
<p>This is called just before application teardown, after the context is lost. Release any left-over non-API resources here.</p>
</ul>
<ul>
<li>Implement <span class="code">pvr::Shell::renderFrame()</span></li>
<p><span class="code">renderFrame()</span> is called every frame and is intended to contain the logic and actual drawing code. Backbuffer swapping is done automatically.</p>
</ul>
<ul>
<li>(Recommended, optional step) Implement input event handling</li>
<p>Override either <span class="code">pvr::Shell::eventMappedInput(...)</span> and/or any of the device-specific input functions (<span class="code">pvr::Shell::onKeyDown</span>,
<span class="code">onKeyUp</span>, <span class="code">onPointingDeviceDown</span>, etc.).</p> <p><span class="code">eventMappedInput()</span> is a call-back provided by PVRShell to
handle simplified input events unified across different platforms (for example, both a swipe left of a touchscreen, and the left arrow on a keyboard would map to <span
class="code">MappedInput::Left</span>).</p>
</ul>
<ul>
<li>The application entry point is provided by PVRShell library. Hence, application start, context creation, initialization, etc., will all be done for you.</li>
</ul>

\section code Code Examples
*****************************

\code
//The five callbacks house the application
void MyApplication::initApplication { setApiTypeRequired(pvr::api::OpenGLES3);}
pvr::Result MyApplication::renderFrame() { float dt =this->getFrameTime(); ... }
\endcode

\code
this->getAssetStream("Texture.pvr"); // Will look everywhere for assets: Files, then Windows //Resources(.rc)/iOS bundled resources/Android assets
\endcode

\code
void MyApplication::eventMappedInput(SimplifiedInput::Actions evt){ //Abstracts/simplifies
switch (evt){case MappedInputEvent::Action1: pauseDemo();break;	case MappedInputEvent::Left: showPreviousPage(); break;	case MappedInputEvent::Quit: if (showExitDialogue())
exitShell(); break;}} \endcode

\code
void MyApplication::eventKeyUp(Keys key){} // Or, detailed keyboard/mouse/touch input
\endcode

\section cmd Command-Line Arguments
*****************************

PVRShell takes a set of command-line arguments which allow items like the position and size of the example to be controlled. The table below identifies these options.
<table>
<tr>
<th>Option</th><th>Description</th>
</tr>
<tr>
<td>-aasamples=N</td><td>Sets the number of samples to use for full screen anti-aliasing, e.g., 0, 2, 4, 8.</td>
</tr>
<tr>
<td>-c=N</td><td>Save a single screenshot or a range, for a given frame or frame range, e.g., -c=14, -c=1-10.</td>
</tr>
<tr>
<td>-colourbpp=N or -colorbpp=N or -cbpp=N</td><td>Frame buffer colour bits per pixel. When choosing an EGL config, N will be used as the value for EGL_BUFFER_SIZE.</td>
</tr>
<tr>
<td>-config=N</td><td>Force the shell to use the EGL config with ID N.</td>
</tr>
<tr>
<td>-depthbpp=N or -dbpp=N</td><td>Depth buffer bits per pixel. When choosing an EGL config, N will be used as the value for EGL_DEPTH_SIZE.</td>
</tr>
<tr>
<td>-display</td><td>EGL only. Allows specifying the native display to use if the device has more than one.</td>
</tr>
<tr>
<td>-forceframetime=N or -fft=N</td><td>Force PVRShellGetTime to report fixed frame time.</td>
</tr>
<tr>
<td>-fps</td><td>Output frames per second.</td>
</tr>
<tr>
<td>-fullscreen=[1,0]</td><td>Runs in full-screen mode (1) or windowed (0).</td>
</tr>
<tr>
<td>-height=N</td><td>Sets the viewport height to N.</td>
</tr>
<tr>
<td>-info</td><td>Output setup information to the debug output.</td>
</tr>
<tr>
<td>-posx=N</td><td>Sets the x coordinate of the viewport.</td>
</tr>
<tr>
<td>-posy=N</td><td>Sets the y coordinate of the viewport.</td>
</tr>
<tr>
<td>-powersaving=[1,0] or -ps=[1,0]</td><td>Where available enable/disable power saving.</td>
</tr>
<tr>
<td>-priority=N</td><td>Sets the priority of the EGL context.</td>
</tr>
<tr>
<td>-quitafterframe=N or -qaf=N</td><td>Specify the frame after which to quit.</td>
</tr>
<tr>
<td>-quitaftertime=N or -qat=N</td><td>Specify the time after which to quit.</td>
</tr>
<tr>
<td>-rotatekeys=N</td><td>Sets the orientation of the keyboard input. N can be 0-3, where 0 is no rotation.</td>
</tr>
<tr>
<td>-screenshotscale=N</td><td>Allows to scale up screenshots to a bigger size (pixel replication).</td>
</tr>
<tr>
<td>-sw</td><td>Software render.</td>
</tr>
<tr>
<td>-version</td><td>Output the SDK version to the debug output.</td>
</tr>
<tr>
<td>-vsync=N</td><td>Where available modify the app's vsync parameters. 0: No vsync, 1: Vsync, -1: Adaptive Vsync</td>
</tr>
<tr>
<td>-width=N</td><td>Sets the viewport width to N.</td>
</tr>
</table>
*/
