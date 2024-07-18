/// SPDX-License-Identifier: MIT
#include "ashura/engine/window.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "ashura/gfx/vulkan.h"
#include "ashura/std/error.h"
#include "ashura/std/sparse_vec.h"
#include "ashura/std/vec.h"

namespace ash
{

#define CHECKSdl(cond_expr) CHECK_DESC(cond_expr, "SDL Error: ", SDL_GetError())

namespace sdl
{
struct WindowEventListener
{
  Fn<void(WindowEvent const &)> callback = fn([](WindowEvent const &) {});
  WindowEventTypes              types    = WindowEventTypes::None;
};

struct WindowImpl
{
  SDL_Window              *win              = nullptr;
  gfx::Surface             surface          = nullptr;
  uid                      backend_id       = UID_MAX;
  Vec<WindowEventListener> listeners        = {};
  SparseVec                listeners_id_map = {};
  gfx::InstanceImpl        instance         = {};
};

struct WindowSystemImpl final : public WindowSystem
{
  SDL_Window *hnd(Window window)
  {
    return ((WindowImpl *) window)->win;
  }

  void init()
  {
    CHECKSdl(!SDL_Init(SDL_INIT_VIDEO));
  }

  Option<Window> create_window(gfx::InstanceImpl instance,
                               Span<char const>  title) override
  {
    char *title_c_str;
    if (!default_allocator.nalloc(title.size() + 1, &title_c_str))
    {
      return None;
    }

    defer title_del{
        [&] { default_allocator.ndealloc(title_c_str, title.size() + 1); }};

    mem::copy(title, title_c_str);
    title_c_str[title.size()] = 0;

    SDL_Window *window = SDL_CreateWindow(
        title_c_str, 1920, 1080, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    CHECKSdl(window != nullptr);
    uid backend_id = SDL_GetWindowID(window);
    CHECKSdl(backend_id != 0);

    CHECK(instance.interface->get_backend(instance.self) ==
          gfx::Backend::Vulkan);

    vk::Instance *vk_instance = (vk::Instance *) instance.self;
    VkSurfaceKHR  surface;

    CHECKSdl(SDL_Vulkan_CreateSurface(window, vk_instance->vk_instance, nullptr,
                                      &surface) == SDL_TRUE);

    WindowImpl *impl;

    CHECK(default_allocator.nalloc(1, &impl));

    new (impl) WindowImpl{.win        = window,
                          .surface    = (gfx::Surface) surface,
                          .backend_id = backend_id,
                          .instance   = instance};

    SDL_PropertiesID props_id = SDL_GetWindowProperties(window);
    CHECK(SDL_SetProperty(props_id, "impl_object", impl) == 0);

    return Some{(Window) impl};
  }

  void destroy_window(Window window) override
  {
    if (window != nullptr)
    {
      WindowImpl *win = (WindowImpl *) window;
      win->instance->destroy_surface(win->instance.self, win->surface);
      SDL_DestroyWindow(win->win);
      win->listeners_id_map.reset(win->listeners);
      default_allocator.ndealloc(win, 1);
    }
  }

  void set_title(Window window, Span<char const> title) override
  {
    char *title_c_str;
    CHECK(default_allocator.nalloc(title.size() + 1, &title_c_str));

    defer title_del{
        [&] { default_allocator.ndealloc(title_c_str, title.size() + 1); }};

    mem::copy(title, title_c_str);
    title_c_str[title.size()] = 0;

    CHECKSdl(!SDL_SetWindowTitle(hnd(window), title_c_str));
  }

  char const *get_title(Window window) override
  {
    char const *title = SDL_GetWindowTitle(hnd(window));
    CHECKSdl(title != nullptr);
    return title;
  }

  void maximize(Window window) override
  {
    CHECKSdl(!SDL_MaximizeWindow(hnd(window)));
  }

  void minimize(Window window) override
  {
    CHECKSdl(!SDL_MinimizeWindow(hnd(window)));
  }

  void set_size(Window window, Vec2U size) override
  {
    CHECKSdl(!SDL_SetWindowSize(hnd(window), static_cast<int>(size.x),
                                static_cast<int>(size.y)));
  }

  void center(Window window) override
  {
    CHECKSdl(!SDL_SetWindowPosition(hnd(window), SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED));
  }

  Vec2U get_size(Window window) override
  {
    int width, height;
    CHECKSdl(!SDL_GetWindowSize(hnd(window), &width, &height));
    return Vec2U{static_cast<u32>(width), static_cast<u32>(height)};
  }

  Vec2U get_surface_size(Window window) override
  {
    int width, height;
    CHECKSdl(!SDL_GetWindowSizeInPixels(hnd(window), &width, &height));
    return Vec2U{static_cast<u32>(width), static_cast<u32>(height)};
  }

  void set_position(Window window, Vec2I pos) override
  {
    CHECKSdl(!SDL_SetWindowPosition(hnd(window), pos.x, pos.y));
  }

  Vec2I get_position(Window window) override
  {
    int x, y;
    CHECKSdl(!SDL_GetWindowPosition(hnd(window), &x, &y));
    return Vec2I{x, y};
  }

  void set_min_size(Window window, Vec2U min) override
  {
    CHECKSdl(!SDL_SetWindowMinimumSize(hnd(window), static_cast<int>(min.x),
                                       static_cast<int>(min.y)));
  }

  Vec2U get_min_size(Window window) override
  {
    int width, height;
    CHECKSdl(!SDL_GetWindowMinimumSize(hnd(window), &width, &height));
    return Vec2U{static_cast<u32>(width), static_cast<u32>(height)};
  }

  void set_max_size(Window window, Vec2U max) override
  {
    CHECKSdl(!SDL_SetWindowMaximumSize(hnd(window), static_cast<int>(max.x),
                                       static_cast<int>(max.y)));
  }

  Vec2U get_max_size(Window window) override
  {
    int width, height;
    CHECKSdl(!SDL_GetWindowMaximumSize(hnd(window), &width, &height));
    return Vec2U{static_cast<u32>(width), static_cast<u32>(height)};
  }

  void set_icon(Window window, ImageSpan<u8 const, 4> image,
                gfx::Format format) override
  {
    SDL_PixelFormatEnum fmt = SDL_PIXELFORMAT_RGBA8888;

    switch (format)
    {
      case gfx::Format::R8G8B8A8_UNORM:
        fmt = SDL_PIXELFORMAT_RGBA8888;
        break;
      case gfx::Format::B8G8R8A8_UNORM:
        fmt = SDL_PIXELFORMAT_BGRA8888;
        break;
      default:
        CHECK_DESC(false, "unsupported image format");
    }

    SDL_Surface *icon = SDL_CreateSurfaceFrom(
        (void *) image.channels.data(), static_cast<int>(image.width),
        static_cast<int>(image.height), static_cast<int>(image.pitch()), fmt);
    CHECKSdl(icon != nullptr);
    CHECKSdl(!SDL_SetWindowIcon(hnd(window), icon));
    SDL_DestroySurface(icon);
  }

  void make_bordered(Window window) override
  {
    CHECKSdl(!SDL_SetWindowBordered(hnd(window), SDL_TRUE));
  }

  void make_borderless(Window window) override
  {
    CHECKSdl(!SDL_SetWindowBordered(hnd(window), SDL_FALSE));
  }

  void show(Window window) override
  {
    CHECKSdl(!SDL_ShowWindow(hnd(window)));
  }

  void hide(Window window) override
  {
    CHECKSdl(!SDL_HideWindow(hnd(window)));
  }

  void raise(Window window) override
  {
    CHECKSdl(!SDL_RaiseWindow(hnd(window)));
  }

  void restore(Window window) override
  {
    CHECKSdl(!SDL_RestoreWindow(hnd(window)));
  }

  void request_attention(Window window, bool briefly) override
  {
    CHECKSdl(!SDL_FlashWindow(hnd(window), briefly ? SDL_FLASH_BRIEFLY :
                                                     SDL_FLASH_UNTIL_FOCUSED));
  }

  void make_fullscreen(Window window) override
  {
    CHECKSdl(!SDL_SetWindowFullscreen(hnd(window), SDL_TRUE));
  }

  void make_windowed(Window window) override
  {
    CHECKSdl(!SDL_SetWindowFullscreen(hnd(window), SDL_FALSE));
  }

  void make_resizable(Window window) override
  {
    CHECKSdl(!SDL_SetWindowResizable(hnd(window), SDL_TRUE));
  }

  void make_unresizable(Window window) override
  {
    CHECKSdl(!SDL_SetWindowResizable(hnd(window), SDL_FALSE));
  }

  uid listen(Window window, WindowEventTypes event_types,
             Fn<void(WindowEvent const &)> callback) override
  {
    uid         out_id;
    WindowImpl *pwin = (WindowImpl *) window;
    CHECK(pwin->listeners_id_map.push(
        [&](uid id, u32) {
          out_id = id;
          CHECK(pwin->listeners.push(callback, event_types));
        },
        pwin->listeners));
    return out_id;
  }

  void unlisten(Window window, uid listener) override
  {
    WindowImpl *pwin = (WindowImpl *) window;
    pwin->listeners_id_map.erase(listener, pwin->listeners);
  }

  gfx::Surface get_surface(Window window) override
  {
    WindowImpl *pwin = (WindowImpl *) window;
    return pwin->surface;
  }

  void publish_event(SDL_WindowID window_id, WindowEvent const &event)
  {
    SDL_Window *win = SDL_GetWindowFromID(window_id);
    CHECK(win != nullptr);
    SDL_PropertiesID props_id = SDL_GetWindowProperties(win);
    WindowImpl      *impl =
        (WindowImpl *) SDL_GetProperty(props_id, "impl_object", nullptr);
    CHECK(impl != nullptr);

    for (WindowEventListener const &listener : impl->listeners)
    {
      if (has_bits(listener.types, event.type))
      {
        listener.callback(event);
      }
    }
  }

  void poll_events() override
  {
    SDL_Event event;

    while (SDL_PollEvent(&event) == SDL_TRUE)
    {
      switch (event.type)
      {
        case SDL_EVENT_WINDOW_SHOWN:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Shown});
          return;
        case SDL_EVENT_WINDOW_HIDDEN:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Hidden});
          return;
        case SDL_EVENT_WINDOW_EXPOSED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Exposed});
          return;
        case SDL_EVENT_WINDOW_MOVED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Moved});
          return;
        case SDL_EVENT_WINDOW_RESIZED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Resized});
          return;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
          publish_event(event.window.windowID,
                        WindowEvent{.none_ = 0,
                                    .type  = WindowEventTypes::SurfaceResized});
          return;
        case SDL_EVENT_WINDOW_MINIMIZED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Minimized});
          return;
        case SDL_EVENT_WINDOW_MAXIMIZED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Maximized});
          return;
        case SDL_EVENT_WINDOW_RESTORED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Restored});
          return;
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::MouseEnter});
          return;
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::MouseLeave});
          return;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::FocusGained});
          return;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::FocusLost});
          return;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
          publish_event(event.window.windowID,
                        WindowEvent{.none_ = 0,
                                    .type  = WindowEventTypes::CloseRequested});
          return;
        case SDL_EVENT_WINDOW_TAKE_FOCUS:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::TakeFocus});
          return;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
          MouseClickEvent mouse_event{.mouse_id = event.button.which,
                                      .position =
                                          Vec2{event.button.x, event.button.y},
                                      .clicks = event.button.clicks};
          switch (event.button.button)
          {
            case SDL_BUTTON_LEFT:
              mouse_event.button = MouseButtons::Primary;
              break;
            case SDL_BUTTON_RIGHT:
              mouse_event.button = MouseButtons::Secondary;
              break;
            case SDL_BUTTON_MIDDLE:
              mouse_event.button = MouseButtons::Middle;
              break;
            case SDL_BUTTON_X1:
              mouse_event.button = MouseButtons::A1;
              break;
            case SDL_BUTTON_X2:
              mouse_event.button = MouseButtons::A2;
              break;
            default:
              UNREACHABLE();
          }

          switch (event.type)
          {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
              mouse_event.action = KeyAction::Press;
              break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
              mouse_event.action = KeyAction::Release;
              break;

            default:
              return;
          }

          publish_event(event.button.windowID,
                        WindowEvent{.mouse_click = mouse_event,
                                    .type = WindowEventTypes::MouseClick});
          return;
        }

        case SDL_EVENT_MOUSE_MOTION:
          publish_event(
              event.motion.windowID,
              WindowEvent{
                  .mouse_motion =
                      MouseMotionEvent{
                          .mouse_id = event.motion.which,
                          .position = Vec2{event.motion.x, event.motion.y},
                          .translation =
                              Vec2{event.motion.xrel, event.motion.yrel}},
                  .type = WindowEventTypes::MouseMotion});
          return;

        case SDL_EVENT_MOUSE_WHEEL:
          publish_event(
              event.wheel.windowID,
              WindowEvent{
                  .mouse_wheel =
                      MouseWheelEvent{
                          .mouse_id = event.wheel.which,
                          .position =
                              Vec2{event.wheel.mouse_x, event.wheel.mouse_y},
                          .translation = Vec2{event.wheel.x, event.wheel.y}},
                  .type = WindowEventTypes::MouseWheel});
          return;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
          publish_event(
              event.key.windowID,
              WindowEvent{
                  .key =
                      KeyEvent{
                          .scan_code = (ScanCode) (event.key.keysym.scancode),
                          .key_code  = (KeyCode) ((u16) event.key.keysym.sym &
                                                 ~SDLK_SCANCODE_MASK),
                          .modifiers =
                              static_cast<KeyModifiers>(event.key.keysym.mod),
                          .action = event.type == SDL_EVENT_KEY_DOWN ?
                                        KeyAction::Press :
                                        KeyAction::Release},
                  .type = WindowEventTypes::Key});
          return;

        case SDL_EVENT_WINDOW_DESTROYED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Destroyed});
          return;

        case SDL_EVENT_SYSTEM_THEME_CHANGED:
          return;
        case SDL_EVENT_FINGER_DOWN:
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_MOTION:
          return;
        case SDL_EVENT_DROP_BEGIN:
        case SDL_EVENT_DROP_COMPLETE:
        case SDL_EVENT_DROP_FILE:
        case SDL_EVENT_DROP_POSITION:
        case SDL_EVENT_DROP_TEXT:
          return;
        case SDL_EVENT_TEXT_EDITING:
        case SDL_EVENT_TEXT_INPUT:
        case SDL_EVENT_KEYMAP_CHANGED:
        case SDL_EVENT_AUDIO_DEVICE_ADDED:
        case SDL_EVENT_AUDIO_DEVICE_REMOVED:
        case SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED:
        case SDL_EVENT_DISPLAY_ORIENTATION:
        case SDL_EVENT_DISPLAY_ADDED:
        case SDL_EVENT_DISPLAY_REMOVED:
        case SDL_EVENT_DISPLAY_MOVED:
          return;

        default:
          return;
      }
    }
  }
};

/*
  // void enable_hit_testing();
  //
  // SDL_HITTEST_NORMAL
  // region is normal and has no special properties
  // SDL_HITTEST_DRAGGABLE
  // region can drag entire window
  // SDL_HITTEST_RESIZE_TOPLEFT
  // region can resize top left window
  // SDL_HITTEST_RESIZE_TOP
  // region can resize top window
  // SDL_HITTEST_RESIZE_TOPRIGHT
  // region can resize top right window
  // SDL_HITTEST_RESIZE_RIGHT
  // region can resize right window
  // SDL_HITTEST_RESIZE_BOTTOMRIGHT
  // region can resize bottom right window
  // SDL_HITTEST_RESIZE_BOTTOM
  // region can resize bottom window
  // SDL_HITTEST_RESIZE_BOTTOMLEFT
  // region can resize bottom left window
  // SDL_HITTEST_RESIZE_LEFT
  // region can resize left window
  //
*/

}        // namespace sdl

WindowSystem *init_sdl_window_system()
{
  static sdl::WindowSystemImpl impl;
  impl.init();
  return &impl;
}
}        // namespace ash