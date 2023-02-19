#include "ashura/engine.h"

#include <fstream>

#include "ashura/asset_bundle.h"
#include "ashura/canvas.h"
#include "ashura/sample_image.h"
#include "ashura/sdl_utils.h"
#include "ashura/shaders.h"
#include "ashura/vulkan_recording_context.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace ash {

namespace impl {

// TODO(lamarrr): take a quick look at UE log file content and structure
static stx::Rc<spdlog::logger*> make_multi_threaded_logger(
    std::string name, std::string file_path) {
  stx::Vec<spdlog::sink_ptr> sinks{stx::os_allocator};
  sinks
      .push(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
          std::move(file_path)))
      .unwrap();

  sinks.push(std::make_shared<spdlog::sinks::stdout_color_sink_mt>()).unwrap();

  return stx::rc::make_inplace<spdlog::logger>(
             stx::os_allocator, std::move(name), sinks.begin(), sinks.end())
      .unwrap();
}
}  // namespace impl

inline stx::Option<stx::Span<vk::PhyDeviceInfo const>> select_device(
    stx::Span<vk::PhyDeviceInfo const> const phy_devices,
    stx::Span<VkPhysicalDeviceType const> preferred_device_types,
    vk::Surface const& target_surface) {
  for (VkPhysicalDeviceType type : preferred_device_types) {
    if (stx::Span selected =
            phy_devices.which([&](vk::PhyDeviceInfo const& dev) -> bool {
              return dev.properties.deviceType == type &&
                     // can use shaders (fragment and vertex)
                     dev.has_geometry_shader() &&
                     // has graphics command queue for rendering commands
                     dev.has_graphics_command_queue_family() &&
                     // has data transfer command queue for uploading textures
                     // or data
                     dev.has_transfer_command_queue_family() &&
                     // can be used for presenting to a specific surface
                     !vk::get_surface_presentation_command_queue_support(
                          dev.phy_device, dev.family_properties,
                          target_surface.surface)
                          .span()
                          .find(true)
                          .is_empty();
            });
        !selected.is_empty()) {
      return stx::Some(std::move(selected));
    }
  }

  return stx::None;
}

gfx::CachedFont* font;
gfx::image img;
stx::Rc<vk::ImageSampler*>* sampler;

Engine::Engine(AppConfig const& cfg) {
  stx::Vec<char const*> required_device_extensions{stx::os_allocator};

  required_device_extensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME).unwrap();

  stx::Vec<char const*> required_validation_layers{stx::os_allocator};

  if (cfg.enable_validation_layers) {
    required_validation_layers.push("VK_LAYER_KHRONOS_validation").unwrap();
  }

  logger = stx::Some(
      impl::make_multi_threaded_logger("ashura", cfg.log_file.c_str()));

  auto& xlogger = *logger.value().handle;

  xlogger.info("Initializing Window API");

  window_api =
      stx::Some(stx::rc::make_inplace<WindowApi>(stx::os_allocator).unwrap());

  xlogger.info("Initialized Window API");
  xlogger.info("Creating root window");

  window = stx::Some(
      create_window(window_api.value().share(), cfg.window_config.copy()));

  xlogger.info("Created root window");

  stx::Vec window_required_instance_extensions =
      window.value()->get_required_instance_extensions();

  stx::Rc<vk::Instance*> vk_instance = vk::create_instance(
      cfg.name.c_str(), VK_MAKE_VERSION(0, 0, 1), cfg.name.c_str(),
      VK_MAKE_VERSION(cfg.version.major, cfg.version.minor, cfg.version.patch),
      window_required_instance_extensions, required_validation_layers);

  window.value()->attach_surface(vk_instance.share());

  stx::Vec phy_devices = vk::get_all_devices(vk_instance);

  VkPhysicalDeviceType const device_preference[] = {
      VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
      VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
      VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, VK_PHYSICAL_DEVICE_TYPE_CPU};

  xlogger.info("Available Physical Devices:");

  for (vk::PhyDeviceInfo const& device : phy_devices) {
    xlogger.info("\t{}", vk::format(device));
    // TODO(lamarrr): log graphics families on devices and other properties
  }

  stx::Rc<vk::PhyDeviceInfo*> phy_device =
      stx::rc::make(
          stx::os_allocator,
          select_device(phy_devices, device_preference,
                        *window.value()->surface.value().handle)
              .expect("Unable to find any suitable rendering device")[0]
              .copy())
          .unwrap();

  xlogger.info("Selected Physical Device: {}", vk::format(*phy_device.handle));

  // we might need multiple command queues, one for data transfer and one for
  // rendering
  f32 queue_priorities[] = {// priority for command queue used for
                            // presentation, rendering, data transfer
                            1};

  stx::Rc graphics_command_queue_family =
      stx::rc::make(stx::os_allocator,
                    vk::get_graphics_command_queue(phy_device)
                        .expect("Unable to get graphics command queue"))
          .unwrap();

  // we can accept queue family struct here instead and thus not have to
  // perform extra manual checks
  // the user shouldn't have to touch handles
  VkDeviceQueueCreateInfo command_queue_create_infos[] = {
      {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
       .pNext = nullptr,
       .flags = 0,
       .queueFamilyIndex = graphics_command_queue_family->index,
       .queueCount = AS_U32(std::size(queue_priorities)),
       .pQueuePriorities = queue_priorities}};

  VkPhysicalDeviceFeatures required_features{};

  required_features.samplerAnisotropy = VK_TRUE;

  stx::Rc<vk::Device*> device = vk::create_device(
      phy_device, command_queue_create_infos, required_device_extensions,
      required_validation_layers, required_features);

  stx::Rc<vk::CommandQueue*> xqueue =
      stx::rc::make_inplace<vk::CommandQueue>(
          stx::os_allocator,
          vk::get_command_queue(device, *graphics_command_queue_family.handle,
                                0)
              .expect("Failed to create graphics command queue"))
          .unwrap();

  queue = stx::Some(xqueue.share());

  window.value()->recreate_swapchain(xqueue);

  renderer = stx::Some(stx::rc::make_inplace<vk::CanvasRenderer>(
                           stx::os_allocator, xqueue.share(),
                           vk::SwapChain::MAX_FRAMES_IN_FLIGHT)
                           .unwrap());

  auto& swp = window.value()->surface.value()->swapchain.value();
  renderer.value()->ctx.rebuild(swp.render_pass, swp.msaa_sample_count);

  window.value()->on(WindowEvent::Resized,
                     stx::fn::rc::make_unique_functor(stx::os_allocator, []() {
                       ASH_LOG("resized");
                     }).unwrap());

  window.value()
      ->mouse_motion_listeners
      .push(stx::fn::rc::make_unique_static([](MouseMotionEvent) {}))
      .unwrap();

  u8 transparent_image_data[] = {0xFF, 0xFF, 0xFF, 0xFF};

  gfx::image transparent_image = image_bundle.add(
      renderer.value()->ctx.upload_image(transparent_image_data, {1, 1}, 4));

  img = image_bundle.add(
      renderer.value()->ctx.upload_image(sample_image, {1920, 1080}, 4));

  ASH_CHECK(transparent_image == 0);

  sampler = new stx::Rc<vk::ImageSampler*>{
      vk::create_image_sampler(queue.value()->device, VK_FILTER_LINEAR,
                               VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_TRUE)};

  font = new gfx::CachedFont[]{
      renderer.value()->ctx.cache_font(
          image_bundle,
          load_font_from_file(
              R"(C:\Users\Basit\OneDrive\Documents\workspace\oss\ashura\assets\fonts\JetBrainsMono-Regular.ttf)"_str)
              .unwrap(),
          26),
      renderer.value()->ctx.cache_font(
          image_bundle,
          load_font_from_file(
              R"(C:\Users\Basit\OneDrive\Desktop\segoeuiemoji\seguiemj.ttf)"_str)
              .unwrap(),
          50),
      renderer.value()->ctx.cache_font(
          image_bundle,
          load_font_from_file(
              R"(C:\Users\Basit\OneDrive\Desktop\adobe-arabic-regular\Adobe Arabic Regular\Adobe Arabic Regular.ttf)"_str)
              .unwrap(),
          26),
      renderer.value()->ctx.cache_font(
          image_bundle,
          load_font_from_file(
              R"(C:\Users\Basit\OneDrive\Documents\workspace\oss\ashura-assets\fonts\MaterialIcons-Regular.ttf)"_str)
              .unwrap(),
          50)};

  canvas = stx::Some(gfx::Canvas{{0, 0}});

  window.value()->on(WindowEvent::Close,
                     stx::fn::rc::make_unique_functor(stx::os_allocator, []() {
                       std::exit(0);
                     }).unwrap());

  window.value()->on(WindowEvent::Resized, stx::fn::rc::make_unique_functor(
                                               stx::os_allocator,
                                               [win = window.value().handle]() {
                                                 win->needs_resizing = true;
                                               })
                                               .unwrap());

  window.value()->on(
      WindowEvent::SizeChanged,
      stx::fn::rc::make_unique_functor(
          stx::os_allocator,
          [win = window.value().handle]() { win->needs_resizing = true; })
          .unwrap());
}

void Engine::tick(std::chrono::nanoseconds interval) {
  // poll events to make the window not be marked as unresponsive.
  // we also poll events from SDL's event queue until there are none left.
  //
  // any missed event should be rolled over to the next tick()
  do {
  } while (window_api.value()->poll_events());

  window.value()->tick(interval);

  auto record_draw_commands = [&]() {
    VkExtent2D extent =
        window.value()->surface_.value()->swapchain.value().window_extent;

    gfx::Canvas& c = canvas.value();
    static int x = 0;
    static auto start = std::chrono::steady_clock::now();

    auto d = std::chrono::duration_cast<std::chrono::seconds>(
                 std::chrono::steady_clock::now() - start)
                 .count();

    c.restart({AS_F32(extent.width), AS_F32(extent.height)});
    c.brush.color = colors::WHITE;
    c.clear();
    c.brush.fill = true;
    c.brush.color = colors::GREEN;
    c.draw_rect({{800, 800}, {300, 100}});
    c.brush.line_thickness = 2;
    c.brush.fill = false;
    c.brush.color = colors::RED;
    c.draw_rect({{90, 400}, {320, 120}});
    c.brush.color = colors::WHITE;
    auto str = fmt::format(
        "Hello World! Examples Ashura Engine Demo.\n Starting in {}", d);
    char arstr[] = {0xd9, 0x84, 0xd8, 0xa7, 0x20, 0xd8, 0xa5, 0xd9, 0x84, 0xd9,
                    0x87, 0x20, 0xd8, 0xa5, 0xd9, 0x84, 0xd8, 0xa7, 0x20, 0xd8,
                    0xa7, 0xd9, 0x84, 0xd9, 0x84, 0xd9, 0x87, 0x20, 0xd9, 0x88,
                    0xd8, 0xa7, 0xd9, 0x84, 0xd9, 0x84, 0xd9, 0x87, 0x20, 0xd8,
                    0xa3, 0xd9, 0x83, 0xd8, 0xa8, 0xd8, 0xb1, 0};
    char emojis[] = {
        0xf0, 0x9f, 0x98, 0x80, 0x20, 0xf0, 0x9f, 0x98, 0x83, 0x20, 0xf0, 0x9f,
        0x98, 0x84, 0x20, 0xf0, 0x9f, 0x98, 0x81, 0x20, 0xf0, 0x9f, 0x98, 0x86,
        0x20, 0xf0, 0x9f, 0x98, 0x85, 0x20, 0xf0, 0x9f, 0x98, 0x82, 0x20, 0xf0,
        0x9f, 0xa4, 0xa3, 0x20, 0xf0, 0x9f, 0xa5, 0xb2, 0x20, 0xf0, 0x9f, 0xa5,
        0xb9, 0x20, 0xe2, 0x98, 0xba, 0xef, 0xb8, 0x8f, 0x20, 0xf0, 0x9f, 0x98,
        0x8a, 0x20, 0xf0, 0x9f, 0x98, 0x87, 0x20, 0xf0, 0x9f, 0x99, 0x82, 0x20,
        0xf0, 0x9f, 0x99, 0x83, 0x20, 0xf0, 0x9f, 0x98, 0x89, 0x20, 0xf0, 0x9f,
        0x98, 0x8c, 0x20, 0xf0, 0x9f, 0x98, 0x8d, 0x20, 0};
    TextRun runs[] = {
        {.text = str,
         .font = 0,
         .style = {.font_height = 30,
                   .letter_spacing = 1,
                   .word_spacing = 16,
                   .foreground_color = colors::CYAN,
                   .background_color = colors::BLACK}},
        {.text = str,
         .font = 0,
         .style = {.font_height = 18,
                   .foreground_color = colors::BLACK,
                   .background_color = color::from_rgb(0x33, 0x33, 0x33),
                   .underline_color = colors::GREEN,
                   .underline_thickness = 1}},
        {.text = arstr,
         .font = 2,
         .style = {.font_height = 30,
                   .letter_spacing = 0,
                   .foreground_color = colors::BLACK,
                   .background_color = colors::GREEN,
                   .underline_color = colors::MAGENTA,
                   .underline_thickness = 1},
         .direction = TextDirection::RightToLeft,
         .script = HB_SCRIPT_ARABIC,
         .language = hb_language_from_string("ar", -1)},
        {.text = emojis,
         .font = 1,
         .style = {.font_height = 20,
                   .letter_spacing = 0,
                   .word_spacing = 15,
                   .foreground_color = colors::WHITE,
                   .background_color = colors::BLACK.with_alpha(0)}},
        {.text = "Face with "
                 "Tears "
                 "of "
                 "Joy",
         .font = 1,
         .style = {.font_height = 50,
                   .letter_spacing = 0,
                   .word_spacing = 15,
                   .foreground_color = colors::WHITE,
                   .background_color = colors::BLACK}},
        {.text = "verified",
         .font = 3,
         .style = {.font_height = 50,
                   .letter_spacing = 0,
                   .word_spacing = 15,
                   .foreground_color = color::from_rgb(29, 155, 240),
                   .background_color = colors::WHITE}}};
    Paragraph paragraph{.runs = runs, .align = TextAlign::Right};
    stx::Vec<gfx::RunSubWord> subwords{stx::os_allocator};
    stx::Vec<gfx::SubwordGlyph> glyphs{stx::os_allocator};
    c.draw_text(paragraph, stx::Span{font, 4}, {100, 500}, /*300*/ 500,
                subwords, glyphs);

    // c.draw_image(atlas->atlas, rect{{0, 0},
    // {atlas->extent.width * 1.0f,
    //  atlas->extent.height * 1.0f}});
    c.brush.color = colors::WHITE.with_alpha(255);
    c.brush.fill = true;
    c.scale(4, 4);
    c.brush.texture = img;
    c.draw_round_rect({{0, 0}, {100, 100}}, {25, 25, 25, 25}, 360);
  };

  record_draw_commands();
  // only try to present if the pipeline has new changes or window was
  // resized

  // only try to recreate swapchain if the present swapchain can't be used for
  // presentation

  WindowSwapchainDiff swapchain_diff = WindowSwapchainDiff::None;

  do {
    if (swapchain_diff != WindowSwapchainDiff::None) {
      window.value()->recreate_swapchain(queue.value());
      auto& swp = window.value()->surface.value()->swapchain.value();
      renderer.value()->ctx.rebuild(swp.render_pass, swp.msaa_sample_count);
      record_draw_commands();
    }

    vk::SwapChain& swapchain =
        window.value()->surface.value()->swapchain.value();

    auto [diff, swapchain_image_index] = window.value()->acquire_image();

    swapchain_diff = diff;

    if (swapchain_diff != WindowSwapchainDiff::None) {
      continue;
    }

    gfx::DrawList const& draw_list = canvas.value().draw_list;

    renderer.value()->submit(
        swapchain.window_extent, swapchain.image_extent, swapchain_image_index,
        swapchain.frame, swapchain.render_fences[swapchain.frame],
        swapchain.image_acquisition_semaphores[swapchain.frame],
        swapchain.render_semaphores[swapchain.frame], swapchain.render_pass,
        swapchain.framebuffers[swapchain_image_index], (*sampler)->sampler,
        draw_list.cmds, draw_list.vertices, draw_list.indices, image_bundle);

    swapchain_diff = window.value()->present(swapchain_image_index);

    // the frame semaphores and synchronization primitives are still used even
    // if an error is returned
    swapchain.frame =
        (swapchain.frame + 1) % vk::SwapChain::MAX_FRAMES_IN_FLIGHT;

  } while (swapchain_diff != WindowSwapchainDiff::None);
}

}  // namespace ash
