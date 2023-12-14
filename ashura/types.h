#pragma once
#include <cfloat>
#include <cinttypes>
#include <cstddef>
#include <cstdint>

// todo(lamarrr): operator == != and assign for mat*

namespace ash
{

using u8    = uint8_t;
using u16   = uint16_t;
using u32   = uint32_t;
using u64   = uint64_t;
using i8    = int8_t;
using i16   = int16_t;
using i32   = int32_t;
using i64   = int64_t;
using f32   = float;
using f64   = double;
using usize = size_t;
using isize = ptrdiff_t;

constexpr u8 U8_MIN = 0;
constexpr u8 U8_MAX = 0xFFU;

constexpr i8 I8_MIN = -0x7F - 1;
constexpr i8 I8_MAX = 0x7F;

constexpr u16 U16_MIN = 0;
constexpr u16 U16_MAX = 0xFFFFU;

constexpr i16 I16_MIN = -0x7FFF - 1;
constexpr i16 I16_MAX = 0x7FFF;

constexpr u32 U32_MIN = 0;
constexpr u32 U32_MAX = 0xFFFFFFFFU;

constexpr i32 I32_MIN = -0x7FFFFFFF - 1;
constexpr i32 I32_MAX = 0x7FFFFFFF;

constexpr u64 U64_MIN = 0;
constexpr u64 U64_MAX = 0xFFFFFFFFFFFFFFFFULL;

constexpr i64 I64_MIN = -0x7FFFFFFFFFFFFFFFLL - 1;
constexpr i64 I64_MAX = 0x7FFFFFFFFFFFFFFFLL;

constexpr usize USIZE_MIN = 0;
constexpr usize USIZE_MAX = SIZE_MAX;

constexpr isize ISIZE_MIN = PTRDIFF_MIN;
constexpr isize ISIZE_MAX = PTRDIFF_MAX;

constexpr f32 F32_MIN          = -FLT_MAX;
constexpr f32 F32_MIN_POSITIVE = FLT_MIN;
constexpr f32 F32_MAX          = FLT_MAX;
constexpr f32 F32_EPSILON      = FLT_EPSILON;

constexpr f64 F64_MIN          = -DBL_MAX;
constexpr f64 F64_MIN_POSITIVE = DBL_MIN;
constexpr f64 F64_MAX          = DBL_MAX;
constexpr f32 F64_EPSILON      = DBL_EPSILON;

constexpr usize MAX_STANDARD_ALIGNMENT = alignof(max_align_t);

constexpr f32 PI = 3.14159265358979323846f;

template <typename T>
constexpr bool has_bits(T src, T cmp)
{
  return (src & cmp) == cmp;
}

template <typename T>
constexpr bool has_any_bit(T src, T cmp)
{
  return (src & cmp) != (T) 0;
}

typedef struct Vec2       Vec2;
typedef struct Vec2       Complex;
typedef struct Vec3       Vec3;
typedef struct Vec4       Vec4;
typedef struct Vec4       Quaternion;
typedef struct Vec4U8     Vec4U8;
typedef struct Vec2I      Vec2I;
typedef struct Vec3I      Vec3I;
typedef struct Vec2U      Vec2U;
typedef struct Vec3U      Vec3U;
typedef struct Vec4U      Vec4U;
typedef struct Mat2       Mat2;
typedef struct Mat3       Mat3;
typedef struct Mat3Affine Mat3Affine;
typedef struct Mat4       Mat4;
typedef struct Mat4Affine Mat4Affine;
typedef struct Slice      Slice;
template <typename T>
struct Span;

struct alignas(8) Vec2
{
  f32 x = 0;
  f32 y = 0;
};

constexpr bool operator==(Vec2 a, Vec2 b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(Vec2 a, Vec2 b)
{
  return a.x != b.x || a.y != b.y;
}

constexpr Vec2 operator+(Vec2 a, Vec2 b)
{
  return Vec2{a.x + b.x, a.y + b.y};
}

constexpr Vec2 operator-(Vec2 a, Vec2 b)
{
  return Vec2{a.x - b.x, a.y - b.y};
}

constexpr Vec2 operator*(Vec2 a, Vec2 b)
{
  return Vec2{a.x * b.x, a.y * b.y};
}

constexpr Vec2 operator/(Vec2 a, Vec2 b)
{
  return Vec2{a.x / b.x, a.y / b.y};
}

constexpr Vec2 &operator+=(Vec2 &a, Vec2 b)
{
  a = a + b;
  return a;
}

constexpr Vec2 &operator-=(Vec2 &a, Vec2 b)
{
  a = a - b;
  return a;
}

constexpr Vec2 &operator*=(Vec2 &a, Vec2 b)
{
  a = a * b;
  return a;
}

constexpr Vec2 &operator/=(Vec2 &a, Vec2 b)
{
  a = a / b;
  return a;
}

struct Vec3
{
  f32 x = 0;
  f32 y = 0;
  f32 z = 0;
};

constexpr bool operator==(Vec3 a, Vec3 b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

constexpr bool operator!=(Vec3 a, Vec3 b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z;
}

constexpr Vec3 operator+(Vec3 a, Vec3 b)
{
  return Vec3{a.x + b.x, a.y + b.y, a.z + b.z};
}

constexpr Vec3 operator-(Vec3 a, Vec3 b)
{
  return Vec3{a.x - b.x, a.y - b.y, a.z - b.z};
}

constexpr Vec3 operator*(Vec3 a, Vec3 b)
{
  return Vec3{a.x * b.x, a.y * b.y, a.z * b.z};
}

constexpr Vec3 operator/(Vec3 a, Vec3 b)
{
  return Vec3{a.x / b.x, a.y / b.y, a.z / b.z};
}

constexpr Vec3 &operator+=(Vec3 &a, Vec3 b)
{
  a = a + b;
  return a;
}

constexpr Vec3 &operator-=(Vec3 &a, Vec3 b)
{
  a = a - b;
  return a;
}

constexpr Vec3 &operator*=(Vec3 &a, Vec3 b)
{
  a = a * b;
  return a;
}

constexpr Vec3 &operator/=(Vec3 &a, Vec3 b)
{
  a = a / b;
  return a;
}

struct alignas(16) Vec4
{
  f32 x = 0;
  f32 y = 0;
  f32 z = 0;
  f32 w = 0;
};

constexpr bool operator==(Vec4 a, Vec4 b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr bool operator!=(Vec4 a, Vec4 b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

constexpr Vec4 operator+(Vec4 a, Vec4 b)
{
  return Vec4{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

constexpr Vec4 operator-(Vec4 a, Vec4 b)
{
  return Vec4{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

constexpr Vec4 operator*(Vec4 a, Vec4 b)
{
  return Vec4{a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

constexpr Vec4 operator/(Vec4 a, Vec4 b)
{
  return Vec4{a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

constexpr Vec4 &operator+=(Vec4 &a, Vec4 b)
{
  a = a + b;
  return a;
}

constexpr Vec4 &operator-=(Vec4 &a, Vec4 b)
{
  a = a - b;
  return a;
}

constexpr Vec4 &operator*=(Vec4 &a, Vec4 b)
{
  a = a * b;
  return a;
}

constexpr Vec4 &operator/=(Vec4 &a, Vec4 b)
{
  a = a / b;
  return a;
}

struct alignas(4) Vec4U8
{
  u8 x = 0;
  u8 y = 0;
  u8 z = 0;
  u8 w = 0;
};

constexpr bool operator==(Vec4U8 a, Vec4U8 b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr bool operator!=(Vec4U8 a, Vec4U8 b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

constexpr Vec4U8 operator+(Vec4U8 a, Vec4U8 b)
{
  return Vec4U8{static_cast<u8>(a.x + b.x), static_cast<u8>(a.y + b.y),
                static_cast<u8>(a.z + b.z), static_cast<u8>(a.w + b.w)};
}

constexpr Vec4U8 operator-(Vec4U8 a, Vec4U8 b)
{
  return Vec4U8{static_cast<u8>(a.x - b.x), static_cast<u8>(a.y - b.y),
                static_cast<u8>(a.z - b.z), static_cast<u8>(a.w - b.w)};
}

constexpr Vec4U8 operator*(Vec4U8 a, Vec4U8 b)
{
  return Vec4U8{static_cast<u8>(a.x * b.x), static_cast<u8>(a.y * b.y),
                static_cast<u8>(a.z * b.z), static_cast<u8>(a.w * b.w)};
}

constexpr Vec4U8 operator/(Vec4U8 a, Vec4U8 b)
{
  return Vec4U8{static_cast<u8>(a.x / b.x), static_cast<u8>(a.y / b.y),
                static_cast<u8>(a.z / b.z), static_cast<u8>(a.w / b.w)};
}

constexpr Vec4U8 &operator+=(Vec4U8 &a, Vec4U8 b)
{
  a = a + b;
  return a;
}

constexpr Vec4U8 &operator-=(Vec4U8 &a, Vec4U8 b)
{
  a = a - b;
  return a;
}

constexpr Vec4U8 &operator*=(Vec4U8 &a, Vec4U8 b)
{
  a = a * b;
  return a;
}

constexpr Vec4U8 &operator/=(Vec4U8 &a, Vec4U8 b)
{
  a = a / b;
  return a;
}

struct alignas(8) Vec2I
{
  i32 x = 0;
  i32 y = 0;
};

constexpr bool operator==(Vec2I a, Vec2I b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(Vec2I a, Vec2I b)
{
  return a.x != b.x || a.y != b.y;
}

constexpr Vec2I operator+(Vec2I a, Vec2I b)
{
  return Vec2I{a.x + b.x, a.y + b.y};
}

constexpr Vec2I operator-(Vec2I a, Vec2I b)
{
  return Vec2I{a.x - b.x, a.y - b.y};
}

constexpr Vec2I operator*(Vec2I a, Vec2I b)
{
  return Vec2I{a.x * b.x, a.y * b.y};
}

constexpr Vec2I operator/(Vec2I a, Vec2I b)
{
  return Vec2I{a.x / b.x, a.y / b.y};
}

constexpr Vec2I &operator+=(Vec2I &a, Vec2I b)
{
  a = a + b;
  return a;
}

constexpr Vec2I &operator-=(Vec2I &a, Vec2I b)
{
  a = a - b;
  return a;
}

constexpr Vec2I &operator*=(Vec2I &a, Vec2I b)
{
  a = a * b;
  return a;
}

constexpr Vec2I &operator/=(Vec2I &a, Vec2I b)
{
  a = a / b;
  return a;
}

struct Vec3I
{
  i32 x = 0;
  i32 y = 0;
  i32 z = 0;
};

constexpr bool operator==(Vec3I a, Vec3I b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

constexpr bool operator!=(Vec3I a, Vec3I b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z;
}

constexpr Vec3I operator+(Vec3I a, Vec3I b)
{
  return Vec3I{a.x + b.x, a.y + b.y, a.z + b.z};
}

constexpr Vec3I operator-(Vec3I a, Vec3I b)
{
  return Vec3I{a.x - b.x, a.y - b.y, a.z - b.z};
}

constexpr Vec3I operator*(Vec3I a, Vec3I b)
{
  return Vec3I{a.x * b.x, a.y * b.y, a.z * b.z};
}

constexpr Vec3I operator/(Vec3I a, Vec3I b)
{
  return Vec3I{a.x / b.x, a.y / b.y, a.z / b.z};
}

constexpr Vec3I &operator+=(Vec3I &a, Vec3I b)
{
  a = a + b;
  return a;
}

constexpr Vec3I &operator-=(Vec3I &a, Vec3I b)
{
  a = a - b;
  return a;
}

constexpr Vec3I &operator*=(Vec3I &a, Vec3I b)
{
  a = a * b;
  return a;
}

constexpr Vec3I &operator/=(Vec3I &a, Vec3I b)
{
  a = a / b;
  return a;
}

struct alignas(16) Vec4I
{
  i32 x = 0;
  i32 y = 0;
  i32 z = 0;
  i32 w = 0;
};

constexpr bool operator==(Vec4I a, Vec4I b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr bool operator!=(Vec4I a, Vec4I b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

constexpr Vec4I operator+(Vec4I a, Vec4I b)
{
  return Vec4I{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

constexpr Vec4I operator-(Vec4I a, Vec4I b)
{
  return Vec4I{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

constexpr Vec4I operator*(Vec4I a, Vec4I b)
{
  return Vec4I{a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

constexpr Vec4I operator/(Vec4I a, Vec4I b)
{
  return Vec4I{a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

constexpr Vec4I &operator+=(Vec4I &a, Vec4I b)
{
  a = a + b;
  return a;
}

constexpr Vec4I &operator-=(Vec4I &a, Vec4I b)
{
  a = a - b;
  return a;
}

constexpr Vec4I &operator*=(Vec4I &a, Vec4I b)
{
  a = a * b;
  return a;
}

constexpr Vec4I &operator/=(Vec4I &a, Vec4I b)
{
  a = a / b;
  return a;
}

struct alignas(8) Vec2U
{
  u32 x = 0;
  u32 y = 0;
};

constexpr bool operator==(Vec2U a, Vec2U b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(Vec2U a, Vec2U b)
{
  return a.x != b.x || a.y != b.y;
}

constexpr Vec2U operator+(Vec2U a, Vec2U b)
{
  return Vec2U{a.x + b.x, a.y + b.y};
}

constexpr Vec2U operator-(Vec2U a, Vec2U b)
{
  return Vec2U{a.x - b.x, a.y - b.y};
}

constexpr Vec2U operator*(Vec2U a, Vec2U b)
{
  return Vec2U{a.x * b.x, a.y * b.y};
}

constexpr Vec2U operator/(Vec2U a, Vec2U b)
{
  return Vec2U{a.x / b.x, a.y / b.y};
}

constexpr Vec2U &operator+=(Vec2U &a, Vec2U b)
{
  a = a + b;
  return a;
}

constexpr Vec2U &operator-=(Vec2U &a, Vec2U b)
{
  a = a - b;
  return a;
}

constexpr Vec2U &operator*=(Vec2U &a, Vec2U b)
{
  a = a * b;
  return a;
}

constexpr Vec2U &operator/=(Vec2U &a, Vec2U b)
{
  a = a / b;
  return a;
}

struct Vec3U
{
  u32 x = 0;
  u32 y = 0;
  u32 z = 0;
};

constexpr bool operator==(Vec3U a, Vec3U b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

constexpr bool operator!=(Vec3U a, Vec3U b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z;
}

constexpr Vec3U operator+(Vec3U a, Vec3U b)
{
  return Vec3U{a.x + b.x, a.y + b.y, a.z + b.z};
}

constexpr Vec3U operator-(Vec3U a, Vec3U b)
{
  return Vec3U{a.x - b.x, a.y - b.y, a.z - b.z};
}

constexpr Vec3U operator*(Vec3U a, Vec3U b)
{
  return Vec3U{a.x * b.x, a.y * b.y, a.z * b.z};
}

constexpr Vec3U operator/(Vec3U a, Vec3U b)
{
  return Vec3U{a.x / b.x, a.y / b.y, a.z / b.z};
}

constexpr Vec3U &operator+=(Vec3U &a, Vec3U b)
{
  a = a + b;
  return a;
}

constexpr Vec3U &operator-=(Vec3U &a, Vec3U b)
{
  a = a - b;
  return a;
}

constexpr Vec3U &operator*=(Vec3U &a, Vec3U b)
{
  a = a * b;
  return a;
}

constexpr Vec3U &operator/=(Vec3U &a, Vec3U b)
{
  a = a / b;
  return a;
}

struct alignas(16) Vec4U
{
  u32 x = 0;
  u32 y = 0;
  u32 z = 0;
  u32 w = 0;
};

constexpr bool operator==(Vec4U a, Vec4U b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr bool operator!=(Vec4U a, Vec4U b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

constexpr Vec4U operator+(Vec4U a, Vec4U b)
{
  return Vec4U{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

constexpr Vec4U operator-(Vec4U a, Vec4U b)
{
  return Vec4U{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

constexpr Vec4U operator*(Vec4U a, Vec4U b)
{
  return Vec4U{a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

constexpr Vec4U operator/(Vec4U a, Vec4U b)
{
  return Vec4U{a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

constexpr Vec4U &operator+=(Vec4U &a, Vec4U b)
{
  a = a + b;
  return a;
}

constexpr Vec4U &operator-=(Vec4U &a, Vec4U b)
{
  a = a - b;
  return a;
}

constexpr Vec4U &operator*=(Vec4U &a, Vec4U b)
{
  a = a * b;
  return a;
}

constexpr Vec4U &operator/=(Vec4U &a, Vec4U b)
{
  a = a / b;
  return a;
}

struct Mat2
{
  Vec2 rows[2] = {};

  constexpr Vec2 &operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec2 const &operator[](usize index) const
  {
    return rows[index];
  }
};

constexpr Mat2 operator+(Mat2 const &a, Mat2 const &b)
{
  return Mat2{.rows = {a[0] + b[0], a[1] + b[1]}};
}

constexpr Mat2 operator-(Mat2 const &a, Mat2 const &b)
{
  return Mat2{.rows = {a[0] - b[0], a[1] - b[1]}};
}

constexpr Mat2 operator*(Mat2 const &a, Mat2 const &b)
{
  return Mat2{.rows = {a[0] * b[0], a[1] * b[1]}};
}

constexpr Mat2 operator/(Mat2 const &a, Mat2 const &b)
{
  return Mat2{.rows = {a[0] / b[0], a[1] / b[1]}};
}

constexpr Mat2 &operator+=(Mat2 &a, Mat2 const &b)
{
  a = a + b;
  return a;
}

constexpr Mat2 &operator-=(Mat2 &a, Mat2 const &b)
{
  a = a - b;
  return a;
}

constexpr Mat2 &operator*=(Mat2 &a, Mat2 const &b)
{
  a = a * b;
  return a;
}

constexpr Mat2 &operator/=(Mat2 &a, Mat2 const &b)
{
  a = a / b;
  return a;
}

struct Mat3
{
  Vec3 rows[3] = {};

  constexpr Vec3 &operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec3 const &operator[](usize index) const
  {
    return rows[index];
  }
};

constexpr Mat3 operator+(Mat3 const &a, Mat3 const &b)
{
  return Mat3{.rows = {a[0] + b[0], a[1] + b[1], a[2] + b[2]}};
}

constexpr Mat3 operator-(Mat3 const &a, Mat3 const &b)
{
  return Mat3{.rows = {a[0] - b[0], a[1] - b[1], a[2] - b[2]}};
}

constexpr Mat3 operator*(Mat3 const &a, Mat3 const &b)
{
  return Mat3{.rows = {a[0] * b[0], a[1] * b[1], a[2] * b[2]}};
}

constexpr Mat3 operator/(Mat3 const &a, Mat3 const &b)
{
  return Mat3{.rows = {a[0] / b[0], a[1] / b[1], a[2] / b[2]}};
}

constexpr Mat3 &operator+=(Mat3 &a, Mat3 const &b)
{
  a = a + b;
  return a;
}

constexpr Mat3 &operator-=(Mat3 &a, Mat3 const &b)
{
  a = a - b;
  return a;
}

constexpr Mat3 &operator*=(Mat3 &a, Mat3 const &b)
{
  a = a * b;
  return a;
}

constexpr Mat3 &operator/=(Mat3 &a, Mat3 const &b)
{
  a = a / b;
  return a;
}

struct Mat3Affine
{
  Vec3 rows[2] = {};

  constexpr Vec3 &operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec3 const &operator[](usize index) const
  {
    return rows[index];
  }
};

constexpr Mat3Affine operator+(Mat3Affine const &a, Mat3Affine const &b)
{
  return Mat3Affine{.rows = {a[0] + b[0], a[1] + b[1]}};
}

constexpr Mat3Affine operator-(Mat3Affine const &a, Mat3Affine const &b)
{
  return Mat3Affine{.rows = {a[0] - b[0], a[1] - b[1]}};
}

constexpr Mat3Affine operator*(Mat3Affine const &a, Mat3Affine const &b)
{
  return Mat3Affine{.rows = {a[0] * b[0], a[1] * b[1]}};
}

constexpr Mat3Affine operator/(Mat3Affine const &a, Mat3Affine const &b)
{
  return Mat3Affine{.rows = {a[0] / b[0], a[1] / b[1]}};
}

constexpr Mat3Affine &operator+=(Mat3Affine &a, Mat3Affine const &b)
{
  a = a + b;
  return a;
}

constexpr Mat3Affine &operator-=(Mat3Affine &a, Mat3Affine const &b)
{
  a = a - b;
  return a;
}

constexpr Mat3Affine &operator*=(Mat3Affine &a, Mat3Affine const &b)
{
  a = a * b;
  return a;
}

constexpr Mat3Affine &operator/=(Mat3Affine &a, Mat3Affine const &b)
{
  a = a / b;
  return a;
}

struct Mat4
{
  Vec4 rows[4] = {};

  constexpr Vec4 &operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec4 const &operator[](usize index) const
  {
    return rows[index];
  }
};

constexpr Mat4 operator+(Mat4 const &a, Mat4 const &b)
{
  return Mat4{.rows = {a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]}};
}

constexpr Mat4 operator-(Mat4 const &a, Mat4 const &b)
{
  return Mat4{.rows = {a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]}};
}

constexpr Mat4 operator*(Mat4 const &a, Mat4 const &b)
{
  return Mat4{.rows = {a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3]}};
}

constexpr Mat4 operator/(Mat4 const &a, Mat4 const &b)
{
  return Mat4{.rows = {a[0] / b[0], a[1] / b[1], a[2] / b[2], a[3] / b[3]}};
}

constexpr Mat4 &operator+=(Mat4 &a, Mat4 const &b)
{
  a = a + b;
  return a;
}

constexpr Mat4 &operator-=(Mat4 &a, Mat4 const &b)
{
  a = a - b;
  return a;
}

constexpr Mat4 &operator*=(Mat4 &a, Mat4 const &b)
{
  a = a * b;
  return a;
}

constexpr Mat4 &operator/=(Mat4 &a, Mat4 const &b)
{
  a = a / b;
  return a;
}

struct Mat4Affine
{
  Vec4 rows[3] = {};

  constexpr Vec4 &operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec4 const &operator[](usize index) const
  {
    return rows[index];
  }
};

constexpr Mat4Affine operator+(Mat4Affine const &a, Mat4Affine const &b)
{
  return Mat4Affine{.rows = {a[0] + b[0], a[1] + b[1], a[2] + b[2]}};
}

constexpr Mat4Affine operator-(Mat4Affine const &a, Mat4Affine const &b)
{
  return Mat4Affine{.rows = {a[0] - b[0], a[1] - b[1], a[2] - b[2]}};
}

constexpr Mat4Affine operator*(Mat4Affine const &a, Mat4Affine const &b)
{
  return Mat4Affine{.rows = {a[0] * b[0], a[1] * b[1], a[2] * b[2]}};
}

constexpr Mat4Affine operator/(Mat4Affine const &a, Mat4Affine const &b)
{
  return Mat4Affine{.rows = {a[0] / b[0], a[1] / b[1], a[2] / b[2]}};
}

constexpr Mat4Affine &operator+=(Mat4Affine &a, Mat4Affine const &b)
{
  a = a + b;
  return a;
}

constexpr Mat4Affine &operator-=(Mat4Affine &a, Mat4Affine const &b)
{
  a = a - b;
  return a;
}

constexpr Mat4Affine &operator*=(Mat4Affine &a, Mat4Affine const &b)
{
  a = a * b;
  return a;
}

constexpr Mat4Affine &operator/=(Mat4Affine &a, Mat4Affine const &b)
{
  a = a / b;
  return a;
}

struct Slice
{
  usize offset = 0;
  usize size   = 0;
};

template <typename T>
struct Span
{
  T    *data = nullptr;
  usize size = 0;

  constexpr usize size_bytes() const
  {
    return sizeof(T) * size;
  }

  constexpr bool is_empty() const
  {
    return size == 0;
  }

  constexpr T *begin() const
  {
    return data;
  }

  constexpr T *end() const
  {
    return data + size;
  }

  constexpr T &operator[](usize index) const
  {
    return data[index];
  }

  constexpr Span<T> operator[](Slice slice) const
  {
    // written such that overflow will not occur even if both offset and size
    // are set to USIZE_MAX
    slice.offset = slice.offset > size ? size : slice.offset;
    slice.size =
        (size - slice.offset) > slice.size ? slice.size : (size - slice.offset);
    return Span<T>{data + slice.offset, slice.size};
  }

  constexpr operator Span<T const>() const
  {
    return Span<T const>{data, size};
  }
};

}        // namespace ash
