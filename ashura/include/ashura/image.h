#pragma once
#include "ashura/primitives.h"
#include "stx/memory.h"
#include "stx/span.h"

namespace ash
{

namespace gfx
{

/// NOTE: resource image with index 0 must be a transparent white image
using image = u64;

constexpr image WHITE_IMAGE = 0;

}        // namespace gfx

// NOTE: pixels are not stored in endian order but in byte by byte interleaving. i.e. for
// RGBA bytes 0 -> R, 1 -> G, 2 -> B, 3 -> A i.e. [r, g, b, a, r, g, b, a]
enum class ImageFormat : u8
{
  Alpha,               // A8
  Antialiasing,        // A8
  Gray,                // A8
  Rgb,                 // R8G8B8
  Rgba,                // R8G8B8A8
  Bgra                 // B8G8R8A8
};

enum class ColorSpace : u8
{
  Rgb,
  Srgb,
  AdobeRgb,
  Dp3,
  DciP3,
  Yuv
};

inline u8 nchannels(ImageFormat fmt)
{
  switch (fmt)
  {
    case ImageFormat::Alpha:
    case ImageFormat::Antialiasing:
    case ImageFormat::Gray:
      return 1;
    case ImageFormat::Rgb:
      return 3;
    case ImageFormat::Rgba:
      return 4;
    case ImageFormat::Bgra:
      return 4;
    default:
      ASH_UNREACHABLE();
  }
}

inline u8 nchannel_bytes(ImageFormat fmt)
{
  switch (fmt)
  {
    case ImageFormat::Alpha:
    case ImageFormat::Antialiasing:
    case ImageFormat::Gray:
      return 1;
    case ImageFormat::Rgb:
      return 3;
    case ImageFormat::Rgba:
      return 4;
    case ImageFormat::Bgra:
      return 4;
    default:
      ASH_UNREACHABLE();
  }
}

struct ImageView
{
  stx::Span<u8 const> data;
  ash::extent         extent;
  ImageFormat         format = ImageFormat::Rgba;
};

struct ImageViewMut
{
  stx::Span<u8> data;
  ash::extent   extent;
  ImageFormat   format = ImageFormat::Rgba;

  constexpr operator ImageView() const
  {
    return ImageView{.data = data, .extent = extent, .format = format};
  }
};

struct ImageBuffer
{
  stx::Memory memory;
  ash::extent extent;
  ImageFormat format = ImageFormat::Rgba;

  stx::Span<u8 const> span() const
  {
    return stx::Span{AS(u8 const *, memory.handle), extent.area() * nchannel_bytes(format)};
  }

  stx::Span<u8> span()
  {
    return stx::Span{AS(u8 *, memory.handle), extent.area() * nchannel_bytes(format)};
  }

  operator ImageView() const
  {
    return ImageView{.data = span(), .extent = extent, .format = format};
  }

  operator ImageViewMut()
  {
    return ImageViewMut{.data = span(), .extent = extent, .format = format};
  }

  void resize(ash::extent new_extent)
  {
    if (extent != new_extent)
    {
      if (extent.area() != new_extent.area())
      {
        stx::mem::reallocate(memory, new_extent.area() * nchannel_bytes(format)).unwrap();
      }
      extent = new_extent;
    }
  }
};

}        // namespace ash
