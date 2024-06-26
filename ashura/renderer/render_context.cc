#include "ashura/renderer/render_context.h"

namespace ash
{

void RenderContext::init(gfx::DeviceImpl p_device, bool p_use_hdr,
                         u32 p_buffering, gfx::Extent p_initial_extent,
                         ShaderMap p_shader_map)
{
  CHECK(p_buffering <= 4 && p_buffering > 0);
  CHECK(p_initial_extent.x > 0 && p_initial_extent.y > 0);
  device = p_device;

  gfx::Format color_format         = gfx::Format::Undefined;
  gfx::Format depth_stencil_format = gfx::Format::Undefined;

  if (p_use_hdr)
  {
    gfx::FormatProperties properties =
        device
            ->get_format_properties(device.self,
                                    gfx::Format::R16G16B16A16_SFLOAT)
            .unwrap();
    if (has_bits(properties.optimal_tiling_features, COLOR_FEATURES))
    {
      color_format = gfx::Format::R16G16B16A16_SFLOAT;
    }
    else
    {
      default_logger->warn("HDR mode requested but Device does not support "
                           "HDR render target, trying UNORM color");
    }
  }

  if (color_format == gfx::Format::Undefined)
  {
    gfx::FormatProperties properties =
        device->get_format_properties(device.self, gfx::Format::B8G8R8A8_UNORM)
            .unwrap();
    if (has_bits(properties.optimal_tiling_features, COLOR_FEATURES))
    {
      color_format = gfx::Format::B8G8R8A8_UNORM;
    }
  }

  if (color_format == gfx::Format::Undefined)
  {
    gfx::FormatProperties properties =
        device->get_format_properties(device.self, gfx::Format::R8G8B8A8_UNORM)
            .unwrap();
    if (has_bits(properties.optimal_tiling_features, COLOR_FEATURES))
    {
      color_format = gfx::Format::R8G8B8A8_UNORM;
    }
  }

  {
    gfx::FormatProperties properties =
        device
            ->get_format_properties(device.self, gfx::Format::D16_UNORM_S8_UINT)
            .unwrap();
    if (has_bits(properties.optimal_tiling_features, DEPTH_STENCIL_FEATURES))
    {
      depth_stencil_format = gfx::Format::D16_UNORM_S8_UINT;
    }
  }

  if (depth_stencil_format == gfx::Format::Undefined)
  {
    gfx::FormatProperties properties =
        device
            ->get_format_properties(device.self, gfx::Format::D24_UNORM_S8_UINT)
            .unwrap();
    if (has_bits(properties.optimal_tiling_features, DEPTH_STENCIL_FEATURES))
    {
      depth_stencil_format = gfx::Format::D24_UNORM_S8_UINT;
    }
  }

  CHECK_EX("Device doesn't support any known color format",
           color_format != gfx::Format::Undefined);
  CHECK_EX("Device doesn't support any depth stencil format",
           depth_stencil_format != gfx::Format::Undefined);

  pipeline_cache             = nullptr;
  buffering                  = p_buffering;
  shader_map                 = p_shader_map;
  this->color_format         = color_format;
  this->depth_stencil_format = depth_stencil_format;

  recreate_framebuffers(p_initial_extent);

  ssbo_layout =
      device
          ->create_descriptor_set_layout(
              device.self,
              gfx::DescriptorSetLayoutDesc{
                  .label    = "SSBO"_span,
                  .bindings = to_span({gfx::DescriptorBindingDesc{
                      .type  = gfx::DescriptorType::DynamicStorageBuffer,
                      .count = 1,
                      .is_variable_length = false}})})
          .unwrap();

  textures_layout =
      device
          ->create_descriptor_set_layout(
              device.self,
              gfx::DescriptorSetLayoutDesc{
                  .label    = "Texture Pack"_span,
                  .bindings = to_span({gfx::DescriptorBindingDesc{
                      .type  = gfx::DescriptorType::CombinedImageSampler,
                      .count = MAX_TEXTURE_PACK_COUNT,
                      .is_variable_length = true}})})
          .unwrap();
}

void recreate_attachment(RenderContext &ctx, Framebuffer &attachment,
                         gfx::Extent new_extent)
{
  ctx.release(attachment.color_image);
  ctx.release(attachment.color_image_view);
  ctx.release(attachment.depth_stencil_image);
  ctx.release(attachment.depth_stencil_image_view);

  attachment.color_image_desc = gfx::ImageDesc{
      .label  = "Framebuffer Color Image"_span,
      .type   = gfx::ImageType::Type2D,
      .format = ctx.color_format,
      .usage  = gfx::ImageUsage::ColorAttachment | gfx::ImageUsage::Sampled |
               gfx::ImageUsage::Storage | gfx::ImageUsage::TransferDst |
               gfx::ImageUsage::TransferSrc,
      .aspects      = gfx::ImageAspects::Color,
      .extent       = gfx::Extent3D{new_extent.x, new_extent.y, 1},
      .mip_levels   = 1,
      .array_layers = 1,
      .sample_count = gfx::SampleCount::Count1};

  attachment.color_image =
      ctx.device->create_image(ctx.device.self, attachment.color_image_desc)
          .unwrap();

  attachment.color_image_view_desc =
      gfx::ImageViewDesc{.label           = "Framebuffer Color Image View"_span,
                         .image           = attachment.color_image,
                         .view_type       = gfx::ImageViewType::Type2D,
                         .view_format     = attachment.color_image_desc.format,
                         .mapping         = {},
                         .aspects         = gfx::ImageAspects::Color,
                         .first_mip_level = 0,
                         .num_mip_levels  = 1,
                         .first_array_layer = 0,
                         .num_array_layers  = 1};
  attachment.color_image_view =
      ctx.device
          ->create_image_view(ctx.device.self, attachment.color_image_view_desc)
          .unwrap();

  attachment.depth_stencil_image_desc = gfx::ImageDesc{
      .label  = "Framebuffer Depth Stencil Image"_span,
      .type   = gfx::ImageType::Type2D,
      .format = ctx.depth_stencil_format,
      .usage  = gfx::ImageUsage::DepthStencilAttachment |
               gfx::ImageUsage::Sampled | gfx::ImageUsage::TransferDst |
               gfx::ImageUsage::TransferSrc,
      .aspects      = gfx::ImageAspects::Depth | gfx::ImageAspects::Stencil,
      .extent       = gfx::Extent3D{new_extent.x, new_extent.y, 1},
      .mip_levels   = 1,
      .array_layers = 1,
      .sample_count = gfx::SampleCount::Count1};

  attachment.depth_stencil_image =
      ctx.device
          ->create_image(ctx.device.self, attachment.depth_stencil_image_desc)
          .unwrap();

  attachment.depth_stencil_image_view_desc = gfx::ImageViewDesc{
      .label           = "Framebuffer Depth Stencil Image View"_span,
      .image           = attachment.depth_stencil_image,
      .view_type       = gfx::ImageViewType::Type2D,
      .view_format     = attachment.depth_stencil_image_desc.format,
      .mapping         = {},
      .aspects         = gfx::ImageAspects::Depth | gfx::ImageAspects::Stencil,
      .first_mip_level = 0,
      .num_mip_levels  = 1,
      .first_array_layer = 0,
      .num_array_layers  = 1};

  attachment.depth_stencil_image_view =
      ctx.device
          ->create_image_view(ctx.device.self,
                              attachment.depth_stencil_image_view_desc)
          .unwrap();

  attachment.extent = new_extent;
}

void RenderContext::uninit()
{
  device->destroy_pipeline_cache(device.self, pipeline_cache);
  device->destroy_image(device.self, framebuffer.color_image);
  device->destroy_image_view(device.self,
                             framebuffer.color_image_view);
  device->destroy_image(device.self,
                        framebuffer.depth_stencil_image);
  device->destroy_image_view(device.self,
                             framebuffer.depth_stencil_image_view);
  device->destroy_image(device.self,
                        scratch_framebuffer.color_image);
  device->destroy_image_view(device.self,
                             scratch_framebuffer.color_image_view);
  device->destroy_image(device.self,
                        scratch_framebuffer.depth_stencil_image);
  device->destroy_image_view(
      device.self, scratch_framebuffer.depth_stencil_image_view);
  device->destroy_descriptor_set_layout(device.self, ssbo_layout);
  device->destroy_descriptor_set_layout(device.self, textures_layout);
  idle_purge();
  released_images.reset();
  released_image_views.reset();
  shader_map.for_each([&](Span<char const>, gfx::Shader shader) {
    device->destroy_shader(device.self, shader);
  });
  shader_map.reset();
}

void RenderContext::recreate_framebuffers(gfx::Extent new_extent)
{
  recreate_attachment(*this, framebuffer, new_extent);
  recreate_attachment(*this, scratch_framebuffer, new_extent);
}

gfx::CommandEncoderImpl RenderContext::encoder()
{
  gfx::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.encoders[ctx.ring_index];
}

u32 RenderContext::ring_index()
{
  gfx::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.ring_index;
}

gfx::FrameId RenderContext::frame_id()
{
  gfx::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.current;
}

gfx::FrameId RenderContext::tail_frame_id()
{
  gfx::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.tail;
}

Option<gfx::Shader> RenderContext::get_shader(Span<char const> name)
{
  gfx::Shader *shader = shader_map[name];
  if (shader == nullptr)
  {
    return None;
  }
  return Some{*shader};
}

void RenderContext::release(gfx::Image image)
{
  if (image == nullptr)
  {
    return;
  }
  CHECK(released_images.push(frame_id(), image));
}

void RenderContext::release(gfx::ImageView view)
{
  if (view == nullptr)
  {
    return;
  }
  CHECK(released_image_views.push(frame_id(), view));
}

void RenderContext::purge()
{
  // TODO(lamarrr): preserve queue order, i.e. some resources need to be deleted
  // in certain order
  gfx::FrameId tail_frame = tail_frame_id();
  {
    auto [good, to_delete] =
        binary_partition(released_images, [tail_frame](auto const &r) {
          return r.v0 >= tail_frame;
        });
    for (auto const &r : to_span(released_images)[to_delete])
    {
      device->destroy_image(device.self, r.v1);
    }
    released_images.erase(to_delete);
  }

  {
    auto [good, to_delete] =
        binary_partition(released_image_views, [tail_frame](auto const &r) {
          return r.v0 >= tail_frame;
        });
    for (auto const &r : to_span(released_image_views)[to_delete])
    {
      device->destroy_image_view(device.self, r.v1);
    }
    released_image_views.erase(to_delete);
  }
}

void RenderContext::idle_purge()
{
  device->wait_idle(device.self).unwrap();
  purge();
}

void RenderContext::begin_frame(gfx::Swapchain swapchain)
{
  device->begin_frame(device.self, swapchain).unwrap();
  purge();
}

void RenderContext::end_frame(gfx::Swapchain swapchain)
{
  gfx::CommandEncoderImpl enc = encoder();
  if (swapchain != nullptr)
  {
    gfx::SwapchainState swapchain_state =
        device->get_swapchain_state(device.self, swapchain).unwrap();

    if (swapchain_state.current_image.is_some())
    {
      enc->blit_image(
          enc.self, framebuffer.color_image,
          swapchain_state.images[swapchain_state.current_image.unwrap()],
          to_span({gfx::ImageBlit{
              .src_layers = {.aspects           = gfx::ImageAspects::Color,
                             .mip_level         = 0,
                             .first_array_layer = 0,
                             .num_array_layers  = 1},
              .src_offsets =
                  {{0, 0, 0},
                   {framebuffer.color_image_desc.extent.x,
                    framebuffer.color_image_desc.extent.y, 1}},
              .dst_layers  = {.aspects           = gfx::ImageAspects::Color,
                              .mip_level         = 0,
                              .first_array_layer = 0,
                              .num_array_layers  = 1},
              .dst_offsets = {{0, 0, 0},
                              {swapchain_state.extent.x,
                               swapchain_state.extent.y, 1}}}}),
          gfx::Filter::Linear);
    }
  }
  device->submit_frame(device.self, swapchain).unwrap();
}

}        // namespace ash