#include "EglContext.h"
#include "PVRCore/Log.h"

enum
{
	retry_RemoveDebugBit,
	retry_DisableAA,
	retry_ReduceStencilBpp,
	retry_NoStencil,
	retry_StencilBpp,
	retry_ColorBpp,
	retry_reduceAlphaBpp,
	retry_NoAlpha,
	retry_DepthBpp,
	retry_DONE
};
const char* retries_string[] = { "RemoveDebugBit", "DisableAA", "ReduceStencilBpp", "NoStencil", "StencilBpp", "ColorBpp", "ReduceAlphaBpp", "NoAlpha", "DepthBpp" };

void fixAttributes(const pvr::DisplayAttributes& origAttr, pvr::DisplayAttributes& attr, uint32_t* retries, bool& debugBit)
{
	if (retries[retry_ColorBpp] == 1) // 0:inactive 1:active, currently tested 2: active, unsure  3:active fixed
	{
		attr.redBits = 1;
		attr.greenBits = 1;
		attr.blueBits = 1;
	}
	else if (retries[retry_ColorBpp] == 0)
	{
		attr.redBits = origAttr.redBits;
		attr.greenBits = origAttr.greenBits;
		attr.blueBits = origAttr.blueBits;
	}
	if (!(retries[retry_reduceAlphaBpp] == 3) && !(retries[retry_NoAlpha] == 3)) // fixed. leave it alone..
	{
		if (retries[retry_reduceAlphaBpp] == 0 && retries[retry_NoAlpha] == 0) // reset.
		{ attr.alphaBits = origAttr.alphaBits; } // mutually exclusive if (retries[retry_reduceAlphaBpp] == 1) // test one
		{
			attr.alphaBits = 1;
		} // mutually exclusive
		if (retries[retry_NoAlpha] == 1) // test two
		{ attr.alphaBits = 0; } // mutually exclusive
	}

	if (retries[retry_DepthBpp] == 1) { attr.depthBPP = 1; }
	else if (retries[retry_DepthBpp] == 0)
	{
		attr.depthBPP = origAttr.depthBPP;
	}
	if (!(retries[retry_ReduceStencilBpp] == 3) && !(retries[retry_NoStencil] == 3)) // fixed. leave it alone..
	{
		if (retries[retry_ReduceStencilBpp] == 0 && retries[retry_NoStencil] == 0) // reset.
		{ attr.stencilBPP = origAttr.stencilBPP; } // mutually exclusive
		if (retries[retry_ReduceStencilBpp] == 1) // test one
		{ attr.stencilBPP = 1; } // mutually exclusive
		if (retries[retry_NoStencil] == 1) // test two
		{ attr.stencilBPP = 0; } // mutually exclusive
	}

	if (retries[retry_DisableAA] == 1)
	{
		if (attr.aaSamples > 0)
		{
			// Reduce the anti-aliasing
			attr.aaSamples = attr.aaSamples >> 1;
		}
	}
	else if (retries[retry_DisableAA] == 0)
	{
		attr.aaSamples = origAttr.aaSamples;
	}
#ifdef DEBUG
	bool ORIG_DEBUG_BIT = true;
#else
	bool ORIG_DEBUG_BIT = false;
#endif
	if (retries[retry_RemoveDebugBit] == 1) { debugBit = false; }
	else if (retries[retry_RemoveDebugBit] == 0)
	{
		debugBit = ORIG_DEBUG_BIT;
	}
}

char const* eglErrorToStr(EGLint errorCode)
{
	switch (errorCode)
	{
	case EGL_SUCCESS: return "EGL_SUCCESS";
	case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
	case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
	case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
	case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
	case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
	case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
	case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
	case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
	case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
	case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
	case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
	case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
	case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
	case EGL_CONTEXT_LOST: return "EGL_CONTEXT_LOST";
	default: return "EGL_SUCCESS";
	}
}
inline EGLNativeDisplayType ptrToEGLNativeDisplayType(void* ptr)
{
	void* tempP[1];
	tempP[0] = ptr;
	return *(EGLNativeDisplayType*)tempP;
}

void EglContext::release()
{
	// Check the current context/surface/display. If they are equal to the handles in this class, remove them from the current context
	if (_platformContextHandles.display == egl::GetCurrentDisplay() && _platformContextHandles.display != EGL_NO_DISPLAY &&
		_platformContextHandles.drawSurface == egl::GetCurrentSurface(EGL_DRAW) && _platformContextHandles.readSurface == egl::GetCurrentSurface(EGL_READ) &&
		_platformContextHandles.context == egl::GetCurrentContext())
	{ egl::MakeCurrent(egl::GetCurrentDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT); } // These are all reference counted, so these can be safely deleted.
	if (_platformContextHandles.display)
	{
		if (_platformContextHandles.context) { egl::DestroyContext(_platformContextHandles.display, _platformContextHandles.context); }

		if (_platformContextHandles.drawSurface) { egl::DestroySurface(_platformContextHandles.display, _platformContextHandles.drawSurface); }

		if (_platformContextHandles.readSurface && _platformContextHandles.readSurface != _platformContextHandles.drawSurface)
		{ egl::DestroySurface(_platformContextHandles.display, _platformContextHandles.readSurface); }
		egl::Terminate(_platformContextHandles.display);
	}
}

bool EglContext::makeCurrent()
{
	bool result =
		(egl::MakeCurrent(_platformContextHandles.display, _platformContextHandles.drawSurface, _platformContextHandles.drawSurface, _platformContextHandles.context) == EGL_TRUE);
#if !defined(__ANDROID__) && !defined(TARGET_OS_IPHONE)
	if (_swapInterval != -2)
	{
		// Set our swap interval which affects the current draw surface
		egl::SwapInterval(_platformContextHandles.display, _swapInterval);
		_swapInterval = -2;
	}
#endif
	return result;
}

bool EglContext::swapBuffers()
{
	static const GLenum attachments[] = { GL_DEPTH, GL_STENCIL };

	if (_isDiscardSupported)
	{
		gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
		if (_apiType >= pvr::Api::OpenGLES3) { gl::InvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments); }
		else
		{
			gl::ext::DiscardFramebufferEXT(GL_FRAMEBUFFER, 1, attachments);
		}
	}
	return (egl::SwapBuffers(_platformContextHandles.display, _platformContextHandles.drawSurface) == EGL_TRUE);
}

EGLContext getContextForConfig(EGLDisplay display, EGLConfig config, pvr::Api graphicsapi)
{
	static bool firstRun = true;
	EGLint contextAttributes[10];
	int i = 0;

	int requestedMajorVersion = -1;
	int requestedMinorVersion = -1;

	switch (graphicsapi)
	{
	case pvr::Api::OpenGLES2:
		requestedMajorVersion = 2;
		requestedMinorVersion = 0;
		break;
	case pvr::Api::OpenGLES3:
		requestedMajorVersion = 3;
		requestedMinorVersion = 0;
		break;
	case pvr::Api::OpenGLES31:
		requestedMajorVersion = 3;
		requestedMinorVersion = 1;
		break;
	default: return EGL_NO_CONTEXT;
	}

#ifdef DEBUG
	int debug_flag = 0;
#endif

#if defined(EGL_CONTEXT_MAJOR_VERSION_KHR)
	if (egl::isEglExtensionSupported(display, "EGL_KHR_create_context"))
	{
		if (firstRun)
		{
			firstRun = false;
			Log(LogLevel::Information, "EGL context creation: EGL_KHR_create_context supported");
		}
		contextAttributes[i++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
		contextAttributes[i++] = requestedMajorVersion;
		contextAttributes[i++] = EGL_CONTEXT_MINOR_VERSION_KHR;
		contextAttributes[i++] = requestedMinorVersion;
#ifdef DEBUG
		debug_flag = i;
		contextAttributes[i++] = EGL_CONTEXT_FLAGS_KHR;
		contextAttributes[i++] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;

#endif
	}
	else
#endif
	{
		if (firstRun)
		{
			firstRun = false;
			Log(LogLevel::Information, "EGL context creation: EGL_KHR_create_context NOT supported. Minor versions and debug context are unavailable.");
		}
		contextAttributes[i++] = EGL_CONTEXT_CLIENT_VERSION;
		contextAttributes[i++] = requestedMajorVersion;
	}

	contextAttributes[i] = EGL_NONE;

	// Create the context
	EGLContext _context = egl::CreateContext(display, config, NULL, contextAttributes);
	if (_context == EGL_NO_CONTEXT)
	{
#ifdef DEBUG
		// RETRY! Do we support debug bit?
		egl::GetError(); // clear error
		contextAttributes[debug_flag] = EGL_NONE;
		_context = egl::CreateContext(display, config, NULL, contextAttributes);
#endif
	}
	return _context;
}

bool EglContext::isGlesVersionSupported(EGLDisplay display, pvr::Api graphicsapi, bool& isSupported)
{
#if defined(TARGET_OS_MAC)
	/* Max API supported on macOS is OpenGLES3*/
	if (graphicsapi > pvr::Api::OpenGLES3) { return false; }
#endif

	isSupported = false;
	std::vector<EGLConfig> configs;
	EGLint configAttributes[32];
	uint32_t i = 0;

	{
		configAttributes[i++] = EGL_SURFACE_TYPE;
		configAttributes[i++] = EGL_WINDOW_BIT;

		switch (graphicsapi)
		{
		case pvr::Api::OpenGLES2: // GLES2
			Log(LogLevel::Debug, "EglPlatformContext.cpp: isGlesVersionSupported: Setting EGL_OPENGL_ES2_BIT");
			configAttributes[i++] = EGL_RENDERABLE_TYPE;
			configAttributes[i++] = EGL_OPENGL_ES2_BIT;
			break;
		case pvr::Api::OpenGLES3: // GLES2
		case pvr::Api::OpenGLES31: // GLES2
			Log(LogLevel::Debug, "EglPlatformContext.cpp: isGlesVersionSupported: Setting EGL_OPENGL_ES3_BIT_KHR");
			configAttributes[i++] = EGL_RENDERABLE_TYPE;
			configAttributes[i++] = EGL_OPENGL_ES3_BIT_KHR;
			break;
		default: return false; break;
		}
	}

	configAttributes[i] = EGL_NONE;

	EGLint numConfigs;
	EGLint configsSize;

	// Find out how many configs there are in total that match our criteria
	if (!egl::ChooseConfig(display, configAttributes, NULL, 0, &configsSize))
	{
		Log("EglPlatformContext.cpp: getMaxEglVersion: eglChooseConfig error");
		return false;
	}
	Log(LogLevel::Debug, "EglPlatformContext.cpp: isGlesVersionSupported: number of configurations found for ES version [%s] was [%d]", apiName(graphicsapi), configsSize);
	if (configsSize >= 0)
	{
		configs.resize(configsSize);

		if (egl::ChooseConfig(display, configAttributes, configs.data(), configsSize, &numConfigs) != EGL_TRUE || numConfigs != configsSize)
		{
			Log("EglPlatformContext.cpp: getMaxEglVersion - eglChooseConfig unexpected error %x getting list of configurations, but %d possible configs were already detected.",
				egl::GetError(), configsSize);
			return false;
		}

		Log(LogLevel::Information, "Trying to create context for all configs.");
		for (size_t ii = 0; ii != configs.size(); ++ii)
		{
			EGLContext _context;
			if ((_context = getContextForConfig(display, configs[ii], graphicsapi)) != EGL_NO_CONTEXT)
			{
				Log(LogLevel::Information, "SUCCESS creating context! Reporting success. (Used config #%d) .", ii);
				isSupported = true;
				egl::DestroyContext(display, _context);
				return true;
			}
		}
		Log(LogLevel::Information, "Failed to create context for any configs. Tried %d configs.", configs.size());
	}
	return true;
}

void EglContext::populateMaxApiVersion()
{
	_maxApiVersion = pvr::Api::Unspecified;
	pvr::Api graphicsapi = pvr::Api::OpenGLESMaxVersion;
	bool supported;
	bool result;
	while (graphicsapi > pvr::Api::Unspecified)
	{
		const char* esversion =
			(graphicsapi == pvr::Api::OpenGLES31 ? "3.1" : graphicsapi == pvr::Api::OpenGLES3 ? "3.0" : graphicsapi == pvr::Api::OpenGLES2 ? "2.0" : "UNKNOWN_VERSION");
		result = isGlesVersionSupported(_platformContextHandles.display, graphicsapi, supported);

		if (result == true)
		{
			if (supported)
			{
				_maxApiVersion = graphicsapi;
				Log(LogLevel::Information, "Maximum API level detected: OpenGL ES %s", esversion);
				return;
			}
			else
			{
				Log(LogLevel::Information, "OpenGL ES %s NOT supported. Trying lower version...", esversion);
			}
		}
		else
		{
			Log("Error detected while testing OpenGL ES version %s for compatibility. Trying lower version", esversion);
		}
		graphicsapi = static_cast<pvr::Api>(static_cast<int32_t>(graphicsapi) - 1);
	}
	Log(LogLevel::Critical, "=== FATAL: COULD NOT FIND COMPATIBILITY WITH ANY OPENGL ES VERSION ===");
}

bool EglContext::preInitialize(pvr::OSDisplay osdisplay, NativePlatformHandle& handles)
{
	// Associate the display with EGL.
	handles.display = egl::GetDisplay(ptrToEGLNativeDisplayType(osdisplay));

	if (handles.display == EGL_NO_DISPLAY) { handles.display = egl::GetDisplay(static_cast<EGLNativeDisplayType>(EGL_DEFAULT_DISPLAY)); }

	if (handles.display == EGL_NO_DISPLAY) { return false; }

	// Initialize the display
	if (egl::Initialize(handles.display, NULL, NULL) != EGL_TRUE) { return false; }

	//  // Bind the correct API
	int result = EGL_FALSE;

	result = egl::BindAPI(EGL_OPENGL_ES_API);

	if (result != EGL_TRUE) { return false; }
	return true;
}

bool EglContext::initializeContext(bool wantWindow, pvr::DisplayAttributes& original_attributes, NativePlatformHandle& handles, EGLConfig& config, pvr::Api graphicsapi)
{
	// Choose a config based on user supplied attributes
	std::vector<EGLConfig> configs;
	EGLint configAttributes[32];
	bool debugBit = 0;

	int requestedMajorVersion = -1;
	int requestedMinorVersion = -1;

	switch (graphicsapi)
	{
	case pvr::Api::OpenGLES2:
		requestedMajorVersion = 2;
		requestedMinorVersion = 0;
		break;
	case pvr::Api::OpenGLES3:
		requestedMajorVersion = 3;
		requestedMinorVersion = 0;
		break;
	case pvr::Api::OpenGLES31:
		requestedMajorVersion = 3;
		requestedMinorVersion = 1;
		break;
	default: break;
	}

	bool create_context_supported = egl::isEglExtensionSupported(handles.display, "EGL_KHR_create_context");
	if (create_context_supported) { Log(LogLevel::Information, "EGL context creation: EGL_KHR_create_context supported..."); }
	else
	{
		Log(requestedMinorVersion ? LogLevel::Warning : LogLevel::Information,
			"EGL context creation: EGL_KHR_create_context not supported. Minor version will be discarded, and debug disabled.");
		requestedMinorVersion = 0;
	}

	Log(LogLevel::Information, "Trying to get OpenGL ES version : %d.%d", requestedMajorVersion, requestedMinorVersion);

	bool context_priority_supported = egl::isEglExtensionSupported(handles.display, "EGL_IMG_context_priority");
	if (context_priority_supported)
	{
		switch (original_attributes.contextPriority)
		{
		case 0: Log(LogLevel::Information, "EGL context creation: EGL_IMG_context_priority supported! Setting context LOW priority..."); break;
		case 1: Log(LogLevel::Information, "EGL context creation: EGL_IMG_context_priority supported! Setting context MEDIUM priority..."); break;
		default: Log(LogLevel::Information, "EGL context creation: EGL_IMG_context_priority supported! Setting context HIGH priority (default)..."); break;
		}
	}
	else
	{
		Log(LogLevel::Information, "EGL context creation: EGL_IMG_context_priority not supported. Ignoring context Priority attribute.");
	}

	uint32_t retries[retry_DONE] = {};
	// O = not tried, 1 = now trying, 2 = not sure, 3 = keep disabled.
	pvr::DisplayAttributes attributes = original_attributes;
#ifdef DEBUG
	debugBit = true;
#endif

	if (!debugBit) { retries[retry_RemoveDebugBit] = 4; }
	if (attributes.aaSamples == 0) { retries[retry_DisableAA] = 3; }
	if (attributes.alphaBits == 0) { retries[retry_reduceAlphaBpp] = 3; }
	if (attributes.alphaBits == 0) { retries[retry_NoAlpha] = 3; }
	if (attributes.stencilBPP == 0) { retries[retry_StencilBpp] = 3; }
	if (attributes.stencilBPP == 0) { retries[retry_NoStencil] = 3; }
	if (attributes.depthBPP == 0) { retries[retry_DepthBpp] = 3; }
	if (attributes.forceColorBPP) { retries[retry_ColorBpp] = 3; }

	for (;;)
	{
		uint32_t i = 0;
		Log(LogLevel::Debug, "Attempting to create context with:\n");
		Log(LogLevel::Debug, "\tDebugbit: %s", debugBit ? "true" : "false");
		Log(LogLevel::Debug, "\tRedBits: %d", attributes.redBits);
		Log(LogLevel::Debug, "\tGreenBits: %d", attributes.greenBits);
		Log(LogLevel::Debug, "\tBlueBits: %d", attributes.blueBits);
		Log(LogLevel::Debug, "\tAlphaBits: %d", attributes.alphaBits);
		Log(LogLevel::Debug, "\tDepthBits: %d", attributes.depthBPP);
		Log(LogLevel::Debug, "\tStencilBits: %d", attributes.stencilBPP);

		if (attributes.configID > 0)
		{
			configAttributes[i++] = EGL_CONFIG_ID;
			configAttributes[i++] = attributes.configID;
		}
		else
		{
			configAttributes[i++] = EGL_RED_SIZE;
			configAttributes[i++] = attributes.redBits;

			configAttributes[i++] = EGL_GREEN_SIZE;
			configAttributes[i++] = attributes.greenBits;

			configAttributes[i++] = EGL_BLUE_SIZE;
			configAttributes[i++] = attributes.blueBits;

			configAttributes[i++] = EGL_ALPHA_SIZE;
			configAttributes[i++] = attributes.alphaBits;

			// For OpenGLES clamp between 0 and 24
			attributes.depthBPP = std::min(attributes.depthBPP, 24u);

			configAttributes[i++] = EGL_DEPTH_SIZE;
			configAttributes[i++] = attributes.depthBPP;

			configAttributes[i++] = EGL_STENCIL_SIZE;
			configAttributes[i++] = attributes.stencilBPP;

			if (wantWindow)
			{
				configAttributes[i++] = EGL_SURFACE_TYPE;
				configAttributes[i++] = EGL_WINDOW_BIT;
			}

			switch (graphicsapi)
			{
			case pvr::Api::OpenGLES2: // GLES2
				Log(LogLevel::Debug, "EGL context creation: Setting EGL_OPENGL_ES2_BIT");
				configAttributes[i++] = EGL_RENDERABLE_TYPE;
				configAttributes[i++] = EGL_OPENGL_ES2_BIT;
				break;
			case pvr::Api::OpenGLES3: // GLES2
			case pvr::Api::OpenGLES31: // GLES2
				Log(LogLevel::Debug, "EGL context creation: EGL_OPENGL_ES3_BIT");
				configAttributes[i++] = EGL_RENDERABLE_TYPE;
				configAttributes[i++] = EGL_OPENGL_ES3_BIT_KHR;
				break;
			default: break;
			}

			// Append number of number of samples depending on AA samples value set
			if (attributes.aaSamples > 0)
			{
				Log(LogLevel::Debug, "EGL context creation: EGL_SAMPLE_BUFFERS 1");
				Log(LogLevel::Debug, "EGL context creation: EGL_SAMPLES %d", attributes.aaSamples);
				configAttributes[i++] = EGL_SAMPLE_BUFFERS;
				configAttributes[i++] = 1;
				configAttributes[i++] = EGL_SAMPLES;
				configAttributes[i++] = attributes.aaSamples;
			}
		}

		configAttributes[i] = EGL_NONE;

		EGLint numConfigs;
		EGLint configsSize;

		EGLint eglerror = egl::GetError();
		assertion(eglerror == EGL_SUCCESS, "initializeContext: egl error logged before choosing egl config");
		eglerror = egl::ChooseConfig(handles.display, configAttributes, NULL, 0, &configsSize);
		assertion(eglerror == EGL_TRUE, "initializeContext: EGL config returned a value that was not EGL_TRUE");
		eglerror = egl::GetError();
		assertion(eglerror == EGL_SUCCESS, "initializeContext: EGL choose config raised EGL error");

		if (attributes.forceColorBPP)
		{
			if (configsSize == 0) { return false; }
		}
		else
		{
			if (configsSize > 1) { configsSize = 1; }
		}
		numConfigs = configsSize;
		if (configsSize)
		{
			configs.resize(configsSize);

			if (egl::ChooseConfig(handles.display, configAttributes, configs.data(), configsSize, &numConfigs) != EGL_TRUE)
			{
				Log("EGL context creation: initializeContext Error choosing egl config. %x.    Expected number of configs: %d    Actual: %d.", egl::GetError(), numConfigs, configsSize);
				return false;
			}
		}
		Log(LogLevel::Information, "EGL context creation: Number of EGL Configs found: %d", configsSize);

		if (numConfigs > 0)
		{
			EGLint configIdx = 0;

			if (attributes.forceColorBPP)
			{
				Log(LogLevel::Information, "EGL context creation: Trying to find a for forced BPP compatible context support...");
				EGLint value;

				for (; configIdx < configsSize; ++configIdx)
				{
					if ((egl::GetConfigAttrib(handles.display, configs[configIdx], EGL_RED_SIZE, &value) && value == static_cast<EGLint>(original_attributes.redBits)) &&
						(egl::GetConfigAttrib(handles.display, configs[configIdx], EGL_GREEN_SIZE, &value) && value == static_cast<EGLint>(original_attributes.greenBits)) &&
						(egl::GetConfigAttrib(handles.display, configs[configIdx], EGL_BLUE_SIZE, &value) && value == static_cast<EGLint>(original_attributes.blueBits)) &&
						(egl::GetConfigAttrib(handles.display, configs[configIdx], EGL_ALPHA_SIZE, &value) && value == static_cast<EGLint>(original_attributes.alphaBits)))
					{ break; }
				}
			}

			config = configs[configIdx];

			EGLint contextAttributes[32];
			i = 0;

#if defined(EGL_CONTEXT_MAJOR_VERSION_KHR)
			if (create_context_supported)
			{
				contextAttributes[i++] = EGL_CONTEXT_MAJOR_VERSION_KHR;
				contextAttributes[i++] = requestedMajorVersion;
				contextAttributes[i++] = EGL_CONTEXT_MINOR_VERSION_KHR;
				contextAttributes[i++] = requestedMinorVersion;
#ifdef DEBUG
				if (debugBit)
				{
					contextAttributes[i++] = EGL_CONTEXT_FLAGS_KHR;
					contextAttributes[i++] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
				}
#endif
			}
			else
#endif
			{
				contextAttributes[i++] = EGL_CONTEXT_CLIENT_VERSION;
				contextAttributes[i++] = requestedMajorVersion;
			}

			if (context_priority_supported)
			{
				contextAttributes[i++] = EGL_CONTEXT_PRIORITY_LEVEL_IMG;

				switch (attributes.contextPriority)
				{
				case 0: contextAttributes[i++] = EGL_CONTEXT_PRIORITY_LOW_IMG; break;
				case 1: contextAttributes[i++] = EGL_CONTEXT_PRIORITY_MEDIUM_IMG; break;
				default: contextAttributes[i++] = EGL_CONTEXT_PRIORITY_HIGH_IMG; break;
				}
			}
			contextAttributes[i] = EGL_NONE;

			Log(LogLevel::Information, "Creating EGL context...");
			// Create the context
			handles.context = egl::CreateContext(handles.display, config, NULL, contextAttributes);

			//// SUCCESS -- FUNCTION SUCCESSFUL EXIT POINT
			if (handles.context != EGL_NO_CONTEXT)
			{
				Log(LogLevel::Debug, "EGL context created. Will now check if any attributes were being debugged, and try to roll back unnecessary changes.");
				bool is_final = true;
				for (uint32_t retrybit = 0; retrybit < retry_DONE && is_final; ++retrybit)
				{
					if (retries[retrybit] == 1) // If was "now trying"...
					{
						Log(LogLevel::Debug,
							"Current testing bit was %s. Will mark this as 'definitely not supported'(3), clear all 'tentative'(2) bits if present. If no tentative bits were "
							"found, will succeed!",
							retries_string[retrybit]);
						retries[retrybit] = 3; // Fix that we definitely need that.
						// Now, we try and enable all the rest of them...
						for (uint32_t retrybit_2 = 0; retrybit_2 < retry_DONE; ++retrybit_2)
						{
							if (retries[retrybit_2] == 2) // Bit is: Tried disabling it, not sure yet
							{
								is_final = false;
								retries[retrybit_2] = 0; // Reset it!
							}
						}
					}
				}

				if (!is_final)
				{
					Log(LogLevel::Debug, "Found EGL attribute retry bits to attempt reset. Will now test without the disabled attributes.");
					fixAttributes(original_attributes, attributes, retries, debugBit);
					continue;
				}

				Log(LogLevel::Debug, "EGL context successfully created! Updating Config Attributes to reflect actual context parameters...");

				// Update the attributes to the config's
				egl::GetConfigAttrib(handles.display, config, EGL_RED_SIZE, (EGLint*)&attributes.redBits);
				egl::GetConfigAttrib(handles.display, config, EGL_GREEN_SIZE, (EGLint*)&attributes.greenBits);
				egl::GetConfigAttrib(handles.display, config, EGL_BLUE_SIZE, (EGLint*)&attributes.blueBits);
				egl::GetConfigAttrib(handles.display, config, EGL_ALPHA_SIZE, (EGLint*)&attributes.alphaBits);
				egl::GetConfigAttrib(handles.display, config, EGL_DEPTH_SIZE, (EGLint*)&attributes.depthBPP);
				egl::GetConfigAttrib(handles.display, config, EGL_STENCIL_SIZE, (EGLint*)&attributes.stencilBPP);
				Log(LogLevel::Information, "EGL Initialized Successfully");
				// BREAK OUT OF THE FOR LOOP TO CONTINUE FURTHER!
				break;
			}

			// clear the EGL error
			eglerror = egl::GetError();
			if (eglerror != EGL_SUCCESS) { Log(LogLevel::Debug, "Context not created yet. Clearing EGL errors."); }
		} // eglChooseConfif failed.

		//// FAILURE ////
		if (attributes.configID > 0)
		{
			Log(LogLevel::Error, "Failed to create egl::Context with config ID %i", attributes.configID);
			return false;
		}

		Log(LogLevel::Debug, "Context creation failed - Will change EGL attributes and retry.");

		// FAILURE
		bool must_retry = false;
		for (uint32_t retry_bit = 0; retry_bit < retry_DONE; ++retry_bit)
		{
			if (retries[retry_bit] == 1) // Was the current one being checked out?
			{
				Log(LogLevel::Information, "Setting bit %s as 'unsure'(2), since the context creation still failed.", retries_string[retry_bit]);
				retries[retry_bit] = 2; // Mark it as "unsure"
				break;
			}
		}
		for (uint32_t retry_bit = 0; retry_bit < retry_DONE && !must_retry; ++retry_bit)
		{
			if (retries[retry_bit] == 0) // Found one we didn't test?
			{
				Log(LogLevel::Information, "Setting bit %s as 'currently testing'(1).", retries_string[retry_bit]);
				retries[retry_bit] = 1; // Test it...
				must_retry = true;
			}
		}

		if (must_retry) { fixAttributes(original_attributes, attributes, retries, debugBit); }
		else
		{
			// Nothing else we can remove, fail
			Log(LogLevel::Critical, "Failed to create egl::Context. Unknown reason of failure. Last error logged is: %s", eglErrorToStr(egl::GetError()));

			return false;
		}
	}
	return true;
}

bool EglContext::init(pvr::OSWindow window, pvr::OSDisplay display, pvr::DisplayAttributes& attributes, pvr::Api minApi, pvr::Api maxApi)
{
	if (!preInitialize(display, _platformContextHandles)) { return false; }

	populateMaxApiVersion();

	if (maxApi == pvr::Api::Unspecified) { maxApi = _maxApiVersion; }
	if (minApi == pvr::Api::Unspecified) { minApi = pvr::Api::OpenGLES2; }
	else
	{
		maxApi = std::min(maxApi, _maxApiVersion);
	}

	if (minApi > maxApi)
	{
		Log(LogLevel::Error,
			"================================================================================\n"
			"API level requested [%s] was not supported. Max supported API level on this device is [%s]\n"
			"**** APPLICATION WILL EXIT ****\n"
			"================================================================================",
			apiName(minApi), apiName(_maxApiVersion));
		return false;
	}

	if (minApi == pvr::Api::Unspecified)
	{
		_apiType = maxApi;
		Log(LogLevel::Information, "Unspecified target API -- Setting to max API level : %s", apiName(_apiType));
	}
	else
	{
		_apiType = std::max(minApi, maxApi);
		Log(LogLevel::Information, "Requested minimum API level : %s. Will actually create %s since it is supported.", apiName(minApi), apiName(_apiType));
	}

	EGLConfig config;
	if (!initializeContext(true, attributes, _platformContextHandles, config, _apiType)) { return false; }

	// CREATE THE WINDOW SURFACE
#if defined(Wayland)
	_platformContextHandles.eglWindow = wl_egl_window_create((wl_surface*)window, attributes.width, attributes.height);
	if (_platformContextHandles.eglWindow == EGL_NO_SURFACE) { Log("Can't create egl window\n"); }
	else
	{
		Log(LogLevel::Information, "Created wl egl window\n");
	}
#endif

	EGLint eglattribs[] = { EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE, EGL_NONE };

	if (attributes.frameBufferSrgb)
	{
		bool isSrgbSupported = egl::isEglExtensionSupported(_platformContextHandles.display, "EGL_KHR_gl_colorspace");
		if (isSrgbSupported)
		{
			eglattribs[0] = EGL_COLORSPACE;
			eglattribs[1] = EGL_COLORSPACE_sRGB;
		}
		else
		{
			Log(LogLevel::Warning, "sRGB window backbuffer requested, but EGL_KHR_gl_colorspace is not supported. Creating linear RGB backbuffer.");
			attributes.frameBufferSrgb = false;
		}
	}

	_platformContextHandles.drawSurface = _platformContextHandles.readSurface = egl::CreateWindowSurface(_platformContextHandles.display, config,
#if defined(Wayland)
		_platformContextHandles.eglWindow,
#else
		reinterpret_cast<EGLNativeWindowType>(window),
#endif
		eglattribs);

	if (_platformContextHandles.drawSurface == EGL_NO_SURFACE)
	{
		Log(LogLevel::Error, "Context creation failed\n");
		return false;
	}

	// Update the attributes to the surface's
	egl::QuerySurface(_platformContextHandles.display, _platformContextHandles.drawSurface, EGL_WIDTH, (EGLint*)&attributes.width);
	egl::QuerySurface(_platformContextHandles.display, _platformContextHandles.drawSurface, EGL_HEIGHT, (EGLint*)&attributes.height);

	_swapInterval = 1;
	switch (attributes.vsyncMode)
	{
	case pvr::VsyncMode::Half: _swapInterval = 2; break;
	case pvr::VsyncMode::Mailbox:
	case pvr::VsyncMode::Off: _swapInterval = 0; break;
	case pvr::VsyncMode::Relaxed: _swapInterval = -1; break;
	default: break;
	}

	_isDiscardSupported = ((_apiType >= pvr::Api::OpenGLES3) || egl::isEglExtensionSupported("GL_EXT_discard_framebuffer"));
	makeCurrent();
	return true;
}
