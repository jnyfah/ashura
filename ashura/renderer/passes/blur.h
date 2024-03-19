#pragma once
#include "ashura/renderer/render_context.h"

namespace ash
{

BEGIN_SHADER_PARAMETER(BlurPassShaderParameter)
SHADER_COMBINED_IMAGE_SAMPLER(src, 1)
END_SHADER_PARAMETER(BlurPassShaderParameter)

struct BlurPassShaderUniform
{
  Vec2I radius;
};

struct BlurPassParams
{
  Vec2U          offset = {};
  Vec2U          extent = {};
  Vec2U          radius = {0, 0};
  gfx::ImageView view   = nullptr;
};

struct BlurPass
{
  ShaderParameterHeap<BlurPassShaderParameter> parameter_heap_      = {};
  gfx::GraphicsPipeline                        downsample_pipeline_ = nullptr;
  gfx::GraphicsPipeline                        upsample_pipeline_   = nullptr;
  gfx::Sampler                                 sampler_             = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, BlurPassParams const &params);
};

}        // namespace ash
