/*!
\brief Contains the implementation for the pvr::platform::ShellOS class on Android systems.
\file PVRShell/OS/Android/ShellOS.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRShell/OS/ShellOS.h"
#include "PVRCore/stream/FilePath.h"
#include "PVRCore/Log.h"

#include <android_native_app_glue.h>
#include <android/sensor.h>
#include <android/window.h>
#include <unistd.h>

//!\cond NO_DOXYGEN
namespace pvr {
namespace platform {
class InternalOS
{};

static Keys keyboardKeyMap[]{
	Keys::Unknown, /* AKEYCODE_UNKNOWN */
	Keys::Left, /* AKEYCODE_SOFT_LEFT */
	Keys::Right, /* AKEYCODE_SOFT_RIGHT */
	Keys::Home, /* AKEYCODE_HOME */
	Keys::Escape, /* AKEYCODE_BACK */
	Keys::Unknown, /* AKEYCODE_CALL */
	Keys::Unknown, /* AKEYCODE_ENDCALL */
	Keys::Key0, /* ACODE_0 */
	Keys::Key1, /* ACODE_1 */
	Keys::Key2, /* ACODE_2 */
	Keys::Key3, /* ACODE_3 */
	Keys::Key4, /* ACODE_4 */
	Keys::Key5, /* ACODE_5 */
	Keys::Key6, /* ACODE_6 */
	Keys::Key7, /* ACODE_7 */
	Keys::Key8, /* ACODE_8 */
	Keys::Key9, /* ACODE_9 */
	Keys::NumMul, /* ACODE_STAR */
	Keys::Backslash, /* ACODE_POUND */
	Keys::Up, /* ACODE_DPAD_UP */
	Keys::Down, /* ACODE_DPAD_DOWN */
	Keys::Left, /* ACODE_DPAD_LEFT */
	Keys::Right, /* ACODE_DPAD_RIGHT */
	Keys::Space, /* ACODE_DPAD_CENTER */
	Keys::Unknown, /* ACODE_VOLUME_UP */
	Keys::Unknown, /* ACODE_VOLUME_DOWN */
	Keys::Unknown, /* ACODE_POWER */
	Keys::Unknown, /* ACODE_CAMERA */
	Keys::Unknown, /* ACODE_CLEAR */
	Keys::A, /* ACODE_A */
	Keys::B, /* ACODE_B */
	Keys::C, /* ACODE_C */
	Keys::D, /* ACODE_D */
	Keys::E, /* ACODE_E */
	Keys::F, /* ACODE_F */
	Keys::G, /* ACODE_G */
	Keys::H, /* ACODE_H */
	Keys::I, /* ACODE_I */
	Keys::J, /* ACODE_J */
	Keys::K, /* ACODE_K */
	Keys::L, /* ACODE_L */
	Keys::M, /* ACODE_M */
	Keys::N, /* ACODE_N */
	Keys::O, /* ACODE_O */
	Keys::P, /* ACODE_P */
	Keys::Q, /* ACODE_Q */
	Keys::R, /* ACODE_R */
	Keys::S, /* ACODE_S */
	Keys::T, /* ACODE_T */
	Keys::U, /* ACODE_U */
	Keys::V, /* ACODE_V */
	Keys::W, /* ACODE_W */
	Keys::X, /* ACODE_X */
	Keys::Y, /* ACODE_Y */
	Keys::Z, /* ACODE_Z */
	Keys::Comma, /* ACODE_COMMA */
	Keys::Period, /* ACODE_PERIOD */
	Keys::Alt, /* ACODE_ALT_LEFT */
	Keys::Alt, /* ACODE_ALT_RIGHT */
	Keys::Shift, /* ACODE_SHIFT_LEFT */
	Keys::Shift, /* ACODE_SHIFT_RIGHT */
	Keys::Tab, /* ACODE_TAB */
	Keys::Space, /* ACODE_SPACE */
	Keys::Unknown, /* ACODE_SYM */
	Keys::Unknown, /* ACODE_EXPLORER */
	Keys::Unknown, /* ACODE_ENVELOPE */
	Keys::Return, /* ACODE_ENTER */
	Keys::Delete, /* ACODE_DEL */
	Keys::Backquote, /* ACODE_GRAVE */
	Keys::Minus, /* ACODE_MINUS */
	Keys::Equals, /* ACODE_EQUALS */
	Keys::SquareBracketLeft, /* ACODE_LEFT_BRACKET */
	Keys::SquareBracketRight, /* ACODE_RIGHT_BRACKET */
	Keys::Backslash, /* ACODE_BACKSLASH */
	Keys::Semicolon, /* ACODE_SEMICOLON */
	Keys::Quote, /* ACODE_APOSTROPHE */
	Keys::Slash, /* ACODE_SLASH */
	Keys::Unknown, /* ACODE_AT */
	Keys::Unknown, /* ACODE_NUM */
	Keys::Unknown, /* ACODE_HEADSETHOOK */
	Keys::Unknown, /* ACODE_FOCUS */
	Keys::NumAdd, /* ACODE_PLUS */
	Keys::Unknown, /* ACODE_MENU */
	Keys::Unknown, /* ACODE_NOTIFICATION */
	Keys::Unknown, /* ACODE_SEARCH */
	Keys::Unknown, /* ACODE_MEDIA_PLAY_PAUSE= */
	Keys::Unknown, /* ACODE_MEDIA_STOP */
	Keys::Unknown, /* ACODE_MEDIA_NEXT */
	Keys::Unknown, /* ACODE_MEDIA_PREVIOUS */
	Keys::Unknown, /* ACODE_MEDIA_REWIND */
	Keys::Unknown, /* ACODE_MEDIA_FAST_FORWARD */
	Keys::Unknown, /* ACODE_MUTE */
	Keys::PageUp, /* ACODE_PAGE_UP */
	Keys::PageDown, /* ACODE_PAGE_DOWN */
	Keys::Unknown, /* ACODE_PICTSYMBOLS */
	Keys::Unknown, /* ACODE_SWITCH_CHARSET */
	Keys::Unknown, /* ACODE_BUTTON_A */
	Keys::Unknown, /* ACODE_BUTTON_B */
	Keys::Unknown, /* ACODE_BUTTON_C */
	Keys::Unknown, /* ACODE_BUTTON_X */
	Keys::Unknown, /* ACODE_BUTTON_Y */
	Keys::Unknown, /* ACODE_BUTTON_Z */
	Keys::Unknown, /* ACODE_BUTTON_L1 */
	Keys::Unknown, /* ACODE_BUTTON_R1 */
	Keys::Unknown, /* ACODE_BUTTON_L2 */
	Keys::Unknown, /* ACODE_BUTTON_R2 */
	Keys::Unknown, /* ACODE_BUTTON_THUMBL */
	Keys::Unknown, /* ACODE_BUTTON_THUMBR */
	Keys::Unknown, /* ACODE_BUTTON_START */
	Keys::Unknown, /* ACODE_BUTTON_SELECT */
	Keys::Unknown, /* ACODE_BUTTON_MODE */
	Keys::Escape, /* ACODE_ESCAPE */
	Keys::Delete, /* ACODE_FORWARD_DEL */
	Keys::Control, /* ACODE_CTRL_LEFT */
	Keys::Control, /* ACODE_CTRL_RIGHT */
	Keys::CapsLock, /* ACODE_CAPS_LOCK */
	Keys::ScrollLock, /* ACODE_SCROLL_LOCK */
	Keys::Unknown, /* ACODE_META_LEFT */
	Keys::Unknown, /* ACODE_META_RIGHT */
	Keys::Unknown, /* ACODE_FUNCTION */
	Keys::PrintScreen, /* ACODE_SYSRQ */
	Keys::Pause, /* ACODE_BREAK */
	Keys::Home, /* ACODE_MOVE_HOME */
	Keys::End, /* ACODE_MOVE_END */
	Keys::Insert, /* ACODE_INSERT */
	Keys::Unknown, /* ACODE_FORWARD */
	Keys::Unknown, /* ACODE_MEDIA_PLAY */
	Keys::Unknown, /* ACODE_MEDIA_PAUSE */
	Keys::Unknown, /* ACODE_MEDIA_CLOSE */
	Keys::Unknown, /* ACODE_MEDIA_EJECT */
	Keys::Unknown, /* ACODE_MEDIA_RECORD */
	Keys::F1, /* ACODE_F1 */
	Keys::F2, /* ACODE_F2 */
	Keys::F3, /* ACODE_F3 */
	Keys::F4, /* ACODE_F4 */
	Keys::F5, /* ACODE_F5 */
	Keys::F6, /* ACODE_F6 */
	Keys::F7, /* ACODE_F7 */
	Keys::F8, /* ACODE_F8 */
	Keys::F9, /* ACODE_F9 */
	Keys::F10, /* ACODE_F10 */
	Keys::F11, /* ACODE_F11 */
	Keys::F12, /* ACODE_F12 */
	Keys::NumLock, /* ACODE_NUM_LOCK */
	Keys::Key0, /* ACODE_NUMPAD_0 */
	Keys::Key1, /* ACODE_NUMPAD_1 */
	Keys::Key2, /* ACODE_NUMPAD_2 */
	Keys::Key3, /* ACODE_NUMPAD_3 */
	Keys::Key4, /* ACODE_NUMPAD_4 */
	Keys::Key5, /* ACODE_NUMPAD_5 */
	Keys::Key6, /* ACODE_NUMPAD_6 */
	Keys::Key7, /* ACODE_NUMPAD_7 */
	Keys::Key8, /* ACODE_NUMPAD_8 */
	Keys::Key9, /* ACODE_NUMPAD_9 */
	Keys::NumDiv, /* ACODE_NUMPAD_DIVIDE */
	Keys::NumMul, /* ACODE_NUMPAD_MULTIPLY */
	Keys::NumSub, /* ACODE_NUMPAD_SUBTRACT */
	Keys::NumAdd, /* ACODE_NUMPAD_ADD */
	Keys::NumPeriod, /* ACODE_NUMPAD_DOT */
	Keys::Comma, /* ACODE_NUMPAD_COMMA */
	Keys::Return, /* ACODE_NUMPAD_ENTER */
	Keys::Equals, /* ACODE_NUMPAD_EQUALS */
	Keys::Unknown, /* ACODE_NUMPAD_LEFT_PAREN */
	Keys::Unknown, /* ACODE_NUMPAD_RIGHT_PAREN */
	Keys::Unknown, /* ACODE_VOLUME_MUTE */
	Keys::Unknown, /* ACODE_INFO */
	Keys::Unknown, /* ACODE_CHANNEL_UP */
	Keys::Unknown, /* ACODE_CHANNEL_DOWN */
	Keys::Unknown, /* ACODE_ZOOM_IN */
	Keys::Unknown, /* ACODE_ZOOM_OUT */
	Keys::Unknown, /* ACODE_TV */
	Keys::Unknown, /* ACODE_WINDOW */
	Keys::Unknown, /* ACODE_GUIDE */
	Keys::Unknown, /* ACODE_DVR */
	Keys::Unknown, /* ACODE_BOOKMARK */
	Keys::Unknown, /* ACODE_CAPTIONS */
	Keys::Unknown, /* ACODE_SETTINGS */
	Keys::Unknown, /* ACODE_TV_POWER */
	Keys::Unknown, /* ACODE_TV_INPUT */
	Keys::Unknown, /* ACODE_STB_POWER */
	Keys::Unknown, /* ACODE_STB_INPUT */
	Keys::Unknown, /* ACODE_AVR_POWER */
	Keys::Unknown, /* ACODE_AVR_INPUT */
	Keys::Unknown, /* ACODE_PROG_RED */
	Keys::Unknown, /* ACODE_PROG_GREEN */
	Keys::Unknown, /* ACODE_PROG_YELLOW */
	Keys::Unknown, /* ACODE_PROG_BLUE */
	Keys::Unknown, /* ACODE_APP_SWITCH */
	Keys::Key1, /* ACODE_BUTTON_1 */
	Keys::Key2, /* ACODE_BUTTON_2 */
	Keys::Key3, /* ACODE_BUTTON_3 */
	Keys::Key4, /* ACODE_BUTTON_4 */
	Keys::Key5, /* ACODE_BUTTON_5 */
	Keys::Key6, /* ACODE_BUTTON_6 */
	Keys::Key7, /* ACODE_BUTTON_7 */
	Keys::Key8, /* ACODE_BUTTON_8 */
	Keys::Key9, /* ACODE_BUTTON_9 */
	Keys::Unknown, /* ACODE_BUTTON_10 */
	Keys::Unknown, /* ACODE_BUTTON_11 */
	Keys::Unknown, /* ACODE_BUTTON_12 */
	Keys::Unknown, /* ACODE_BUTTON_13 */
	Keys::Unknown, /* ACODE_BUTTON_14 */
	Keys::Unknown, /* ACODE_BUTTON_15 */
	Keys::Unknown, /* ACODE_BUTTON_16 */
	Keys::Unknown, /* ACODE_LANGUAGE_SWITCH */
	Keys::Unknown, /* ACODE_MANNER_MODE */
	Keys::Unknown, /* ACODE_3D_MODE */
	Keys::Unknown, /* ACODE_CONTACTS */
	Keys::Unknown, /* ACODE_CALENDAR */
	Keys::Unknown, /* ACODE_MUSIC */
	Keys::Unknown, /* ACODE_CALCULATOR */
	Keys::Unknown, /* ACODE_ZENKAKU_HANKAKU */
	Keys::Unknown, /* ACODE_EISU */
	Keys::Unknown, /* ACODE_MUHENKAN */
	Keys::Unknown, /* ACODE_HENKAN */
	Keys::Unknown, /* ACODE_KATAKANA_HIRAGANA */
	Keys::Unknown, /* ACODE_YEN */
	Keys::Unknown, /* ACODE_RO */
	Keys::Unknown, /* ACODE_KANA */
	Keys::Unknown, /* ACODE_ASSIST */
};

static int16_t cursor_x;
static int16_t cursor_y;
void ShellOS::updatePointingDeviceLocation() { _shell->updatePointerPosition(PointerLocation(cursor_x, cursor_y)); }

// Setup the callback function that handles the input.
static int32_t handle_input(struct android_app* app, AInputEvent* event)
{
	ShellOS* ourApp = static_cast<ShellOS*>(app->userData);
	Shell* theShell = ourApp ? ourApp->getShell() : 0;

	if (theShell)
	{
		switch (AInputEvent_getType(event))
		{
		case AINPUT_EVENT_TYPE_KEY: // Handle keyboard events.
		{
			int eventType = AKeyEvent_getAction(event);
			switch (eventType)
			{
			case AKEY_EVENT_ACTION_DOWN:
			{
				Keys key = keyboardKeyMap[AKeyEvent_getKeyCode(event)];
				theShell->onKeyDown(key);
			}
			break;
			case AKEY_EVENT_ACTION_UP:
			{
				Keys key = keyboardKeyMap[AKeyEvent_getKeyCode(event)];
				theShell->onKeyUp(key);
			}
			break;
			default: break;
			}
			return 1;
		}
		case AINPUT_EVENT_TYPE_MOTION: // Handle touch events.
		{
			switch (AMotionEvent_getAction(event))
			{
			case AMOTION_EVENT_ACTION_DOWN:
			{
				cursor_x = static_cast<int16_t>(AMotionEvent_getX(event, 0));
				cursor_y = static_cast<int16_t>(AMotionEvent_getY(event, 0));
				theShell->onPointingDeviceDown(0);
				break;
			}
			case AMOTION_EVENT_ACTION_MOVE:
			{
				cursor_x = static_cast<int16_t>(AMotionEvent_getX(event, 0));
				cursor_y = static_cast<int16_t>(AMotionEvent_getY(event, 0));
				break;
			}
			case AMOTION_EVENT_ACTION_UP:
			{
				cursor_x = static_cast<int16_t>(AMotionEvent_getX(event, 0));
				cursor_y = static_cast<int16_t>(AMotionEvent_getY(event, 0));
				theShell->onPointingDeviceUp(0);
				break;
			}
			}
			return 1;
		}
		}
	}

	return 1;
}

// Setup the capabilities.
const ShellOS::Capabilities ShellOS::_capabilities = { Capability::Unsupported, Capability::Unsupported };

ShellOS::ShellOS(void* hInstance, OSDATA osdata) : _instance(hInstance)
{
	_OSImplementation = std::make_unique<InternalOS>();

	android_app* state = static_cast<android_app*>(hInstance);

	if (state && !state->onInputEvent)
	{
		if (!state->userData) { state->userData = this; }

		state->onInputEvent = handle_input;
	}
}

ShellOS::~ShellOS() {}

bool ShellOS::init(DisplayAttributes& data)
{
	if (!_OSImplementation) { return false; }

	// Get PID (Process ID).
	char *pszAppName = 0, pszSrcLink[64];

	snprintf(pszSrcLink, 64, "/proc/%d/cmdline", getpid());

	FILE* pFile = fopen(pszSrcLink, "rb");

	if (pFile)
	{
		// Get the file size.
		size_t size = 256;

		pszAppName = static_cast<char*>(malloc(size));

		if (pszAppName)
		{
			char* ptr = pszAppName;
			while (fread(ptr, 1, 256, pFile) == 256)
			{
				char* resized = static_cast<char*>(realloc(pszAppName, size + 256));

				if (!resized)
				{
					free(pszAppName);
					pszAppName = 0;
					break;
				}
				size += 256;
				pszAppName = resized;
				ptr = pszAppName + size;
			}
		}

		fclose(pFile);
	}

	if (!pszAppName) { Log(LogLevel::Debug, "Warning: Unable to set app name.\n"); }
	else
	{
		_appName = pszAppName;
		free(pszAppName);
	}

	// Setup the read/write path.
	android_app* instance = static_cast<android_app*>(_instance);
	// Construct the binary path for GetReadPath() and GetWritePath().
	const char* internalDataPath = static_cast<const char*>(instance->activity->internalDataPath);

	if (!internalDataPath) // Due to a bug in Gingerbread this may be null.
	{
		Log(LogLevel::Debug, "Warning: The internal data path returned from Android is null. Attempting to generate from the app name..\n");

		char* dataPath = 0;

		if (!_appName.empty())
		{
			size_t size = strlen("/data/data/") + _appName.length() + 2;

			dataPath = static_cast<char*>(malloc(size));

			if (dataPath) { snprintf(dataPath, size, "/data/data/%s/", _appName.c_str()); }
		}

		if (!dataPath) { dataPath = const_cast<char*>("/sdcard/"); }

		_writePath = dataPath;

		if (!_appName.empty()) { free(dataPath); }
	}
	else
	{
		_writePath = internalDataPath;
	}

	_readPaths.clear();
	_readPaths.push_back(_writePath);
	return true;
}

bool ShellOS::initializeWindow(DisplayAttributes& data)
{
	android_app* instance = static_cast<android_app*>(_instance);
	if (!instance->window) { return false; }

	data.fullscreen = true;
	data.x = instance->contentRect.left;
	data.y = instance->contentRect.bottom;
	data.width = instance->contentRect.right - data.x;
	data.height = instance->contentRect.top - data.y;
	return true;
}

void ShellOS::releaseWindow() {}

OSApplication ShellOS::getApplication() const { return _instance; }

OSConnection ShellOS::getConnection() const { return nullptr; }

OSDisplay ShellOS::getDisplay() const { return nullptr; }

OSWindow ShellOS::getWindow() const { return static_cast<android_app*>(_instance)->window; }

bool ShellOS::handleOSEvents()
{
	// The OS events for Android are already handled externally.
	return true;
}

bool ShellOS::popUpMessage(const char* title, const char* message, ...) const
{
	if (!title && !message && _instance) { return false; }

	bool result = false;

	ANativeActivity* activity = static_cast<android_app*>(_instance)->activity;

	if (activity)
	{
		JNIEnv* env;
		activity->vm->AttachCurrentThread(&env, NULL);

		if (env)
		{
			jclass clazz = env->GetObjectClass(activity->clazz);
			jmethodID methodID = env->GetMethodID(clazz, "displayExitMessage", "(Ljava/lang/String;)V");

			if (methodID)
			{
				va_list arg;
				char buf[1024];

				va_start(arg, message);
				vsnprintf(buf, 1024, message, arg);
				va_end(arg);

				jstring exitMsg = env->NewStringUTF(buf);
				env->CallVoidMethod(activity->clazz, methodID, exitMsg);
				result = true;
			}
		}

		activity->vm->DetachCurrentThread();
	}

	return result;
}
} // namespace platform
} // namespace pvr
//!\endcond