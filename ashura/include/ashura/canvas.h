#pragma once

#include <chrono>
#include <cinttypes>
#include <cmath>

#include "ashura/primitives.h"
#include "ashura/vulkan.h"
#include "stx/string.h"
#include "stx/vec.h"
#include "vulkan/vulkan.h"

namespace asr {
using namespace stx::literals;
using namespace std::chrono_literals;

namespace gfx {

// compute area of a contour/polygon
constexpr f32 compute_polygon_area(stx::Span<vec2 const> polygon) {
  usize npoints = polygon.size();

  f32 area = 0.0f;

  if (npoints < 3) return area;

  for (usize p = npoints - 1, q = 0; q < npoints; p = q++) {
    area += cross(polygon[p], polygon[q]);
  }

  return area * 0.5f;
}

constexpr bool polygon_snip_check(stx::Span<vec2 const> polygon, i64 u, i64 v,
                                  i64 w, i64 n, stx::Span<i64 const> V) {
  vec2 a = polygon[V[u]];
  vec2 b = polygon[V[v]];
  vec2 c = polygon[V[w]];

  if (stx::f32_epsilon > cross(b - a, c - a)) return false;

  for (i64 p = 0; p < n; p++) {
    if ((p == u) || (p == v) || (p == w)) continue;
    if (is_inside_triangle(a, b, c, polygon[V[p]])) return false;
  }

  return true;
}

inline stx::Vec<vec2> triangulate_polygon(stx::Span<vec2 const> polygon) {
  stx::Vec<vec2> result{stx::os_allocator};

  i64 npoints = polygon.size();

  ASR_ENSURE(npoints >= 3, "polygon must have 3 or more points");

  stx::Vec<i64> V{stx::os_allocator};

  V.resize(npoints, 0).unwrap();

  // we want a counter-clockwise polygon in V
  if (0.0f < compute_polygon_area(polygon)) {
    for (i64 v = 0; v < npoints; v++) {
      V[v] = v;
    }
  } else {
    for (i64 v = 0; v < npoints; v++) {
      V[v] = (npoints - 1) - v;
    }
  }

  i64 nv = npoints;

  //  remove nv-2 vertices, creating 1 triangle every time
  i64 count = 2 * nv;  // error detection

  for (i64 m = 0, v = nv - 1; nv > 2;) {
    // if we loop, it is probably a non-simple polygon
    if (0 >= (count--)) {
      ASR_PANIC("Polygon triangulation error: probable bad polygon!");
    }

    // three consecutive vertices in current polygon, <u, v, w>
    i64 u = v;

    if (nv <= u) u = 0;  //  previous

    v = u + 1;

    if (nv <= v) v = 0;  // new v

    i64 w = v + 1;

    if (nv <= w) w = 0;  // next

    if (polygon_snip_check(polygon, u, v, w, nv, V)) {
      // true names of the vertices
      i64 a = V[u];
      i64 b = V[v];
      i64 c = V[w];

      // output triangle
      result.push_inplace(polygon[a]).unwrap();
      result.push_inplace(polygon[b]).unwrap();
      result.push_inplace(polygon[c]).unwrap();

      m++;

      // remove v from remaining polygon
      for (i64 s = v, t = v + 1; t < nv; s++, t++) {
        V[s] = V[t];
      }

      nv--;

      // reset error detection counter
      count = 2 * nv;
    }
  }

  return result;
}

struct TextMetrics {
  // x-direction
  f32 width = 0.0f;
  f32 actual_bounding_box_left = 0.0f;
  f32 actual_bounding_box_right = 0.0f;

  // y-direction
  f32 font_bounding_box_ascent = 0.0f;
  f32 font_bounding_box_descent = 0.0f;
  f32 actual_bounding_box_ascent = 0.0f;
  f32 actual_bounding_box_descent = 0.0f;
  f32 ascent = 0.0f;
  f32 descent = 0.0f;
  f32 hanging_baseline = 0.0f;
  f32 alphabetic_baseline = 0.0f;
  f32 ideographic_baseline = 0.0f;
};

struct Shadow {
  f32 offset_x = 0.0f;
  f32 offset_y = 0.0f;
  f32 blur_radius = 0.0f;
  Color color = colors::BLACK;
};

struct Filter {
  // None by default
};

enum class TextAlign : u8 {
  // detect locale and other crap
  Start,
  End,
  Left,
  Right,
  Center
};

enum class TextBaseline : u8 {
  Top,
  Hanging,
  Middle,
  Alphabetic,
  Ideographic,
  Bottom
};

enum class TextDirection : u8 { Ltr, Rtl, Ttb, Btt };

enum class FontKerning : u8 { Normal, None };

enum class FontStretch : u8 {
  UltraCondensed,
  ExtraCondensed,
  Condensed,
  SemiCondensed,
  Normal,
  SemiExpanded,
  Expanded,
  ExtraExpanded,
  UltraExpanded
};

enum class FontWeight : u32 {
  Thin = 100,
  ExtraLight = 200,
  Light = 300,
  Normal = 400,
  Medium = 500,
  Semi = 600,
  Bold = 700,
  ExtraBold = 800,
  Black = 900,
  ExtraBlack = 950
};

// requirements:
// - easy resource specification
// - caching so we don't have to reupload on every frame
// - GPU texture/image
// NOTE: canvas is low-level so we don't use paths, urls, or uris

// requirements:
// -
struct TypefaceId {
  u64 id = 0;
};

// stored in vulkan context
using Image = stx::Rc<vk::ImageSampler*>;
struct Typeface;

struct TypefaceRetainer;

// TODO(lamarrr): embed font into a cpp file
//
// on font loading
//
struct TextStyle {
  stx::String font_family = "SF Pro"_str;
  FontWeight font_weight = FontWeight::Normal;
  u32 font_size = 10;
  TextAlign align = TextAlign::Start;
  TextBaseline baseline = TextBaseline::Alphabetic;
  TextDirection direction = TextDirection::Ltr;
  u32 letter_spacing = 0;
  FontKerning font_kerning = FontKerning::None;
  FontStretch font_stretch = FontStretch::Normal;
  u32 word_spacing = 0;
};

struct Brush {
  bool fill = true;
  Color color = colors::BLACK;
  f32 opacity = 1.0f;
  f32 line_width = 1.0f;
  stx::Option<Image> pattern;
  TextStyle text_style;
  Filter filter;
  Shadow shadow;
};

// TODO(lamarrr): invert these rows and columns
namespace transforms {

inline mat4x4 translate(vec3 t) {
  return mat4x4{
      vec4{1.0f, 0.0f, 0.0f, t.x},
      vec4{0.0f, 1.0f, 0.0f, t.y},
      vec4{0.0f, 0.0f, 1.0f, t.z},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

inline mat4x4 scale(vec3 s) {
  return mat4x4{
      vec4{s.x, 0.0f, 0.0f, 0.0f},
      vec4{0.0f, s.y, 0.0f, 0.0f},
      vec4{0.0f, 0.0f, s.z, 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

inline mat4x4 rotate_x(f32 degree) {
  return mat4x4{
      vec4{1.0f, 0.0f, 0.0f, 0.0f},
      vec4{0.0f, std::cos(degree), -std::sin(degree), 0.0f},
      vec4{0.0f, std::sin(degree), std::cos(degree), 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

inline mat4x4 rotate_y(f32 degree) {
  return mat4x4{
      vec4{std::cos(degree), 0.0f, std::sin(degree), 0.0f},
      vec4{0.0f, 1.0f, 0.0f, 0.0f},
      vec4{-std::sin(degree), 0, std::cos(degree), 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

inline mat4x4 rotate_z(f32 degree) {
  return mat4x4{
      vec4{std::cos(degree), -std::sin(degree), 0.0f, 0.0f},
      vec4{std::sin(degree), std::cos(degree), 0.0f, 0.0f},
      vec4{0.0f, 0.0f, 1.0f, 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

};  // namespace transforms

// TODO(lamarrr): examine placement and transform
// TODO(lamarrr): placement and transform for clip
struct DrawCommand {
  u32 indices_offset = 0;
  u32 ntriangles = 0;
  u32 clip_indices_offset = 0;
  u32 nclip_triangles = 0;
  // defines size, rotation, and position of the object
  mat4x4 placement = mat4x4::identity();
  // transform contains position (translation from origin)
  mat4x4 transform = mat4x4::identity();
  // color to use for the output
  Color color = colors::BLACK;
  // texture to use for output
  Image texture;
};

struct DrawList {
  stx::Vec<vec3> vertices{stx::os_allocator};
  stx::Vec<u32> indices{stx::os_allocator};
  stx::Vec<vec2> clip_vertices{stx::os_allocator};
  stx::Vec<u32> clip_indices{stx::os_allocator};
  stx::Vec<DrawCommand> commands{stx::os_allocator};
};

// TODO(lamarrr): how do we handle selection of transformed widgets?
// Topleft origin coordinate system
//
//
// TODO(lamarrr): we need to implement clipping via clipping bitmaps, create
// framebuffer with only an alpha attachment, this framebuffer will be the size
// of the window and will be reused for every drawing operation so we don't have
// to recreate or resize the bitmap everytime
//
//
// TODO(lamarrr): implement clipping
//
//
// TODO(lamarrr): how will color and image blending work, image and color
// srcover or dstover, color over image or image over color
//
//
// TODO(lamarrr): is there a way we can not require specifying the coordinates
// in unit?
//
//
// TODO(lamarrr): add clipping, atleast clip_rounded_rect
// use clipping masks, just a simple convex polygon
//
//
//
// TODO(lamarrr): separate shape generating functions
//
struct Canvas {
  vec2 extent;
  Brush brush;

  mat4x4 transform = mat4x4::identity();
  stx::Vec<mat4x4> transform_state_stack{stx::os_allocator};
  // clip with size and

  stx::Vec<vec2> clip_area{stx::os_allocator};
  stx::Vec<stx::Vec<vec2>> clip_area_state_stack{stx::os_allocator};

  // clip should be rect by default
  // can only have one clip polygon

  DrawList draw_list;

  // push state (transform and clips) on state stack
  Canvas& save() {
    transform_state_stack.push_inplace(transform).unwrap();
    return *this;
  }

  // save current transform and clip state
  // pop state (transform and clips) stack and restore state
  Canvas& restore() {
    ASR_ENSURE(!transform_state_stack.is_empty());
    transform = *(transform_state_stack.end() - 1);
    transform_state_stack.resize(transform_state_stack.size() - 1).unwrap();

    return *this;
  }

  // reset the rendering context to its default state (transform
  // and clips)
  Canvas& reset() {
    transform = mat4x4::identity();
    return *this;
  }

  Canvas& translate(f32 x, f32 y, f32 z) {
    transform = transforms::translate(vec3{x, y, z}) * transform;
    return *this;
  }

  Canvas& translate(f32 x, f32 y) { return translate(x, y, 0.0f); }

  Canvas& rotate(f32 degree) {
    transform = transforms::rotate_z(degree) * transform;
    return *this;
  }

  Canvas& rotate(f32 x, f32 y, f32 z) {
    transform = transforms::rotate_z(z) * transforms::rotate_y(y) *
                transforms::rotate_x(x) * transform;
    return *this;
  }

  Canvas& scale(f32 x, f32 y, f32 z) {
    transform = transforms::scale(vec3{x, y, z}) * transform;
    return *this;
  }

  Canvas& scale(f32 x, f32 y) { return scale(x, y, 1.0f); }

  Canvas& clear() {
    u32 start = AS_U32(draw_list.vertices.size());

    vec3 vertices[] = {{0.0f, 0.0f, 0.0f},
                       {1.0f, 0.0f, 0.0f},
                       {1.0f, 1.0f, 0.0f},
                       {0.0f, 1.0f, 0.0f}};

    draw_list.vertices.extend(vertices).unwrap();

    u32 indices[] = {start, start + 1, start + 2, start + 2, start, start + 3};

    draw_list.indices.extend(indices).unwrap();

    // draw_list.commands
    //     .push(DrawCommand{
    //         .color = brush.color,
    //         .frag_shader,
    //         .indices_offset = start,
    //         .ntriangles = 2,
    //         .opacity,
    //         .texture,
    //         .transform = transforms::scale(vec3{extent.x, extent.y, 1.0f}),
    //         .vert_shader})
    //     .unwrap();

    return *this;
  }

  Canvas& clip_rect();
  Canvas& clip_round_rect();
  Canvas& clip_slanted_rect();
  Canvas& clip_circle();
  Canvas& clip_ellipse();

  // vertices are expected to be specified in unit dimension. i.e. ranging
  // from 0.0f to 1.0f
  Canvas& draw_polygon_line(stx::Span<vec2 const> line, mat4x4 placement) {
    ASR_ENSURE(line.size() >= 2);

    for (usize i = 0; i < line.size(); i++) {
      usize j = (i == line.size()) ? 0 : (i + 1);

      vec2 p1 = line[i];
      vec2 p2 = line[j];

      vec2 d = p2 - p1;

      {
        float dot_product = dot(d, d);
        if (dot_product > 0.0f) {
          float inverse_length = 1 / std::sqrt(dot_product);
          d.x *= inverse_length;
          d.y *= inverse_length;
        }
      }

      d.x *= brush.line_width * 0.5f;
      d.y *= brush.line_width * 0.5f;

      u32 start = AS_U32(draw_list.indices.size());

      vec3 vertices[] = {{p1.x + d.y, p1.y - d.x, 0.0f},
                         {p2.x + d.y, p2.y - d.x, 0.0f},
                         {p2.x - d.y, p2.y + d.x, 0.0f},
                         {p1.x - d.y, p1.y + d.x, 0.0f}};

      u32 indices[] = {start,     start + 1, start + 2,
                       start + 3, start + 4, start + 5};

      draw_list.vertices.extend(vertices).unwrap();
      draw_list.indices.extend(indices).unwrap();

      //   draw_list.commands
      //       .push(DrawCommand{.color,
      //                         .frag_shader,
      //                         .indices_offset = start,
      //                         .ntriangles = 2,
      //                         .opacity,
      //                         .placement,
      //                         .texture,
      //                         .transform,
      //                         .vert_shader})
      //       .unwrap();
    }

    return *this;
  }

  // vertices are expected to be specified in unit dimension. i.e. ranging from
  // 0.0f to 1.0f
  Canvas& draw_polygon_filled(stx::Span<vec2 const> polygon, mat4x4 placement) {
    ASR_ENSURE(polygon.size() >= 3);

    stx::Vec<vec2> polygon_vertices = triangulate_polygon(polygon);

    for (vec2 vertex : polygon_vertices) {
      draw_list.vertices.push(vec3{vertex.x, vertex.y, 0.0f}).unwrap();
    }

    u32 start = AS_U32(draw_list.indices.size());

    for (u32 index = start; index < (start + polygon_vertices.size());
         index++) {
      draw_list.indices.push_inplace(index).unwrap();
    }

    u32 ntriangles = AS_U32(polygon_vertices.size()) / 3U;

    // draw_list.commands
    //     .push(DrawCommand{.color,
    //                       .frag_shader,
    //                       .indices_offset = start,
    //                       .ntriangles = ntriangles,
    //                       .opacity,
    //                       .placement,
    //                       .texture,
    //                       .transform,
    //                       .vert_shader})
    //     .unwrap();

    return *this;
  }

  Canvas& draw_line(vec2 p1, vec2 p2) {
    f32 line_width = brush.line_width;

    mat4x4 placement;

    vec3 vertices[] = {{0.0f, 0.0f, 0.0f},
                       {1.0f, 0.0f, 0.0f},
                       {1.0f, 1.0f, 0.0f},
                       {0.0f, 1.0f, 0.0f}};

    draw_list.vertices.extend(vertices).unwrap();

    u32 start = AS_U32(draw_list.indices.size());

    u32 indices[] = {start, start + 1, start + 2, start + 2, start, start + 3};

    draw_list.indices.extend(indices).unwrap();

    // draw_list.commands.push(
    //     DrawCommand{.color,
    //                 .frag_shader,
    //                 .indices_offset = start,
    //                 .ntriangles = std::size(indices) / 3U,
    //                 .opacity,
    //                 .placement,
    //                 .texture,
    //                 .transform,
    //                 .vert_shader});

    return *this;
  }

  Canvas& draw_rect(f32 x, f32 y, f32 width, f32 height) {
    // TODO(lamarrr): what about textured backgrounds?

    // save();
    // translate(x, y);
    // scale(width, height);
    // TODO(lamarrr): we need a separate shape translate and scale as we want to
    // specify in unit dimensions? otherwise we need to pass in width and height
    // to shaders, which still won't work since we need to use some for sampling
    // from textures

    vec2 points[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

    mat4x4 placement = transforms::translate({x, y, 0.0f}) *
                       transforms::scale({width, height, 1.0f});

    if (brush.fill) {
      draw_polygon_filled(points, placement);
    } else {
      draw_polygon_line(points, placement);
    }

    // restore();

    return *this;
  }

  // angle = 0.0f to 90.0f for top left, angle
  // = 90.0f to 180.0f for top right,
  // angle = 180.0f to 270.0f for bottom right,
  // angle = 270.0f to 360.0f for bottom left,
  // nsegments
  //
  //
  Canvas& draw_round_rect(vec2 offset, vec2 extent, vec4 radii,
                          usize nsegments);
  Canvas& draw_slanted_rect(vec2 offset, vec2 extent);

  // within circle and within a rect that contains
  // that circle (for filled arc)
  Canvas& draw_circle(vec2 center, f32 radius, usize nsegments) {
    f32 delta = 360.0f / nsegments;
    // from angle = 0.0f to 360.0f with
    // TODO(lamarrr): what if segment count is 1, 0
    // needs clamping as well? cos and sin are already clamped
    for (i64 i = 0; i < AS_I64(nsegments); i++) {
      f32 angle = delta + i * delta;
      vec2 point{std::cos(angle), std::sin(angle)};
    }
    return *this;
  }

  Canvas& draw_ellipse(vec2 center, vec2 radius, f32 rotation, f32 start_angle,
                       f32 end_angle);

  // Text API
  Canvas& draw_text(stx::StringView text, vec2 position);

  // Image API
  Canvas& draw_image(Image const& image, vec2 position);
  Canvas& draw_image(Image const& image, vec2 position, vec2 extent);
  Canvas& draw_image(Image const& image, vec2 portion_offset, vec2 portion_size,
                     vec2 target_offset, vec2 target_extent);
};

void sample(Canvas& canvas) {
  Image* image;

  canvas.save()
      .rotate(45)
      .draw_circle({0, 0}, 20.0f)
      .draw_image(*image, {{0.0, 0.0}, {20, 40}})
      .restore()
      .scale(2.0f, 2.0f)
      .draw_line({0, 0}, {200, 200})
      .draw_text("Hello World, こんにちは世界", {10.0f, 10.0f})
      .draw_rect(0, 0, 20, 20)
      .draw_round_rect({{0.0f, 0.0f}, {20.0f, 20.0f}},
                       {10.0f, 10.0f, 10.0f, 10.0f});
}

struct Transform {
  mat4x4 value = mat4x4::identity();
};

struct Overlay {
  vec4 color{0.0f, 0.0f, 0.0f, 0.0f};
};

struct CanvasContext {
  VkBuffer transform_buffer = VK_NULL_HANDLE;
  VkDeviceMemory transform_memory = VK_NULL_HANDLE;
  void* transform_memory_map = nullptr;

  VkBuffer clip_transform_buffer = VK_NULL_HANDLE;
  VkDeviceMemory clip_transform_memory = VK_NULL_HANDLE;
  void* clip_transform_memory_map = nullptr;

  VkBuffer overlay_buffer = VK_NULL_HANDLE;
  VkDeviceMemory overlay_memory = VK_NULL_HANDLE;
  void* overlay_memory_map = nullptr;

  stx::Rc<vk::CommandQueue*> queue;

  CanvasContext(stx::Rc<vk::CommandQueue*> aqueue) : queue{std::move(aqueue)} {
    VkDevice dev = queue.handle->device.handle->device;

    {
      auto [buffer, memory] = vk::create_buffer_with_memory(
          dev, queue.handle->info.family,
          queue.handle->device.handle->phy_device.handle->memory_properties,
          sizeof(Transform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

      transform_memory = memory;
      transform_buffer = buffer;

      ASR_VK_CHECK(vkMapMemory(dev, transform_memory, 0, VK_WHOLE_SIZE, 0,
                               &transform_memory_map));
    }

    {
      auto [buffer, memory] = vk::create_buffer_with_memory(
          dev, queue.handle->info.family,
          queue.handle->device.handle->phy_device.handle->memory_properties,
          sizeof(Transform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

      clip_transform_buffer = buffer;
      clip_transform_memory = memory;

      ASR_VK_CHECK(vkMapMemory(dev, clip_transform_memory, 0, VK_WHOLE_SIZE, 0,
                               &clip_transform_memory_map));
    }

    {
      auto [buffer, memory] = vk::create_buffer_with_memory(
          dev, queue.handle->info.family,
          queue.handle->device.handle->phy_device.handle->memory_properties,
          sizeof(Overlay), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

      overlay_buffer = buffer;
      overlay_memory = memory;

      ASR_VK_CHECK(
          vkMapMemory(dev, memory, 0, VK_WHOLE_SIZE, 0, &overlay_memory_map));
    }
  }

  STX_MAKE_PINNED(CanvasContext)

  ~CanvasContext() {
    VkDevice dev = queue.handle->device.handle->device;

    vkUnmapMemory(dev, transform_memory);
    vkUnmapMemory(dev, clip_transform_memory);
    vkUnmapMemory(dev, overlay_memory);

    vkFreeMemory(dev, transform_memory, nullptr);
    vkFreeMemory(dev, clip_transform_memory, nullptr);
    vkFreeMemory(dev, overlay_memory, nullptr);

    vkDestroyBuffer(dev, transform_buffer, nullptr);
    vkDestroyBuffer(dev, clip_transform_buffer, nullptr);
    vkDestroyBuffer(dev, overlay_buffer, nullptr);
  }

  void write_transform(Transform const& transform) {
    std::memcpy(transform_memory_map, &transform, sizeof(Transform));

    VkDevice dev = queue.handle->device.handle->device;

    VkMappedMemoryRange range{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = nullptr,
        .memory = transform_memory,
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };

    ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));
  }

  void write_clip_transform(Transform const& transform) {
    std::memcpy(clip_transform_memory_map, &transform, sizeof(Transform));

    VkDevice dev = queue.handle->device.handle->device;

    VkMappedMemoryRange range{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = nullptr,
        .memory = clip_transform_memory,
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };

    ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));
  }

  void write_overlay(Overlay const& overlay) {
    std::memcpy(overlay_memory_map, &overlay, sizeof(Overlay));

    VkDevice dev = queue.handle->device.handle->device;

    VkMappedMemoryRange range{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = nullptr,
        .memory = overlay_memory,
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };

    ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));
  }
};

// TODO(lamarrr): add to list of device/operations currently using resources so
// resource won't be freed when in use
//
//
// TODO(lamarrr): how to ensure resources are not destroyed whilst in use
//
//
// TODO(lamarrr): clear draw list after render call
//
inline void render(vk::RecordingContext& ctx, CanvasContext& canvas_ctx,
                   DrawList const& draw_list) {
  static constexpr u64 TIMEOUT = AS_U64(
      std::chrono::duration_cast<std::chrono::nanoseconds>(1min).count());

  vk::SwapChain const& swapchain =
      *ctx.surface.handle->swapchain.value().handle;

  stx::Rc<vk::Device*> const& device = swapchain.queue.handle->device;

  VkPhysicalDeviceMemoryProperties const& memory_properties =
      device.handle->phy_device.handle->memory_properties;

  vk::CommandQueueFamilyInfo const& family =
      swapchain.queue.handle->info.family;

  VkDevice dev = device.handle->device;

  stx::Rc<vk::Buffer*> vertex_buffer =
      upload_vertices(device, family, memory_properties,
                      stx::Span<vec3 const>{draw_list.vertices});

  stx::Rc<vk::Buffer*> index_buffer =
      upload_indices(device, family, memory_properties, draw_list.indices);

  stx::Rc<vk::Buffer*> clip_vertex_buffer =
      upload_vertices(device, family, memory_properties,
                      stx::Span<vec2 const>{draw_list.clip_vertices});

  stx::Rc<vk::Buffer*> clip_index_buffer =
      upload_indices(device, family, memory_properties, draw_list.clip_indices);

  ASR_VK_CHECK(vkResetCommandBuffer(ctx.command_buffer, 0));

  for (DrawCommand const& draw_command : draw_list.commands) {
    VkCommandBufferBeginInfo command_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    ASR_VK_CHECK(
        vkBeginCommandBuffer(ctx.command_buffer, &command_buffer_begin_info));

    //
    // render clip if any
    // if none, fill the whole buffer with the color (2 triangles, 1 color),
    // done in canvas
    //
    //
    // TODO(lamarrr): sampler will not be able to pick up the correct
    // coordinates for the texture
    //
    //
    // TODO(lamarrr): does the descriptor set copy the data on write
    //
    // this also means we might need to have different buffers and memory per
    // flight frame
    //
    //
    // TODO(lamarrr): clip shape orientation and sizing to fit
    {
      Transform clip_transform{draw_command.transform * draw_command.placement};

      canvas_ctx.write_clip_transform(clip_transform);

      vk::DescriptorBinding set0[] = {
          vk::DescriptorBinding::make_buffer(canvas_ctx.clip_transform_buffer)};

      stx::Span<vk::DescriptorBinding const> sets[] = {set0};

      ctx.clip_descriptor_sets.write(sets);

      ASR_VK_CHECK(vkResetCommandBuffer(ctx.clip_command_buffer, 0));

      VkCommandBufferBeginInfo command_buffer_begin_info{
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
          .pNext = nullptr,
          .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
          .pInheritanceInfo = nullptr,
      };

      ASR_VK_CHECK(vkBeginCommandBuffer(ctx.clip_command_buffer,
                                        &command_buffer_begin_info));

      VkClearValue clear_values[] = {
          {.color = VkClearColorValue{{0.0f, 0.0f, 0.0f, 0.0f}}}};

      // TODO(lamarrr): set render area to clip area
      VkRenderPassBeginInfo render_pass_begin_info{
          .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
          .pNext = nullptr,
          .renderPass = swapchain.clip.render_pass,
          .framebuffer = swapchain.clip.framebuffer,
          .renderArea = VkRect2D{.offset = {0, 0}, .extent = swapchain.extent},
          .clearValueCount = AS_U32(std::size(clear_values)),
          .pClearValues = clear_values};

      vkCmdBeginRenderPass(ctx.clip_command_buffer, &render_pass_begin_info,
                           VK_SUBPASS_CONTENTS_INLINE);

      // TODO(lamarrr): set viewport and scissor to clip area
      VkRect2D scissor{.offset = {0, 0}, .extent = swapchain.extent};

      vkCmdSetScissor(ctx.clip_command_buffer, 0, 1, &scissor);

      VkViewport viewport{.x = 0.0f,
                          .y = 0.0f,
                          .width = AS_F32(swapchain.extent.width),
                          .height = AS_F32(swapchain.extent.height),
                          .minDepth = 0.0f,
                          .maxDepth = 1.0f};

      vkCmdSetViewport(ctx.clip_command_buffer, 0, 1, &viewport);

      vkCmdBindVertexBuffers(ctx.clip_command_buffer, 0, 1,
                             &clip_vertex_buffer.handle->buffer, 0);

      vkCmdBindIndexBuffer(ctx.clip_command_buffer,
                           clip_index_buffer.handle->buffer, 0,
                           VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(
          ctx.clip_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
          ctx.clip_pipeline.layout, 0,
          AS_U32(ctx.clip_descriptor_sets.descriptor_sets.size()),
          ctx.clip_descriptor_sets.descriptor_sets.data(), 0, nullptr);

      vkCmdBindPipeline(ctx.clip_command_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        ctx.clip_pipeline.pipeline);

      vkCmdDrawIndexed(ctx.clip_command_buffer,
                       draw_command.nclip_triangles * 3, 1, 0, 0, 0);

      vkCmdEndRenderPass(ctx.clip_command_buffer);

      ASR_VK_CHECK(vkEndCommandBuffer(ctx.clip_command_buffer));

      VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                               .pNext = nullptr,
                               .waitSemaphoreCount = 0,
                               .pWaitSemaphores = nullptr,
                               .pWaitDstStageMask = nullptr,
                               .commandBufferCount = 0,
                               .pCommandBuffers = nullptr,
                               .signalSemaphoreCount = 0,
                               .pSignalSemaphores = nullptr};

      ASR_VK_CHECK(vkQueueSubmit(ctx.queue.handle->info.queue, 1, &submit_info,
                                 swapchain.clip.fence));
    }

    Transform transform{draw_command.transform * draw_command.placement};

    Overlay overlay{.color = {draw_command.color.r / 255.0f,
                              draw_command.color.g / 255.0f,
                              draw_command.color.b / 255.0f,
                              draw_command.color.a / 255.0f}};

    canvas_ctx.write_transform(transform);
    canvas_ctx.write_overlay(overlay);

    vk::DescriptorBinding set0[] = {
        vk::DescriptorBinding::make_buffer(canvas_ctx.transform_buffer),
        vk::DescriptorBinding::make_buffer(canvas_ctx.overlay_buffer)};

    vk::DescriptorBinding set1[] = {
        vk::DescriptorBinding::make_sampler(
            draw_command.texture.handle->image.handle->view,
            draw_command.texture.handle->sampler),
        vk::DescriptorBinding::make_sampler(swapchain.clip.view,
                                            swapchain.clip.sampler)};

    stx::Span<vk::DescriptorBinding const> sets[] = {set0, set1};

    ctx.descriptor_sets[swapchain.next_frame_flight_index].write(sets);

    VkClearValue clear_values[] = {
        {.color = VkClearColorValue{{0.0f, 0.0f, 0.0f, 0.0f}}},
        {.depthStencil =
             VkClearDepthStencilValue{.depth = 1.0f, .stencil = 0}}};

    VkRenderPassBeginInfo render_pass_begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = swapchain.render_pass,
        .framebuffer =
            swapchain.frame_buffers[swapchain.next_frame_flight_index],
        .renderArea = VkRect2D{.offset = {0, 0}, .extent = swapchain.extent},
        .clearValueCount = AS_U32(std::size(clear_values)),
        .pClearValues = clear_values};

    vkCmdBeginRenderPass(ctx.command_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    VkRect2D scissor{.offset = {0, 0}, .extent = swapchain.extent};

    vkCmdSetScissor(ctx.command_buffer, 0, 1, &scissor);

    VkViewport viewport{.x = 0.0f,
                        .y = 0.0f,
                        .width = AS_F32(swapchain.extent.width),
                        .height = AS_F32(swapchain.extent.height),
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f};

    vkCmdSetViewport(ctx.command_buffer, 0, 1, &viewport);

    vkCmdBindVertexBuffers(ctx.command_buffer, 0, 1,
                           &vertex_buffer.handle->buffer, 0);

    vkCmdBindIndexBuffer(ctx.command_buffer, index_buffer.handle->buffer, 0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(
        ctx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        ctx.pipeline.layout, 0,
        AS_U32(ctx.descriptor_sets[ctx.next_swapchain_image_index]
                   .descriptor_sets.size()),
        ctx.descriptor_sets[swapchain.next_frame_flight_index]
            .descriptor_sets.data(),
        0, nullptr);

    vkCmdBindPipeline(ctx.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      ctx.pipeline.pipeline);

    vkCmdDrawIndexed(ctx.command_buffer, draw_command.ntriangles * 3, 1, 0, 0,
                     0);

    vkCmdEndRenderPass(ctx.command_buffer);

    VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                             .pNext = nullptr,
                             .waitSemaphoreCount = 0,
                             .pWaitSemaphores = nullptr,
                             .pWaitDstStageMask = nullptr,
                             .commandBufferCount = 0,
                             .pCommandBuffers = nullptr,
                             .signalSemaphoreCount = 0,
                             .pSignalSemaphores = nullptr};

    ASR_VK_CHECK(vkEndCommandBuffer(ctx.command_buffer));

    ASR_VK_CHECK(
        vkWaitForFences(dev, 1, &swapchain.clip.fence, VK_TRUE, TIMEOUT));

    ASR_VK_CHECK(vkResetFences(dev, 1, &swapchain.clip.fence));

    ASR_VK_CHECK(vkQueueSubmit(
        ctx.queue.handle->info.queue, 1, &submit_info,
        swapchain.rendering_fences[swapchain.next_frame_flight_index]));

    ASR_VK_CHECK(vkWaitForFences(
        dev, 1, &swapchain.rendering_fences[swapchain.next_frame_flight_index],
        VK_TRUE, TIMEOUT));

    ASR_VK_CHECK(vkResetFences(
        dev, 1,
        &swapchain.rendering_fences[swapchain.next_frame_flight_index]));
  }
}

}  // namespace gfx
};  // namespace asr