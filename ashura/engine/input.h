/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/math.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

enum class SystemTheme : u8
{
  None    = 0,
  Unknown = 1,
  Light   = 2,
  Dark    = 3
};

enum class KeyAction : u8
{
  None    = 0,
  Press   = 1,
  Release = 2
};

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

/// Scan Codes vs Key Codes:
/// https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input?redirectedfrom=MSDN#_win32_Keyboard_Input_Model
enum class ScanCode : u16
{
  None               = 0,
  Return             = '\r',
  Escape             = '\x1B',
  Backspace          = '\b',
  Tab                = '\t',
  Space              = ' ',
  Exclaim            = '!',
  QuoteDbl           = '"',
  Hash               = '#',
  Percent            = '%',
  Dollar             = '$',
  Ampersand          = '&',
  Quote              = '\'',
  LeftParen          = '(',
  RightParen         = ')',
  Asterisk           = '*',
  Plus               = '+',
  Comma              = ',',
  Minus              = '-',
  Period             = '.',
  Slash              = '/',
  Num0               = '0',
  Num1               = '1',
  Num2               = '2',
  Num3               = '3',
  Num4               = '4',
  Num5               = '5',
  Num6               = '6',
  Num7               = '7',
  Num8               = '8',
  Num9               = '9',
  Colon              = ':',
  SemiColon          = ';',
  Less               = '<',
  Equals             = '=',
  Greater            = '>',
  Question           = '?',
  At                 = '@',
  LeftBracket        = '[',
  BackSlash          = '\\',
  RightBracket       = ']',
  Caret              = '^',
  Underscore         = '_',
  BackQuote          = '`',
  A                  = 'a',
  B                  = 'b',
  C                  = 'c',
  D                  = 'd',
  E                  = 'e',
  F                  = 'f',
  G                  = 'g',
  H                  = 'h',
  I                  = 'i',
  J                  = 'j',
  K                  = 'k',
  L                  = 'l',
  M                  = 'm',
  N                  = 'n',
  O                  = 'o',
  P                  = 'p',
  Q                  = 'q',
  R                  = 'r',
  S                  = 's',
  T                  = 't',
  U                  = 'u',
  V                  = 'v',
  W                  = 'w',
  X                  = 'x',
  Y                  = 'y',
  Z                  = 'z',
  CapsLock           = 57,
  F1                 = 58,
  F2                 = 59,
  F3                 = 60,
  F4                 = 61,
  F5                 = 62,
  F6                 = 63,
  F7                 = 64,
  F8                 = 65,
  F9                 = 66,
  F10                = 67,
  F11                = 68,
  F12                = 69,
  PrintScreen        = 70,
  ScrollLock         = 71,
  Pause              = 72,
  Insert             = 73,
  Home               = 74,
  PageUp             = 75,
  Delete             = 76,
  End                = 77,
  PageDown           = 78,
  Right              = 79,
  Left               = 80,
  Down               = 81,
  Up                 = 82,
  NumLockClear       = 83,
  KP_Divide          = 84,
  KP_Multiply        = 85,
  KP_Minus           = 86,
  KP_Plus            = 87,
  KP_Enter           = 88,
  KP_1               = 89,
  KP_2               = 90,
  KP_3               = 91,
  KP_4               = 92,
  KP_5               = 93,
  KP_6               = 94,
  KP_7               = 95,
  KP_8               = 96,
  KP_9               = 97,
  KP_0               = 98,
  KP_Period          = 99,
  Application        = 101,
  Power              = 102,
  KP_Equals          = 103,
  F13                = 104,
  F14                = 105,
  F15                = 106,
  F16                = 107,
  F17                = 108,
  F18                = 109,
  F19                = 110,
  F20                = 111,
  F21                = 112,
  F22                = 113,
  F23                = 114,
  F24                = 115,
  Execute            = 116,
  Help               = 117,
  Menu               = 118,
  Select             = 119,
  Stop               = 120,
  Again              = 121,
  Undo               = 122,
  Cut                = 123,
  Copy               = 124,
  Paste              = 125,
  Find               = 126,
  Mute               = 127,
  VolumeUp           = 128,
  VolumeDown         = 129,
  KP_Comma           = 133,
  KP_EqualsAs400     = 134,
  AltErase           = 153,
  SysReq             = 154,
  Cancel             = 155,
  Clear              = 156,
  Prior              = 157,
  Return2            = 158,
  Separator          = 159,
  Out                = 160,
  Oper               = 161,
  ClearAgain         = 162,
  CrSel              = 163,
  ExSel              = 164,
  KP_00              = 176,
  KP_000             = 177,
  ThousandsSeparator = 178,
  DecimalSeparator   = 179,
  CurrencyUnit       = 180,
  CurrencySubunit    = 181,
  KP_LeftParen       = 182,
  KP_RightParen      = 183,
  KP_LeftBrace       = 184,
  KP_RightBrace      = 185,
  KP_Tab             = 186,
  KP_BackSpace       = 187,
  KP_A               = 188,
  KP_B               = 189,
  KP_C               = 190,
  KP_D               = 191,
  KP_E               = 192,
  KP_F               = 193,
  KP_Xor             = 194,
  KP_Power           = 195,
  KP_Percent         = 196,
  KP_Less            = 197,
  KP_Greater         = 198,
  KP_Ampersand       = 199,
  KP_DblAmpersanD    = 200,
  KP_VerticalBar     = 201,
  KP_DblVerticalBar  = 202,
  KP_Colon           = 203,
  KP_Hash            = 204,
  KP_Space           = 205,
  KP_At              = 206,
  KP_Exclam          = 207,
  KP_MemStore        = 208,
  KP_MemRecall       = 209,
  KP_MemClear        = 210,
  KP_MemAdd          = 211,
  KP_MemSubtract     = 212,
  KP_MemMultiply     = 213,
  KP_MemDivide       = 214,
  KP_PlusMinus       = 215,
  KP_Clear           = 216,
  KP_Clearentry      = 217,
  KP_Binary          = 218,
  KP_Octal           = 219,
  KP_Decimal         = 220,
  KP_HexaDecimal     = 221,
  LCtrl              = 224,
  LShift             = 225,
  LAlt               = 226,
  LGui               = 227,
  RCtrl              = 228,
  RShift             = 229,
  RAlt               = 230,
  RGui               = 231,
  Mode               = 257,
  AudioNext          = 258,
  AudioPrev          = 259,
  AudioStop          = 260,
  AudioPlay          = 261,
  AudioMute          = 262,
  MediaSelect        = 263,
  WWW                = 264,
  Mail               = 265,
  Calculator         = 266,
  Computer           = 267,
  AC_Search          = 268,
  AC_Home            = 269,
  AC_Back            = 270,
  AC_Forward         = 271,
  AC_Stop            = 272,
  AC_Refresh         = 273,
  AC_Bookmarks       = 274,
  BrightnessDown     = 275,
  BrightnessUp       = 276,
  DisplaySwitch      = 277,
  KbdillumToggle     = 278,
  KbdillumDown       = 279,
  KbdillumUp         = 280,
  Eject              = 281,
  Sleep              = 282,
  App1               = 283,
  App2               = 284,
  AudioRewind        = 285,
  AudioFastForward   = 286,
  SoftLeft           = 287,
  SoftRight          = 288,
  Call               = 289,
  EndCall            = 290
};

enum class KeyCode : u16
{
  None               = 0,
  Return             = '\r',
  Escape             = '\x1B',
  Backspace          = '\b',
  Tab                = '\t',
  Space              = ' ',
  Exclaim            = '!',
  QuoteDbl           = '"',
  Hash               = '#',
  Percent            = '%',
  Dollar             = '$',
  Ampersand          = '&',
  Quote              = '\'',
  LeftParen          = '(',
  RightParen         = ')',
  Asterisk           = '*',
  Plus               = '+',
  Comma              = ',',
  Minus              = '-',
  Period             = '.',
  Slash              = '/',
  Num0               = '0',
  Num1               = '1',
  Num2               = '2',
  Num3               = '3',
  Num4               = '4',
  Num5               = '5',
  Num6               = '6',
  Num7               = '7',
  Num8               = '8',
  Num9               = '9',
  Colon              = ':',
  SemiColon          = ';',
  Less               = '<',
  Equals             = '=',
  Greater            = '>',
  Question           = '?',
  At                 = '@',
  LeftBracket        = '[',
  BackSlash          = '\\',
  RightBracket       = ']',
  Caret              = '^',
  Underscore         = '_',
  BackQuote          = '`',
  A                  = 'a',
  B                  = 'b',
  C                  = 'c',
  D                  = 'd',
  E                  = 'e',
  F                  = 'f',
  G                  = 'g',
  H                  = 'h',
  I                  = 'i',
  J                  = 'j',
  K                  = 'k',
  L                  = 'l',
  M                  = 'm',
  N                  = 'n',
  O                  = 'o',
  P                  = 'p',
  Q                  = 'q',
  R                  = 'r',
  S                  = 's',
  T                  = 't',
  U                  = 'u',
  V                  = 'v',
  W                  = 'w',
  X                  = 'x',
  Y                  = 'y',
  Z                  = 'z',
  CapsLock           = 57,
  F1                 = 58,
  F2                 = 59,
  F3                 = 60,
  F4                 = 61,
  F5                 = 62,
  F6                 = 63,
  F7                 = 64,
  F8                 = 65,
  F9                 = 66,
  F10                = 67,
  F11                = 68,
  F12                = 69,
  PrintScreen        = 70,
  ScrollLock         = 71,
  Pause              = 72,
  Insert             = 73,
  Home               = 74,
  PageUp             = 75,
  Delete             = 76,
  End                = 77,
  PageDown           = 78,
  Right              = 79,
  Left               = 80,
  Down               = 81,
  Up                 = 82,
  NumLockClear       = 83,
  KP_Divide          = 84,
  KP_Multiply        = 85,
  KP_Minus           = 86,
  KP_Plus            = 87,
  KP_Enter           = 88,
  KP_1               = 89,
  KP_2               = 90,
  KP_3               = 91,
  KP_4               = 92,
  KP_5               = 93,
  KP_6               = 94,
  KP_7               = 95,
  KP_8               = 96,
  KP_9               = 97,
  KP_0               = 98,
  KP_Period          = 99,
  Application        = 101,
  Power              = 102,
  KP_Equals          = 103,
  F13                = 104,
  F14                = 105,
  F15                = 106,
  F16                = 107,
  F17                = 108,
  F18                = 109,
  F19                = 110,
  F20                = 111,
  F21                = 112,
  F22                = 113,
  F23                = 114,
  F24                = 115,
  Execute            = 116,
  Help               = 117,
  Menu               = 118,
  Select             = 119,
  Stop               = 120,
  Again              = 121,
  Undo               = 122,
  Cut                = 123,
  Copy               = 124,
  Paste              = 125,
  Find               = 126,
  Mute               = 127,
  VolumeUp           = 128,
  VolumeDown         = 129,
  KP_Comma           = 133,
  KP_EqualsAs400     = 134,
  AltErase           = 153,
  SysReq             = 154,
  Cancel             = 155,
  Clear              = 156,
  Prior              = 157,
  Return2            = 158,
  Separator          = 159,
  Out                = 160,
  Oper               = 161,
  ClearAgain         = 162,
  CrSel              = 163,
  ExSel              = 164,
  KP_00              = 176,
  KP_000             = 177,
  ThousandsSeparator = 178,
  DecimalSeparator   = 179,
  CurrencyUnit       = 180,
  CurrencySubunit    = 181,
  KP_LeftParen       = 182,
  KP_RightParen      = 183,
  KP_LeftBrace       = 184,
  KP_RightBrace      = 185,
  KP_Tab             = 186,
  KP_BackSpace       = 187,
  KP_A               = 188,
  KP_B               = 189,
  KP_C               = 190,
  KP_D               = 191,
  KP_E               = 192,
  KP_F               = 193,
  KP_Xor             = 194,
  KP_Power           = 195,
  KP_Percent         = 196,
  KP_Less            = 197,
  KP_Greater         = 198,
  KP_Ampersand       = 199,
  KP_DblAmpersanD    = 200,
  KP_VerticalBar     = 201,
  KP_DblVerticalBar  = 202,
  KP_Colon           = 203,
  KP_Hash            = 204,
  KP_Space           = 205,
  KP_At              = 206,
  KP_Exclam          = 207,
  KP_MemStore        = 208,
  KP_MemRecall       = 209,
  KP_MemClear        = 210,
  KP_MemAdd          = 211,
  KP_MemSubtract     = 212,
  KP_MemMultiply     = 213,
  KP_MemDivide       = 214,
  KP_PlusMinus       = 215,
  KP_Clear           = 216,
  KP_Clearentry      = 217,
  KP_Binary          = 218,
  KP_Octal           = 219,
  KP_Decimal         = 220,
  KP_HexaDecimal     = 221,
  LCtrl              = 224,
  LShift             = 225,
  LAlt               = 226,
  LGui               = 227,
  RCtrl              = 228,
  RShift             = 229,
  RAlt               = 230,
  RGui               = 231,
  Mode               = 257,
  AudioNext          = 258,
  AudioPrev          = 259,
  AudioStop          = 260,
  AudioPlay          = 261,
  AudioMute          = 262,
  MediaSelect        = 263,
  WWW                = 264,
  Mail               = 265,
  Calculator         = 266,
  Computer           = 267,
  AC_Search          = 268,
  AC_Home            = 269,
  AC_Back            = 270,
  AC_Forward         = 271,
  AC_Stop            = 272,
  AC_Refresh         = 273,
  AC_Bookmarks       = 274,
  BrightnessDown     = 275,
  BrightnessUp       = 276,
  DisplaySwitch      = 277,
  KbdillumToggle     = 278,
  KbdillumDown       = 279,
  KbdillumUp         = 280,
  Eject              = 281,
  Sleep              = 282,
  App1               = 283,
  App2               = 284,
  AudioRewind        = 285,
  AudioFastForward   = 286,
  SoftLeft           = 287,
  SoftRight          = 288,
  Call               = 289,
  EndCall            = 290
};

constexpr u16 NUM_KEYS = 512;

enum class MouseButtons : u8
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
  All       = 0xFF
};

ASH_DEFINE_ENUM_BIT_OPS(MouseButtons)

struct KeyEvent
{
  ScanCode     scan_code = ScanCode::None;
  KeyCode      key_code  = KeyCode::None;
  KeyModifiers modifiers = KeyModifiers::None;
  KeyAction    action    = KeyAction::Press;
};

struct MouseMotionEvent
{
  u64  id          = U64_MAX;
  Vec2 position    = {};
  Vec2 translation = {};
};

struct MouseClickEvent
{
  u64          id       = U64_MAX;
  Vec2         position = {};
  u32          clicks   = 0;
  MouseButtons button   = MouseButtons::None;
  KeyAction    action   = KeyAction::Press;
};

struct MouseWheelEvent
{
  u64  id          = U64_MAX;
  Vec2 position    = {};
  Vec2 translation = {};
};

enum class WindowEventTypes : u32
{
  None            = 0x0000'0000,
  Shown           = 0x0000'0001,
  Hidden          = 0x0000'0002,
  Exposed         = 0x0000'0004,
  Moved           = 0x0000'0008,
  Resized         = 0x0000'0010,
  SurfaceResized  = 0x0000'0020,
  Minimized       = 0x0000'0040,
  Maximized       = 0x0000'0080,
  Restored        = 0x0000'0100,
  MouseEnter      = 0x0000'0200,
  MouseLeave      = 0x0000'0400,
  FocusIn         = 0x0000'0800,
  FocusOut        = 0x0000'1000,
  CloseRequested  = 0x0000'2000,
  Occluded        = 0x0000'4000,
  EnterFullScreen = 0x0000'8000,
  LeaveFullScreen = 0x0001'0000,
  Key             = 0x0002'0000,
  MouseMotion     = 0x0004'0000,
  MouseClick      = 0x0008'0000,
  MouseWheel      = 0x0010'0000,
  Destroyed       = 0x0020'0000,
  All             = 0xFFFF'FFFF
};

ASH_DEFINE_ENUM_BIT_OPS(WindowEventTypes)

struct WindowEvent
{
  union
  {
    KeyEvent         key;
    MouseMotionEvent mouse_motion;
    MouseClickEvent  mouse_click;
    MouseWheelEvent  mouse_wheel;
  };

  WindowEventTypes type = WindowEventTypes::None;
};

enum class SystemEventTypes : u32
{
  None                     = 0x0000'0000,
  ThemeChanged             = 0x0000'0001,
  KeymapChanged            = 0x0000'0002,
  AudioDeviceAdded         = 0x0000'0004,
  AudioDeviceRemoved       = 0x0000'0008,
  AudioDeviceFormatChanged = 0x0000'0010,
  DisplayReoriented        = 0x0000'0020,
  DisplayAdded             = 0x0000'0040,
  DisplayRemoved           = 0x0000'0080,
  DisplayMoved             = 0x0000'0100,
  CameraAdded              = 0x0000'0200,
  CameraRemoved            = 0x0000'0400,
  CameraApproved           = 0x0000'0800,
  CameraDenied             = 0x0000'1000,
  All                      = 0xFFFF'FFFF
};

ASH_DEFINE_ENUM_BIT_OPS(SystemEventTypes)

struct SystemEvent
{
  union
  {
    SystemTheme theme;
  };

  SystemEventTypes type = SystemEventTypes::None;
};

/// @param Normal region is normal and has no special properties
/// @param Draggable region can drag entire window
/// @param ResizeTopLeft region can resize top left window
/// @param ResizeTop region can resize top window
/// @param ResizeTopRight region can resize top right window
/// @param ResizeRight region can resize right window
/// @param ResizeBottomRight region can resize bottom right window
/// @param ResizeBottom region can resize bottom window
/// @param ResizeBottomLeft region can resize bottom left window
/// @param ResizeLeft region can resize left window
enum class WindowRegion : u8
{
  Normal            = 0,
  Draggable         = 1,
  ResizeTopLeft     = 2,
  ResizeTop         = 3,
  ResizeTopRight    = 4,
  ResizeRight       = 5,
  ResizeBottomRight = 6,
  ResizeBottom      = 7,
  ResizeBottomLeft  = 8,
  ResizeLeft        = 9
};

/// @param Default Default cursor. Usually an arrow
/// @param None No cursor (disable cursor).
/// @param Text Text selection. Usually an I-beam
/// @param Wait Wait. Usually an hourglass or watch or spinning ball.
/// @param CrossHair
/// @param Progress Program is busy but still interactive. Usually it's WAIT
/// with an arrow.
/// @param NWSEResize Double arrow pointing northwest and
/// southeast.
/// @param NESWResize  Double arrow pointing northeast and
/// southwest.
/// @param EWResize  Double arrow pointing west and east.
/// @param NSResize  Double arrow pointing north and south.
/// @param Move Four pointed arrow pointing north, south, east, and west.
/// @param NotAllowed Not permitted. Usually a slashed circle or crossbones.
/// @param Pointer Pointer that indicates a link. Usually a pointing hand.
/// @param NWResize Window resize top-left
/// @param NorthResize Window resize top
/// @param NEResize  Window resize top-right
/// @param EastResize Window resize right
/// @param SEResize resize bottom-right
/// @param SouthResize  Window resize bottom
/// @param SWResize Window resize bottom-left
/// @param WestResize  Window resize left
enum class Cursor
{
  Default     = 0,
  None        = 1,
  Text        = 2,
  Wait        = 3,
  CrossHair   = 4,
  Progress    = 5,
  NWSEResize  = 6,
  NESWResize  = 7,
  EWResize    = 8,
  NSResize    = 9,
  Move        = 10,
  NotAllowed  = 11,
  Pointer     = 12,
  NWResize    = 13,
  NorthResize = 14,
  NEResize    = 15,
  EastResize  = 16,
  SEResize    = 17,
  SouthResize = 18,
  SWResize    = 19,
  WestResize  = 20
};

/// @brief default charset is ASCII
constexpr char const MIME_TEXT_PLAIN[]    = "text/plain";
constexpr char const MIME_TEXT_UTF8[]     = "text/plain;charset=UTF-8";
constexpr char const MIME_TEXT_CSS[]      = "text/css";
constexpr char const MIME_TEXT_CSV[]      = "text/csv";
constexpr char const MIME_TEXT_HTML[]     = "text/html";
constexpr char const MIME_TEXT_JS[]       = "text/javascript";
constexpr char const MIME_TEXT_MARKDOWN[] = "text/markdown";

constexpr char const MIME_IMAGE_AVIF[] = "image/avif";
constexpr char const MIME_IMAGE_BMP[]  = "image/bmp";
constexpr char const MIME_IMAGE_HEIF[] = "image/heif";
constexpr char const MIME_IMAGE_JPEG[] = "image/jpeg";
constexpr char const MIME_IMAGE_PNG[]  = "image/png";
constexpr char const MIME_IMAGE_SVG[]  = "image/svg+xml";
constexpr char const MIME_IMAGE_WEBP[] = "image/webp";

constexpr char const MIME_VIDEO_AV1[]      = "video/AV1";
constexpr char const MIME_VIDEO_H264[]     = "video/H264";
constexpr char const MIME_VIDEO_H265[]     = "video/H265";
constexpr char const MIME_VIDEO_H266[]     = "video/H266";
constexpr char const MIME_VIDEO_MATROSKA[] = "video/matroska";
constexpr char const MIME_VIDEO_MP4[]      = "video/mp4";
constexpr char const MIME_VIDEO_RAW[]      = "video/raw";
constexpr char const MIME_VIDEO_VP8[]      = "video/VP8";
constexpr char const MIME_VIDEO_VP9[]      = "video/VP9";

constexpr char const MIME_MODEL_GLTF_BINARY[] = "model/gltf+binary";
constexpr char const MIME_MODEL_GLTF_JSON[]   = "model/gltf+json";
constexpr char const MIME_MODEL_MESH[]        = "model/mesh";
constexpr char const MIME_MODEL_MTL[]         = "model/mtl";
constexpr char const MIME_MODEL_OBJ[]         = "model/obj";
constexpr char const MIME_MODEL_STL[]         = "model/stl";

constexpr char const MIME_FONT_OTF[]   = "font/otf";
constexpr char const MIME_FONT_SFNT[]  = "font/sfnt";
constexpr char const MIME_FONT_TTF[]   = "font/ttf";
constexpr char const MIME_FONT_WOFF[]  = "font/woff";
constexpr char const MIME_FONT_WOFF2[] = "font/woff2";

struct ClipBoard
{
  virtual Result<> get(Span<char const> mime, Vec<c8> & out)
  {
    (void) mime;
    (void) out;
    return Err{};
  }

  virtual Result<> set(Span<char const> mime, Span<c8 const> data)
  {
    (void) mime;
    (void) data;
    return Err{};
  }

  Result<> get_text(Vec<c8> & out)
  {
    return get(MIME_TEXT_UTF8, out);
  }

  Result<> set_text(Span<c8 const> text)
  {
    return set(MIME_TEXT_UTF8, text);
  }
};

}        // namespace ash
