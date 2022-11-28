#pragma once

#include <chrono>

#include "ashura/app_config.h"
#include "ashura/canvas.h"
#include "ashura/version.h"
#include "ashura/vulkan.h"
#include "ashura/window.h"
#include "ashura/window_api.h"
#include "spdlog/logger.h"
#include "stx/rc.h"
#include "stx/string.h"

namespace asr {

struct Engine {
  Engine(AppConfig const& cfg);

  stx::Option<stx::Rc<spdlog::logger*>> logger;
  stx::Option<stx::Rc<WindowApi*>> window_api;
  stx::Option<stx::Rc<Window*>> window;
  stx::Option<stx::Rc<vk::CommandQueue*>> queue;
  stx::Option<gfx::Canvas> canvas;
  stx::Option<stx::Rc<gfx::CanvasContext*>> canvas_context;

  // asset manager
  // plugins & systems

  void tick(std::chrono::nanoseconds interval);
};

}  // namespace asr