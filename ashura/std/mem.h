#pragma once
#include "ashura/std/traits.h"
#include "ashura/std/types.h"
#include <string.h>

namespace ash
{
namespace mem
{

constexpr u64 align_offset(u64 alignment, u64 offset)
{
  return (offset + (alignment - 1)) & ~(alignment - 1);
}

constexpr bool is_aligned(u64 alignment, u64 offset)
{
  return (offset & (alignment - 1)) == 0;
}

template <typename T>
void copy(T const *src, T *dst, usize count)
{
  if (count == 0)
  {
    return;
  }
  memcpy(dst, src, sizeof(T) * count);
}

template <typename T>
void copy(Span<T const> src, Span<T> dst)
{
  if (src.size() == 0)
  {
    return;
  }
  memcpy(dst.data, src.data(), src.size_bytes());
}

template <typename T>
void copy(Span<T const> src, T *dst)
{
  if (src.size() == 0)
  {
    return;
  }
  memcpy(dst, src.data(), src.size_bytes());
}

template <typename T>
void move(T const *src, T *dst, usize count)
{
  if (count == 0)
  {
    return;
  }
  memmove(dst, src, sizeof(T) * count);
}

template <typename T>
void move(Span<T const> src, Span<T> dst)
{
  if (src.size() == 0)
  {
    return;
  }
  memmove(dst.data(), src.data(), src.size_bytes());
}

template <typename T>
void move(Span<T const> src, T *dst)
{
  if (src.size() == 0)
  {
    return;
  }
  memmove(dst, src.data(), src.size_bytes());
}

template <typename T>
void zero(T *dst, usize count)
{
  if (count == 0)
  {
    return;
  }
  memset(dst, 0, sizeof(T) * count);
}

inline void zero(void *dst, usize size)
{
  if (size == 0)
  {
    return;
  }
  memset(dst, 0, size);
}

template <typename T>
void zero(Span<T> dst)
{
  if (dst.size() == 0)
  {
    return;
  }
  memset(dst.data(), 0, dst.size_bytes());
}

template <typename T>
void fill(T *dst, usize count, u8 byte)
{
  if (count == 0)
  {
    return;
  }
  memset(dst, byte, sizeof(T) * count);
}

template <typename T>
void fill(Span<T> dst, u8 byte)
{
  if (dst.size() == 0)
  {
    return;
  }
  memset(dst.data(), byte, dst.size_bytes());
}

/// move-construct object from src to an uninitialized memory range dst and
/// destroy object at src, leaving src uninitialized.
///
/// src and dst must not be same nor overlapping.
template <typename T>
void relocate(T *src, T *uninit_dst, usize count)
{
  if constexpr (TriviallyRelocatable<T>)
  {
    copy(src, uninit_dst, count);
  }
  else
  {
    T       *in  = src;
    T       *out = uninit_dst;
    T *const end = src + count;
    while (in != end)
    {
      new (out) T{(T &&) *in};
      in++;
      out++;
    }
    in = src;
    while (in != end)
    {
      in->~T();
      in++;
    }
  }
}

}        // namespace mem
}        // namespace ash
