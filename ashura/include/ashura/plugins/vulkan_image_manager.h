#pragma once
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <string_view>

#include "ashura/image.h"
#include "ashura/image_decoder.h"
#include "ashura/plugin.h"
#include "ashura/plugins/image_manager.h"
#include "ashura/primitives.h"
#include "ashura/vulkan.h"
#include "ashura/vulkan_context.h"
#include "stx/memory.h"
#include "stx/rc.h"
#include "stx/scheduler.h"
#include "stx/scheduler/scheduling/schedule.h"
#include "stx/span.h"
#include "stx/void.h"

namespace ash
{

struct VulkanImageManager : public ImageManager
{
  VulkanImageManager(vk::RenderResourceManager &imgr) :
      mgr{&imgr}
  {}

  virtual constexpr void on_startup(Context &context) override
  {}

  virtual constexpr void tick(Context &context, std::chrono::nanoseconds interval) override
  {}

  virtual constexpr void on_exit(Context &context) override
  {}

  virtual constexpr ~VulkanImageManager() override
  {}

  virtual gfx::image add(ImageView view, bool is_real_time) override
  {
    return mgr->add_image(view, is_real_time);
  }

  virtual void update(gfx::image image, ImageView view)
  {
    mgr->update(image, view);
  }

  virtual void remove(gfx::image image) override
  {
    mgr->remove(image);
  }

  vk::RenderResourceManager *mgr = nullptr;
};

}        // namespace ash