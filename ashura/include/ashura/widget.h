#pragma once
#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <utility>

#include "ashura/canvas.h"
#include "ashura/context.h"
#include "ashura/event.h"
#include "ashura/primitives.h"
#include "ashura/uuid.h"
#include "stx/async.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/span.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace ash
{

enum class Visibility : u8
{
  Visible,
  Hidden
};

// TODO(lamarrr): we need to pass in a zoom level to the rendering widget? so
// that widgets like text can shape their glyphs properly

struct WidgetDebugInfo
{
  std::string_view type;
};

struct DragData
{
  stx::String                      type;
  stx::Unique<stx::Span<u8 const>> data;
};

// TODO(lamarrr): we might need request detach so child widgets can request to
// be removed and remove all callbacks they may have attached or cancel tasks
// they have pending.
// consider: having tokens that de-register themselves once deleted

/// @brief Base widget class. All widget types must inherit from this struct.
/// all methods are already implemented with reasonable defaults.
struct Widget
{
  Widget()
  {}

  virtual ~Widget()
  {}

  /// @brief get child widgets
  /// @param ctx
  /// @return
  virtual stx::Span<Widget *const> get_children(Context &ctx)
  {
    return {};
  }

  /// @brief get debug and logging information
  /// @param ctx
  /// @return
  virtual WidgetDebugInfo get_debug_info(Context &ctx)
  {
    return WidgetDebugInfo{.type = "Widget"};
  }

  // TODO(lamarrr): we need re-calculable offsets so we can shift the parents around without shifting the children
  // this is important for cursors, drag and drop?
  // this might mean we need to totally remove the concept of area. storing transformed area might not be needed?

  /// @brief distributes the size allocated to it to its child widgets.
  /// unlike CSS. has the advantage that children wouldn't need extra attributes for specific kind of placements i.e. relative, absolute, etc.
  /// @param ctx
  /// @param allocated_size the size allocated to this widget
  /// @param[out] children_allocation sizes allocated to the children.
  virtual void allocate_size(Context &ctx, vec2 allocated_size, stx::Span<vec2> children_allocation)
  {
    children_allocation.fill(vec2{0, 0});
  }

  /// @brief fits itself around its children and positions child widgets along/relative to itself (i.e. position {0, 0} means the child will be placed on the top left of the parent)
  /// @param ctx
  /// @param allocated_size the size allocated to this widget. the widget can decide to disregard or fit to this as needed.
  /// @param children_sizes sizes of the child widgets
  /// @param[out] children_positions positions of the children widget on the parent
  /// @return this widget's fitted extent
  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_allocations, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions)
  {
    return vec2{0, 0};
  }

  /// @brief this is used for absolute positioning of the widget
  /// @param ctx
  /// @param allocated_position the allocated absolute position of this widget
  /// @return
  virtual vec2 position(Context &ctx, vec2 allocated_position)
  {
    return allocated_position;
  }

  /// @brief returns the visibility of this widget. an invisible widget will not be drawn nor receive mouse/touch events.
  /// parents can decide the visibility of eacg child
  /// @param ctx
  /// @param allocated_visibility
  /// @param[out] children_allocation visibility assigned to children
  /// @return
  virtual Visibility get_visibility(Context &ctx, Visibility allocated_visibility, stx::Span<Visibility> children_allocation)
  {
    children_allocation.fill(allocated_visibility);
    return allocated_visibility;
  }

  /// @brief returns the z-index of itself and assigns z-indices to its children
  /// @param ctx
  /// @param allocated_z_index
  /// @param[out] children_allocation z-index assigned to children
  /// @return
  virtual i32 z_stack(Context &ctx, i32 allocated_z_index, stx::Span<i32> children_allocation)
  {
    children_allocation.fill(allocated_z_index + 1);
    return allocated_z_index;
  }

  /// @brief this is used for clipping widget views. the provided clip is relative to the root widget's axis (0, 0).
  /// this can be used for nested viewports where there are multiple intersecting clips.
  /// transforms do not apply to the clip rects. this is used for visibility testing and eventually actual vertex culling.
  /// a nested viewport for example can therefore use the intersection of its allocated clip and it's own viewport clip and assign that to its children,
  /// whilst using the allocated clip on itself.
  /// @param ctx
  /// @param allocated_clip
  /// @param[out] children_allocation
  /// @return
  virtual rect clip(Context &ctx, rect allocated_clip, stx::Span<rect> children_allocation)
  {
    children_allocation.fill(allocated_clip);
    return allocated_clip;
  }

  /// @brief record draw commands needed to render this widget. this method is only called if the widget passes the visibility tests.
  /// this is called on every frame.
  /// @param ctx
  /// @param canvas
  ///
  ///
  virtual void draw(Context &ctx, gfx::Canvas &canvas)
  {
    // TODO(lamarrr): the whole widget tree will be rendered and clipped as necessary
  }

  // TODO(lamarrr): draw_tooltip();

  /// @brief called on every frame. used for state changes, animations, task dispatch and lightweight processing related to the GUI.
  /// heavy-weight and non-sub-millisecond tasks should be dispatched to a Subsystem that would handle that. i.e. using the multi-tasking system.
  /// @param ctx
  /// @param interval time passed since last call to this method
  virtual void tick(Context &ctx, std::chrono::nanoseconds interval)
  {}

  /// @brief called on every frame the widget is viewed on the viewport.
  /// @param ctx
  virtual void on_view_hit(Context &ctx)
  {}

  /// @brief called on every frame that the widget is not seen on the viewport
  /// this can be because it has hidden visibility, is clipped away, or parent positioned out of the visible region
  /// @param ctx
  virtual void on_view_miss(Context &ctx)
  {}

  // TODO(lamarrr): this needs to happen before mouse actions as some widgets .i.e. text don't need to intercept or receive mouse events
  virtual bool hit_test(Context &ctx, vec2 mouse_position)
  {
    return false;
  }

  virtual void on_mouse_down(Context &ctx, MouseButton button, vec2 mouse_position, u32 nclicks)
  {}

  virtual void on_mouse_up(Context &ctx, MouseButton button, vec2 mouse_position, u32 nclicks)
  {}

  // TODO(lamarrr): how do we fix translation and zooming? i.e. positioning once transform is applied
  virtual void on_mouse_move(Context &ctx, vec2 mouse_position, vec2 translation)
  {}

  virtual void on_mouse_enter(Context &ctx, vec2 mouse_position)
  {}

  virtual void on_mouse_leave(Context &ctx, stx::Option<vec2> mouse_position)
  {}

  // virtual bool on_mouse_wheel(Context& ctx, vec2 translation, vec2 mouse_position?). propagates up

  /// callback to begin drag operation
  /// if this returns false, it is treated as a click operation.???
  virtual stx::Option<DragData> on_drag_start(Context &ctx, vec2 mouse_position)
  {
    return stx::None;
  }

  /// @brief called when theres a drag position update
  /// @param ctx
  /// @param mouse_position current global drag position
  /// @param translation difference between this drag update and the last drag update position
  virtual void on_drag_update(Context &ctx, vec2 mouse_position, vec2 translation, DragData const &drag_data)
  {}

  /// the drop of the drag data has been ended
  virtual void on_drag_end(Context &ctx, vec2 mouse_position)
  {}

  /// this widget has begun receiving drag data, i.e. it has been dragged onto
  ///
  /// returns true if widget can accept this drag event
  virtual void on_drag_enter(Context &ctx, DragData const &drag_data)
  {
  }

  /// this widget has previously begun receiving drag data, but the mouse is still dragging within it
  virtual void on_drag_over(Context &ctx, DragData const &drag_data)
  {}

  /// the drag event has left this widget
  virtual void on_drag_leave(Context &ctx,stx::Option<vec2> mouse_position)
  {}

  /// drop of drag data on this widget
  virtual bool on_drop(Context &ctx, vec2 mouse_position, DragData const &drag_data)
  {
    return false;
  }

  //
  virtual void on_tap(Context &ctx)
  {}

  //
  virtual void on_touch_cancel(Context &ctx)
  {}

  //
  virtual void on_touch_end(Context &ctx)
  {}

  //
  virtual void on_touch_move(Context &ctx)
  {}

  //
  virtual void on_touch_start(Context &ctx)
  {}

  //
  virtual void on_touch_enter(Context &ctx)
  {}

  //
  virtual void on_touch_leave(Context &ctx)
  {}

  stx::Option<uuid> id;          /// id used to recognise the widget. checked every frame. if one is not present or removed. a new uuid is generated and assigned.
  rect              area;        ///
};

template <typename T>
concept WidgetImpl = std::is_base_of_v<Widget, T>;

inline Widget *__find_widget_recursive(Context &ctx, Widget &widget, uuid id)
{
  if (widget.id.contains(id))
  {
    return &widget;
  }

  for (Widget *child : widget.get_children(ctx))
  {
    Widget *found = __find_widget_recursive(ctx, *child, id);
    if (found != nullptr)
    {
      return found;
    }
  }

  return nullptr;
}

using entity_id            = uint64_t;
using component_id         = uint64_t;
using system_id            = uint64_t;
using component_destructor = void (*)(uint64_t const *free_mask, void *reps, uint64_t count);
using component_batching   = uint64_t;

constexpr component_batching COMPONENT_BATCHED;

struct SampleComponent
{
  static constexpr component_id COMPONENT_ID = 0;
};

// guaranteed to be contigous in memory both component-wise and entity-wise
//
// system can decide to only process batches
// should registry require batches???
//
// they are thus not disjoint in memory
struct entity_batch
{
  // must all share the same components, and freed together
  // data may change differently
  // can share signals?
  entity_id first_entity = 0;
  u64       nentities    = 0;
};

// TODO(lamarrr): mask these signals using a u32 flag
// SIGNALS

// cache preload multiple components into memory
// we can tell the compiler that the component queries will not alias
// delete, update, modify callbacks or events
//
// systems are thus free to re-organize the components
//
//
struct ComponentTable
{
  uint64_t size_bytes = 0;

  uint64_t capacity = 0;

  uint64_t count = 0;

  uint64_t nfree = 0;

  uint64_t nsignals = 0;

  void *reps = nullptr;

  uint64_t nbatches = 0;        // only valid for batch registries

  uint64_t *batch_sizes = nullptr;        // only valid for batch registries

  uint64_t *freed_mask = nullptr;        // if it is a batch registry, it will represent each batch

  // signals to-and-from the systems.
  // it is left to the component and the system to interpret and use these signals.
  // example signals are: component_disabled, component_updated, component_has_request, etc.
  uint64_t **signals = nullptr;

  // default state for the signals
  uint64_t *default_signal_states = nullptr;

  component_destructor destructor = nullptr;
};

// THE GOAL IS FOR FAST ACCESS OF COMPONENTS, NOT OF ENTITIES
// SYSTEMS OPERATE ON COMPONENTS, NOT ENTITIES
// COMPONENTS can only be disabled, not removed
/// encourages memory re-use
struct Registry
{
  // can request that the component type has a tag to it
  template <typename... Components>
  void register_components(component_id (&ids)[sizeof...(Components)]);

  // entity tags???
  entity_id create_entity();
  void      disable_entity(entity_id);
  void      remove_entity(entity_id);
  // entity and components batching

  template <typename Component>
  Component *get_component(entity_id, component_id);

  void *get_component(entity_id, component_id);

  template <typename... Components>
  void add_components(entity_id, component_id (&ids)[sizeof...(Components)], Components &&...);

  template <typename Component>
  void add_component(entity_id, component_id, Component &&);

  template <typename Component>
  void disable_component(entity_id, component_id);

  template <typename Component>
  void remove_component(entity_id, component_id);

  void get_component_state(entity_id, component_id);

  // template <typename Callback>
  // void query(component_id const *components_query, component_id const *components_to_fetch);
  // callback return -> component_id, registry

  uint64_t        components_capacity  = 0;
  uint64_t        entities_capacity    = 0;
  uint64_t        ncomponents          = 0;
  uint64_t        nentities            = 0;
  uint64_t        nfree_entities       = 0;
  ComponentTable *components           = nullptr;
  uint64_t       *entity_component_map = nullptr;        // (entity_id * ncomponents * n_entities + component_id) maps entity to index of its component in a map, uint64_t_MAX if entity does not have a component
  uint64_t       *entity_free_mask     = nullptr;
  // everytime a component type is added we need to extend the component attributes of all entities
};

// say for example we want a system that processes elements in batches
// batches may not be speci

// entity/registry batches are somewhat useless?
// disjoint-entity component batching are useful and what we need as some systems i.e. particle processing systems won't function properly without them
// we can have reserves of components
}        // namespace ash
