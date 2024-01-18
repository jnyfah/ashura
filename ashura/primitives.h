#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

#include "ashura/math.h"
#include "ashura/types.h"
#include "stx/limits.h"
#include "stx/option.h"

namespace ash
{
typedef struct Rect Rect;
typedef struct Box  Box;

struct Rect
{
  Vec2 offset;
  Vec2 extent;

  constexpr Vec2 center() const
  {
    return offset + (extent / 2);
  }

  constexpr Vec2 end() const
  {
    return offset + extent;
  }

  constexpr f32 area() const
  {
    return extent.x * extent.y;
  }

  constexpr bool contains(Vec2 point) const
  {
    return offset.x <= point.x && offset.y <= point.y &&
           (offset.x + extent.x) >= point.x && (offset.y + extent.y) >= point.y;
  }

  constexpr bool overlaps(Rect const &other) const
  {
    Vec2 a_begin = offset;
    Vec2 a_end   = offset + extent;
    Vec2 b_begin = other.offset;
    Vec2 b_end   = other.offset + other.extent;
    return a_begin.x <= b_end.x && a_end.x >= b_begin.x &&
           a_begin.y <= b_end.y && a_end.y >= b_begin.y;
  }

  constexpr Rect intersect(Rect const &other) const
  {
    if (!overlaps(other))
    {
      return Rect{offset, {0, 0}};
    }

    Vec2 a_begin = offset;
    Vec2 a_end   = offset + extent;
    Vec2 b_begin = other.offset;
    Vec2 b_end   = other.offset + other.extent;
    Vec2 int_begin{op::max(a_begin.x, b_begin.x),
                   op::max(a_begin.y, b_begin.y)};
    Vec2 int_end{op::min(a_end.x, b_end.x), op::min(a_end.y, b_end.y)};

    return Rect{int_begin, int_end - int_begin};
  }
};

struct Box
{
  Vec3 offset;
  Vec3 extent;

  constexpr Vec3 center() const
  {
    return offset + (extent / 2);
  }

  constexpr Vec3 end() const
  {
    return offset + extent;
  }

  constexpr f32 volume() const
  {
    return extent.x * extent.y * extent.z;
  }

  constexpr bool contains(Vec3 point) const
  {
    return offset.x <= point.x && offset.y <= point.y && offset.z <= point.z &&
           (offset.x + extent.x) >= point.x &&
           (offset.y + extent.y) >= point.y && (offset.z + extent.z) >= point.z;
    return true;
  }

  constexpr bool overlaps(Box const &other) const
  {
    Vec3 a_begin = offset;
    Vec3 a_end   = offset + extent;
    Vec3 b_begin = other.offset;
    Vec3 b_end   = other.offset + other.extent;
    return a_begin.x <= b_end.x && a_end.x >= b_begin.x &&
           a_begin.y <= b_end.y && a_end.y >= b_begin.y &&
           a_begin.z <= b_end.z && a_end.z >= b_begin.z;
  }
};

}        // namespace ash
