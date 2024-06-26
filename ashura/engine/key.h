#pragma once

#include "ashura/std/types.h"

namespace ash
{

enum class KeyModifiers : u32
{
  None       = 0x0000,
  LeftShift  = 0x0001,
  RightShift = 0x0002,
  LeftCtrl   = 0x0004,
  RightCtrl  = 0x0008,
  LeftAlt    = 0x0010,
  RightAlt   = 0x0020,
  LeftWin    = 0x0040,
  RightWin   = 0x0080,
  Num        = 0x0100,
  Caps       = 0x0200,
  AltGr      = 0x0400,
  ScrollLock = 0x0800,
  Ctrl       = 0x1000,
  Shift      = 0x2000,
  Alt        = 0x4000,
  Gui        = 0x8000,
  All        = 0xFFFF
};

ASH_DEFINE_ENUM_BIT_OPS(KeyModifiers)

/// https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input?redirectedfrom=MSDN#_win32_Keyboard_Input_Model
enum Key : u32
{
  KEY_UNKNOWN            = 0,
  KEY_RETURN             = '\r',
  KEY_ESCAPE             = '\x1B',
  KEY_BACKSPACE          = '\b',
  KEY_TAB                = '\t',
  KEY_SPACE              = ' ',
  KEY_EXCLAIM            = '!',
  KEY_QUOTEDBL           = '"',
  KEY_HASH               = '#',
  KEY_PERCENT            = '%',
  KEY_DOLLAR             = '$',
  KEY_AMPERSAND          = '&',
  KEY_QUOTE              = '\'',
  KEY_LEFTPAREN          = '(',
  KEY_RIGHTPAREN         = ')',
  KEY_ASTERISK           = '*',
  KEY_PLUS               = '+',
  KEY_COMMA              = ',',
  KEY_MINUS              = '-',
  KEY_PERIOD             = '.',
  KEY_SLASH              = '/',
  KEY_NUM_0              = '0',
  KEY_NUM_1              = '1',
  KEY_NUM_2              = '2',
  KEY_NUM_3              = '3',
  KEY_NUM_4              = '4',
  KEY_NUM_5              = '5',
  KEY_NUM_6              = '6',
  KEY_NUM_7              = '7',
  KEY_NUM_8              = '8',
  KEY_NUM_9              = '9',
  KEY_COLON              = ':',
  KEY_SEMICOLON          = ';',
  KEY_LESS               = '<',
  KEY_EQUALS             = '=',
  KEY_GREATER            = '>',
  KEY_QUESTION           = '?',
  KEY_AT                 = '@',
  KEY_LEFTBRACKET        = '[',
  KEY_BACKSLASH          = '\\',
  KEY_RIGHTBRACKET       = ']',
  KEY_CARET              = '^',
  KEY_UNDERSCORE         = '_',
  KEY_BACKQUOTE          = '`',
  KEY_a                  = 'a',
  KEY_b                  = 'b',
  KEY_c                  = 'c',
  KEY_d                  = 'd',
  KEY_e                  = 'e',
  KEY_f                  = 'f',
  KEY_g                  = 'g',
  KEY_h                  = 'h',
  KEY_i                  = 'i',
  KEY_j                  = 'j',
  KEY_k                  = 'k',
  KEY_l                  = 'l',
  KEY_m                  = 'm',
  KEY_n                  = 'n',
  KEY_o                  = 'o',
  KEY_p                  = 'p',
  KEY_q                  = 'q',
  KEY_r                  = 'r',
  KEY_s                  = 's',
  KEY_t                  = 't',
  KEY_u                  = 'u',
  KEY_v                  = 'v',
  KEY_w                  = 'w',
  KEY_x                  = 'x',
  KEY_y                  = 'y',
  KEY_z                  = 'z',
  KEY_CAPSLOCK           = 0x40000000 | 57,
  KEY_F1                 = 0x40000000 | 58,
  KEY_F2                 = 0x40000000 | 59,
  KEY_F3                 = 0x40000000 | 60,
  KEY_F4                 = 0x40000000 | 61,
  KEY_F5                 = 0x40000000 | 62,
  KEY_F6                 = 0x40000000 | 63,
  KEY_F7                 = 0x40000000 | 64,
  KEY_F8                 = 0x40000000 | 65,
  KEY_F9                 = 0x40000000 | 66,
  KEY_F10                = 0x40000000 | 67,
  KEY_F11                = 0x40000000 | 68,
  KEY_F12                = 0x40000000 | 69,
  KEY_PRINTSCREEN        = 0x40000000 | 70,
  KEY_SCROLLLOCK         = 0x40000000 | 71,
  KEY_PAUSE              = 0x40000000 | 72,
  KEY_INSERT             = 0x40000000 | 73,
  KEY_HOME               = 0x40000000 | 74,
  KEY_PAGEUP             = 0x40000000 | 75,
  KEY_DELETE             = 0x40000000 | 76,
  KEY_END                = 0x40000000 | 77,
  KEY_PAGEDOWN           = 0x40000000 | 78,
  KEY_RIGHT              = 0x40000000 | 79,
  KEY_LEFT               = 0x40000000 | 80,
  KEY_DOWN               = 0x40000000 | 81,
  KEY_UP                 = 0x40000000 | 82,
  KEY_NUMLOCKCLEAR       = 0x40000000 | 83,
  KEY_KP_DIVIDE          = 0x40000000 | 84,
  KEY_KP_MULTIPLY        = 0x40000000 | 85,
  KEY_KP_MINUS           = 0x40000000 | 86,
  KEY_KP_PLUS            = 0x40000000 | 87,
  KEY_KP_ENTER           = 0x40000000 | 88,
  KEY_KP_1               = 0x40000000 | 89,
  KEY_KP_2               = 0x40000000 | 90,
  KEY_KP_3               = 0x40000000 | 91,
  KEY_KP_4               = 0x40000000 | 92,
  KEY_KP_5               = 0x40000000 | 93,
  KEY_KP_6               = 0x40000000 | 94,
  KEY_KP_7               = 0x40000000 | 95,
  KEY_KP_8               = 0x40000000 | 96,
  KEY_KP_9               = 0x40000000 | 97,
  KEY_KP_0               = 0x40000000 | 98,
  KEY_KP_PERIOD          = 0x40000000 | 99,
  KEY_APPLICATION        = 0x40000000 | 101,
  KEY_POWER              = 0x40000000 | 102,
  KEY_KP_EQUALS          = 0x40000000 | 103,
  KEY_F13                = 0x40000000 | 104,
  KEY_F14                = 0x40000000 | 105,
  KEY_F15                = 0x40000000 | 106,
  KEY_F16                = 0x40000000 | 107,
  KEY_F17                = 0x40000000 | 108,
  KEY_F18                = 0x40000000 | 109,
  KEY_F19                = 0x40000000 | 110,
  KEY_F20                = 0x40000000 | 111,
  KEY_F21                = 0x40000000 | 112,
  KEY_F22                = 0x40000000 | 113,
  KEY_F23                = 0x40000000 | 114,
  KEY_F24                = 0x40000000 | 115,
  KEY_EXECUTE            = 0x40000000 | 116,
  KEY_HELP               = 0x40000000 | 117,
  KEY_MENU               = 0x40000000 | 118,
  KEY_SELECT             = 0x40000000 | 119,
  KEY_STOP               = 0x40000000 | 120,
  KEY_AGAIN              = 0x40000000 | 121,
  KEY_UNDO               = 0x40000000 | 122,
  KEY_CUT                = 0x40000000 | 123,
  KEY_COPY               = 0x40000000 | 124,
  KEY_PASTE              = 0x40000000 | 125,
  KEY_FIND               = 0x40000000 | 126,
  KEY_MUTE               = 0x40000000 | 127,
  KEY_VOLUMEUP           = 0x40000000 | 128,
  KEY_VOLUMEDOWN         = 0x40000000 | 129,
  KEY_KP_COMMA           = 0x40000000 | 133,
  KEY_KP_EQUALSAS400     = 0x40000000 | 134,
  KEY_ALTERASE           = 0x40000000 | 153,
  KEY_SYSREQ             = 0x40000000 | 154,
  KEY_CANCEL             = 0x40000000 | 155,
  KEY_CLEAR              = 0x40000000 | 156,
  KEY_PRIOR              = 0x40000000 | 157,
  KEY_RETURN2            = 0x40000000 | 158,
  KEY_SEPARATOR          = 0x40000000 | 159,
  KEY_OUT                = 0x40000000 | 160,
  KEY_OPER               = 0x40000000 | 161,
  KEY_CLEARAGAIN         = 0x40000000 | 162,
  KEY_CRSEL              = 0x40000000 | 163,
  KEY_EXSEL              = 0x40000000 | 164,
  KEY_KP_00              = 0x40000000 | 176,
  KEY_KP_000             = 0x40000000 | 177,
  KEY_THOUSANDSSEPARATOR = 0x40000000 | 178,
  KEY_DECIMALSEPARATOR   = 0x40000000 | 179,
  KEY_CURRENCYUNIT       = 0x40000000 | 180,
  KEY_CURRENCYSUBUNIT    = 0x40000000 | 181,
  KEY_KP_LEFTPAREN       = 0x40000000 | 182,
  KEY_KP_RIGHTPAREN      = 0x40000000 | 183,
  KEY_KP_LEFTBRACE       = 0x40000000 | 184,
  KEY_KP_RIGHTBRACE      = 0x40000000 | 185,
  KEY_KP_TAB             = 0x40000000 | 186,
  KEY_KP_BACKSPACE       = 0x40000000 | 187,
  KEY_KP_A               = 0x40000000 | 188,
  KEY_KP_B               = 0x40000000 | 189,
  KEY_KP_C               = 0x40000000 | 190,
  KEY_KP_D               = 0x40000000 | 191,
  KEY_KP_E               = 0x40000000 | 192,
  KEY_KP_F               = 0x40000000 | 193,
  KEY_KP_XOR             = 0x40000000 | 194,
  KEY_KP_POWER           = 0x40000000 | 195,
  KEY_KP_PERCENT         = 0x40000000 | 196,
  KEY_KP_LESS            = 0x40000000 | 197,
  KEY_KP_GREATER         = 0x40000000 | 198,
  KEY_KP_AMPERSAND       = 0x40000000 | 199,
  KEY_KP_DBLAMPERSAND    = 0x40000000 | 200,
  KEY_KP_VERTICALBAR     = 0x40000000 | 201,
  KEY_KP_DBLVERTICALBAR  = 0x40000000 | 202,
  KEY_KP_COLON           = 0x40000000 | 203,
  KEY_KP_HASH            = 0x40000000 | 204,
  KEY_KP_SPACE           = 0x40000000 | 205,
  KEY_KP_AT              = 0x40000000 | 206,
  KEY_KP_EXCLAM          = 0x40000000 | 207,
  KEY_KP_MEMSTORE        = 0x40000000 | 208,
  KEY_KP_MEMRECALL       = 0x40000000 | 209,
  KEY_KP_MEMCLEAR        = 0x40000000 | 210,
  KEY_KP_MEMADD          = 0x40000000 | 211,
  KEY_KP_MEMSUBTRACT     = 0x40000000 | 212,
  KEY_KP_MEMMULTIPLY     = 0x40000000 | 213,
  KEY_KP_MEMDIVIDE       = 0x40000000 | 214,
  KEY_KP_PLUSMINUS       = 0x40000000 | 215,
  KEY_KP_CLEAR           = 0x40000000 | 216,
  KEY_KP_CLEARENTRY      = 0x40000000 | 217,
  KEY_KP_BINARY          = 0x40000000 | 218,
  KEY_KP_OCTAL           = 0x40000000 | 219,
  KEY_KP_DECIMAL         = 0x40000000 | 220,
  KEY_KP_HEXADECIMAL     = 0x40000000 | 221,
  KEY_LCTRL              = 0x40000000 | 224,
  KEY_LSHIFT             = 0x40000000 | 225,
  KEY_LALT               = 0x40000000 | 226,
  KEY_LGUI               = 0x40000000 | 227,
  KEY_RCTRL              = 0x40000000 | 228,
  KEY_RSHIFT             = 0x40000000 | 229,
  KEY_RALT               = 0x40000000 | 230,
  KEY_RGUI               = 0x40000000 | 231,
  KEY_MODE               = 0x40000000 | 257,
  KEY_AUDIONEXT          = 0x40000000 | 258,
  KEY_AUDIOPREV          = 0x40000000 | 259,
  KEY_AUDIOSTOP          = 0x40000000 | 260,
  KEY_AUDIOPLAY          = 0x40000000 | 261,
  KEY_AUDIOMUTE          = 0x40000000 | 262,
  KEY_MEDIASELECT        = 0x40000000 | 263,
  KEY_WWW                = 0x40000000 | 264,
  KEY_MAIL               = 0x40000000 | 265,
  KEY_CALCULATOR         = 0x40000000 | 266,
  KEY_COMPUTER           = 0x40000000 | 267,
  KEY_AC_SEARCH          = 0x40000000 | 268,
  KEY_AC_HOME            = 0x40000000 | 269,
  KEY_AC_BACK            = 0x40000000 | 270,
  KEY_AC_FORWARD         = 0x40000000 | 271,
  KEY_AC_STOP            = 0x40000000 | 272,
  KEY_AC_REFRESH         = 0x40000000 | 273,
  KEY_AC_BOOKMARKS       = 0x40000000 | 274,
  KEY_BRIGHTNESSDOWN     = 0x40000000 | 275,
  KEY_BRIGHTNESSUP       = 0x40000000 | 276,
  KEY_DISPLAYSWITCH      = 0x40000000 | 277,
  KEY_KBDILLUMTOGGLE     = 0x40000000 | 278,
  KEY_KBDILLUMDOWN       = 0x40000000 | 279,
  KEY_KBDILLUMUP         = 0x40000000 | 280,
  KEY_EJECT              = 0x40000000 | 281,
  KEY_SLEEP              = 0x40000000 | 282,
  KEY_APP1               = 0x40000000 | 283,
  KEY_APP2               = 0x40000000 | 284,
  KEY_AUDIOREWIND        = 0x40000000 | 285,
  KEY_AUDIOFASTFORWARD   = 0x40000000 | 286,
  KEY_SOFTLEFT           = 0x40000000 | 287,
  KEY_SOFTRIGHT          = 0x40000000 | 288,
  KEY_CALL               = 0x40000000 | 289,
  KEY_ENDCAL             = 0x40000000 | 290
};

enum class MouseButtons : u32
{
  None      = 0x00,
  Primary   = 0x01,
  Secondary = 0x02,
  Middle    = 0x04,
  A1        = 0x08,
  A2        = 0x10,
  A3        = 0x20,
  A4        = 0x40,
  A5        = 0x80,
  All       = 0xFFFFFFFF
};

ASH_DEFINE_ENUM_BIT_OPS(MouseButtons)

}        // namespace ash
