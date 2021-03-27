
#pragma once

#include <chrono>
#include <cinttypes>
#include <functional>
#include <string>
#include <string_view>

#include "stx/option.h"
#include "stx/report.h"
#include "stx/span.h"
#include "vlk/utils/limits.h"
#include "vlk/utils/utils.h"

#include "vlk/ui/canvas.h"
#include "vlk/ui/layout.h"

// TODO(lamarrr): should we pass a rasterization constext to the draw function?
// (do draw commands keep references to image data)?
// can we abstract the datasource provider to destroy returned memeory
// immediately after use so we can copy elsewhere? also, we need to have async
// first asset loading as loading synchronously will be slow

namespace vlk {
namespace ui {

/// NOTE: Widget is a very large struct (about 420-bytes). avoid
/// touching the struct in hot code paths as it could disrupt cache
/// if you're touching a large number of them, especially whilst not all fields
/// of it are touched
/// NOTE: this struct's data is always accessed from the main thread.
struct Widget {
  friend struct WidgetStateProxyAccessor;

  struct DebugInfo {
    DebugInfo(std::string_view const &name = "<unnamed>",
              std::string_view const &type_hint = "<none>",
              std::string_view const &info = "<none>")
        : name{name}, type_hint{type_hint}, info{info} {}

    std::string_view name;
    std::string_view type_hint;
    std::string_view info;
  };

  struct StateProxy {
    StateProxy()
        : on_render_dirty{[] {}},
          on_layout_dirty{[] {}},
          on_children_changed{[] {}},
          on_view_offset_dirty{[](ViewOffset const &) {}} {}

    /// informs the system that the widget's render data has changed
    std::function<void()> on_render_dirty;

    /// informs the system that the widget's layout has changed
    std::function<void()> on_layout_dirty;

    /// informs the system that the widget's children has changed (possibly
    /// requiring a full rebuild)
    std::function<void()> on_children_changed;

    /// informs the system that a view-widgets's offset (or visible area)
    /// has changed
    std::function<void(ViewOffset const &)> on_view_offset_dirty;
  };

  enum class Type : uint8_t {
    /// occupies space and has render data
    Render,
    /// marks a separate composition context, can have render data
    View
  };

  Widget(Type const &type = Type::Render, SelfExtent const &self_extent = {},
         Flex const &flex = {}, stx::Span<Widget *const> const &children = {},
         DebugInfo const &debug_info = DebugInfo{},
         stx::Option<ZIndex> const &z_index = stx::None,
         ViewExtent const &view_extent = {}, ViewOffset const &view_offset = {})
      : type_{type},
        self_extent_{self_extent},
        flex_{flex},
        children_{children},
        debug_info_{debug_info},
        z_index_{z_index.clone()},
        view_extent_{view_extent},
        view_offset_{view_offset},
        state_proxy_{} {}

  Type get_type() const { return type_; }

  SelfExtent get_self_extent() const { return self_extent_; }

  Flex get_flex() const { return flex_; }

  stx::Span<Widget *const> get_children() const { return children_; }

  bool has_children() const { return !get_children().empty(); }

  DebugInfo get_debug_info() const { return debug_info_; }

  stx::Option<ZIndex> get_z_index() const { return z_index_.clone(); }

  ViewExtent get_view_extent() const { return view_extent_; }

  ViewOffset get_view_offset() const { return view_offset_; }

  /// create draw commands
  virtual void draw([[maybe_unused]] Canvas &canvas) {
    // no-op
  }

  /// process any event you need to process here.
  virtual void tick([[maybe_unused]] std::chrono::nanoseconds const &interval) {
    // no-op
  }

  virtual ~Widget() {}

  void init_type(Type type) { type_ = type; }

  void update_self_extent(SelfExtent const &self_extent) {
    self_extent_ = self_extent;
    mark_layout_dirty();
  }

  // NOTE: this widget will now bind itself to this string
  void set_debug_info(DebugInfo const &info) { debug_info_ = info; }

  void init_z_index(stx::Option<ZIndex> const &z_index) {
    z_index_ = z_index.clone();
  }

  void update_flex(Flex const &flex) {
    flex_ = flex;
    mark_layout_dirty();
  }

  void update_view_extent(ViewExtent view_extent) {
    VLK_DEBUG_ENSURE(type_ == Type::View);
    view_extent_ = view_extent;
    mark_layout_dirty();
  }

  void update_view_offset(ViewOffset view_offset) {
    VLK_DEBUG_ENSURE(type_ == Type::View);
    view_offset_ = view_offset;
    mark_view_offset_dirty(view_offset_);
    mark_render_dirty();
  }

  /// MOTE: this does not free the memory associated with the referenced
  /// container. the derived widget is in charge of freeing memory as necessary
  void update_children(stx::Span<Widget *const> children) {
    children_ = children;
    mark_children_dirty();
  }

  void mark_children_dirty() const { state_proxy_.on_children_changed(); }

  void mark_layout_dirty() const { state_proxy_.on_layout_dirty(); }

  void mark_view_offset_dirty(ViewOffset const &offset) const {
    state_proxy_.on_view_offset_dirty(offset);
  }

  void mark_render_dirty() const { state_proxy_.on_render_dirty(); }

 private:
  /// constant throughout lifetime
  Type type_;

  /// variable throughout lifetime. communicate changes using `on_layout_dirty`.
  // for view widgets, this is effectively the size that's actually visible.
  SelfExtent self_extent_;

  /// variable throughout lifetime. communicate changes using `on_layout_dirty`
  Flex flex_;

  /// variable throughout lifetime. communicate changes using
  /// `on_children_changed`
  stx::Span<Widget *const> children_;

  /// variable throughout lifetime
  DebugInfo debug_info_;

  /// constant throughout lifetime
  stx::Option<ZIndex> z_index_;

  /// for view widgets (used for laying out its children).
  ///
  /// variable throughout lifetime.
  /// resolved using the parent allotted extent.
  /// TODO(lamarrr): how is this resolved?
  ViewExtent view_extent_;

  /// for view widgets (used for scrolling or moving of the view)
  ///
  /// variable throughout lifetime. communicate changes with
  /// `on_view_offset_changed`.
  /// resolved using the view extent.
  ViewOffset view_offset_;

  // TODO(lamarrr): padding left, right, top, bottom???
  Padding padding_;

  /// constant throughout lifetime, but modified and used for communication and
  /// managing updates in the system
  StateProxy state_proxy_;
};

constexpr auto h = sizeof(Widget);
constexpr auto c = sizeof(SelfExtent);
constexpr auto d = sizeof(ViewOffset);
constexpr auto v = sizeof(OutputClamp);
constexpr auto e = sizeof(Widget::DebugInfo);

inline stx::FixedReport operator>>(stx::ReportQuery, Widget const &widget) {
  Widget::DebugInfo const debug_info = widget.get_debug_info();
  std::string message = "Widget: ";
  message += debug_info.name;
  message += " (type hint: ";
  message += debug_info.type_hint;
  message += ", info: ";
  message += debug_info.info;
  message += ", address: " + std::to_string((uintptr_t)&widget) + ")";
  return stx::FixedReport(message);
}

}  // namespace ui
}  // namespace vlk
