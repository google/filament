/*!
\brief Contains a common implementation for pvr::platform::InternalOS specifically for Linux platforms.
\file PVRShell/OS/Linux/InternalOS.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRShell/OS/ShellOS.h"

namespace pvr {
namespace platform {

// When using termios keypresses are reported as their ASCII values directly
// Most keys translate directly to the characters they represent
static Keys ASCIIStandardKeyMap[128] = {
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, /* 0   */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Backspace, Keys::Tab, /* 5   */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Return, Keys::Unknown, /* 10  */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, /* 15  */
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, /* 20  */
	Keys::Unknown, Keys::Unknown, Keys::Escape, Keys::Unknown, Keys::Unknown, /* 25  */
	Keys::Unknown, Keys::Unknown, Keys::Space, Keys::Key1, Keys::Quote, /* 30  */
	Keys::Backslash, Keys::Key4, Keys::Key5, Keys::Key7, Keys::Quote, /* 35  */
	Keys::Key9, Keys::Key0, Keys::NumMul, Keys::NumAdd, Keys::Comma, /* 40  */
	Keys::Minus, Keys::Period, Keys::Slash, Keys::Key0, Keys::Key1, /* 45  */
	Keys::Key2, Keys::Key3, Keys::Key4, Keys::Key5, Keys::Key6, /* 50  */
	Keys::Key7, Keys::Key8, Keys::Key9, Keys::Semicolon, Keys::Semicolon, /* 55  */
	Keys::Comma, Keys::Equals, Keys::Period, Keys::Slash, Keys::Key2, /* 60  */
	Keys::A, Keys::B, Keys::C, Keys::D, Keys::E, /* upper case */ /* 65  */
	Keys::F, Keys::G, Keys::H, Keys::I, Keys::J, /* 70  */
	Keys::K, Keys::L, Keys::M, Keys::N, Keys::O, /* 75  */
	Keys::P, Keys::Q, Keys::R, Keys::S, Keys::T, /* 80  */
	Keys::U, Keys::V, Keys::W, Keys::X, Keys::Y, /* 85  */
	Keys::Z, Keys::SquareBracketLeft, Keys::Backslash, Keys::SquareBracketRight, Keys::Key6, /* 90  */
	Keys::Minus, Keys::Backquote, Keys::A, Keys::B, Keys::C, /* 95  */
	Keys::D, Keys::E, Keys::F, Keys::G, Keys::H, /* lower case */ /* 100 */
	Keys::I, Keys::J, Keys::K, Keys::L, Keys::M, /* 105 */
	Keys::N, Keys::O, Keys::P, Keys::Q, Keys::R, /* 110 */
	Keys::S, Keys::T, Keys::U, Keys::V, Keys::W, /* 115 */
	Keys::X, Keys::Y, Keys::Z, Keys::SquareBracketLeft, Keys::Backslash, /* 120 */
	Keys::SquareBracketRight, Keys::Backquote, Keys::Backspace /* 125 */
};

// Provides a mapping between a special key code combination with the associated Key
struct SpecialKeyCode
{
	const char* str;
	Keys key;
};

// Some codes for F-keys can differ, depending on whether we are reading a
// /dev/tty from within X or from a text console.
// Some keys (e.g. Home, Delete) have multiple codes one for the standard version and one for the numpad version.
static SpecialKeyCode ASCIISpecialKeyMap[] = { { "[A", Keys::Up }, { "[B", Keys::Down }, { "[C", Keys::Right }, { "[D", Keys::Left },
	{ "[E", Keys::Key5 }, // Numpad 5 has no second function - do this to avoid the code being interpreted as Escape.
	{ "OP", Keys::F1 }, // Within X
	{ "[[A", Keys::F1 }, // Text console
	{ "OQ", Keys::F2 }, // Within X
	{ "[[B", Keys::F2 }, // Text console
	{ "OR", Keys::F3 }, // Within X
	{ "[[C", Keys::F3 }, // Text console
	{ "OS", Keys::F4 }, // Within X
	{ "[[D", Keys::F4 }, // Text console
	{ "[15~", Keys::F5 }, // Within X
	{ "[[E", Keys::F5 }, // Text console
	{ "[17~", Keys::F6 }, { "[18~", Keys::F7 }, { "[19~", Keys::F8 }, { "[20~", Keys::F9 }, { "[21~", Keys::F10 }, { "[23~", Keys::F11 }, { "[24~", Keys::F12 },
	{ "[1~", Keys::Home }, { "OH", Keys::Home }, { "[2~", Keys::Insert }, { "[3~", Keys::Delete }, { "[4~", Keys::End }, { "OF", Keys::End }, { "[5~", Keys::PageUp },
	{ "[6~", Keys::PageDown }, { NULL, Keys::Unknown } };

// This mapping is taken from input-event-codes.h - see http://www.usb.org/developers/hidpage
static Keys KeyboardKeyMap[] = { Keys::Unknown, Keys::Escape, Keys::Key1, Keys::Key2, Keys::Key3, Keys::Key4, Keys::Key5, Keys::Key6, Keys::Key7, Keys::Key8, Keys::Key9,
	Keys::Key0, Keys::Minus, Keys::Equals, Keys::Backspace, Keys::Tab, Keys::Q, Keys::W, Keys::E, Keys::R, Keys::T, Keys::Y, Keys::U, Keys::I, Keys::O, Keys::P,
	Keys::SquareBracketLeft, Keys::SquareBracketRight, Keys::Return, Keys::Control, Keys::A, Keys::S, Keys::D, Keys::F, Keys::G, Keys::H, Keys::J, Keys::K, Keys::L,
	Keys::Semicolon, Keys::Quote, Keys::Backquote, Keys::Shift, Keys::Backslash, Keys::Z, Keys::X, Keys::C, Keys::V, Keys::B, Keys::N, Keys::M, Keys::Comma, Keys::Period,
	Keys::Slash, Keys::Shift, Keys::NumMul, Keys::Alt, Keys::Space, Keys::CapsLock, Keys::F1, Keys::F2, Keys::F3, Keys::F4, Keys::F5, Keys::F6, Keys::F7, Keys::F8, Keys::F9,
	Keys::F10, Keys::NumLock, Keys::ScrollLock, Keys::Num7, Keys::Num8, Keys::Num9, Keys::NumSub, Keys::Num4, Keys::Num5, Keys::Num6, Keys::NumAdd, Keys::Num1, Keys::Num2,
	Keys::Num3, Keys::Num0, Keys::NumPeriod, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::F11, Keys::F12, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown,
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Return, Keys::Control, Keys::NumDiv, Keys::PrintScreen, Keys::Alt, Keys::Unknown, Keys::Home, Keys::Up, Keys::PageUp,
	Keys::Left, Keys::Right, Keys::End, Keys::Down, Keys::PageDown, Keys::Insert, Keys::Delete, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown,
	Keys::Unknown, Keys::Unknown, Keys::Pause, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::SystemKey1, Keys::SystemKey1, Keys::SystemKey2,
	Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown, Keys::Unknown };

class InternalOS
{
public:
	InternalOS(ShellOS* shellOS);
	virtual ~InternalOS();

	void setIsInitialized(bool isInitialized) { _isInitialized = isInitialized; }

	ShellOS* getShellOS() { return _shellOS; }

	const ShellOS* getShellOS() const { return _shellOS; }

	bool isInitialized() { return _isInitialized; }

	virtual bool handleOSEvents(std::unique_ptr<Shell>& shell);

	Keys getKeyFromAscii(unsigned char initialKey);
	Keys getKeyFromEVCode(uint32_t keycode);

private:
	bool _isInitialized;
	ShellOS* _shellOS;

	Keys getSpecialKey(unsigned char firstCharacter) const;
};
} // namespace platform
} // namespace pvr
