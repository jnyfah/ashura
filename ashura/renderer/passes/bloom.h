
#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/renderer/render_context.h"
#include "ashura/renderer/view.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

/// E' = Blur(E)
/// D' = Blur(D) + E'
/// C' = Blur(C) + D'
/// B' = Blur(B) + C'
/// A' = Blur(A) + B'
// downsample to mip chains of 5 total
// perform gaussian blur of the image
// addittive composite back unto the first mip
struct BloomPassParams
{
  BloomConfig    config;
  Vec2U          offset = {};
  Vec2U          extent = {};
  gfx::Image     image  = nullptr;
  gfx::ImageView view   = nullptr;
};

struct BloomPass
{
  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, BloomPassParams const &params);
};

}        // namespace ash
