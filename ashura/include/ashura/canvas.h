#pragma once

#include <cinttypes>
#include <cmath>
#include <filesystem>
#include <fstream>

#include "ashura/primitives.h"
#include "ashura/vulkan.h"
#include "stx/string.h"
#include "stx/vec.h"
#include "vulkan/vulkan.h"

// TODO(lamarrr): we'll acrtually generate 3D vertices with them so they can
// play well with 3D graphics and animations

namespace asr {
namespace gfx {

// TODO(lamarrr): child must inherit parent's transformation and opacity

// Sutherland-Hodgman clipping
inline stx::Vec<vec2> clip_polygon(stx::Span<vec2 const> subject_polygon,
                                   stx::Span<vec2 const> clip_polygon) {
  ASR_ENSURE(!clip_polygon.is_empty());
  ASR_ENSURE(!subject_polygon.is_empty());

  stx::Vec<vec2> ring{stx::os_allocator};

  ring.extend(subject_polygon).unwrap();

  vec2 p1 = clip_polygon[clip_polygon.size() - 1];

  stx::Vec<vec2> input{stx::os_allocator};

  for (vec2 p2 : clip_polygon) {
    input.clear();
    input.extend(ring).unwrap();
    vec2 s = input[input.size() - 1];

    ring.clear();

    for (vec2 e : input) {
      if (is_inside(p1, p2, e)) {
        if (!is_inside(p1, p2, s)) {
          ring.push(intersection(p1, p2, s, e)).unwrap();
        }

        ring.push_inplace(e).unwrap();
      } else if (is_inside(p1, p2, s)) {
        ring.push(intersection(p1, p2, s, e)).unwrap();
      }

      s = e;
    }

    p1 = p2;
  }

  return std::move(ring);
}

// compute area of a contour/polygon
constexpr f32 compute_polygon_area(stx::Span<vec2 const> polygon) {
  usize num_points = polygon.size();

  f32 area = 0.0f;

  for (usize p = num_points - 1, q = 0; q < num_points; p = q++) {
    area += cross(polygon[p], polygon[q]);
  }

  return area * 0.5f;
}

constexpr bool is_valid_polygon(stx::Span<vec2 const> polygon) {
  if (polygon.size() < 3) return false;

  // check that points do not overlap
  vec2 x = polygon[0];

  for (vec2 y : polygon.slice(1)) {
    if (x == y) return false;
    x = y;
  }

  return true;
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

  i64 num_points = polygon.size();

  ASR_ENSURE(num_points >= 3, "polygon must have 3 or more points");

  stx::Vec<i64> V{stx::os_allocator};

  V.resize(num_points, 0).unwrap();

  // we want a counter-clockwise polygon in V
  if (0.0f < compute_polygon_area(polygon)) {
    for (i64 v = 0; v < num_points; v++) {
      V[v] = v;
    }
  } else {
    for (i64 v = 0; v < num_points; v++) {
      V[v] = (num_points - 1) - v;
    }
  }

  i64 nv = num_points;

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

  return std::move(result);
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
struct Image {
  u64 id = 0;
};

// requirements:
// -
struct Typeface {
  u64 id = 0;
};

// requirements:
struct Shader {
  u64 id = 0;
};

// TODO(lamarrr): embed font into a cpp file
//
// on font loading
//
struct TextStyle {
  stx::String font_family = stx::string::make_static("SF Pro");
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
constexpr mat4x4 translate(vec3 t) {
  return mat4x4{
      vec4{1.0f, 0.0f, 0.0f, t.x},
      vec4{0.0f, 1.0f, 0.0f, t.y},
      vec4{0.0f, 0.0f, 1.0f, t.z},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

constexpr mat4x4 scale(vec3 s) {
  return mat4x4{
      vec4{s.x, 0.0f, 0.0f, 0.0f},
      vec4{0.0f, s.y, 0.0f, 0.0f},
      vec4{0.0f, 0.0f, s.z, 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

constexpr mat4x4 rotate_x(f32 degree) {
  return mat4x4{
      vec4{1.0f, 0.0f, 0.0f, 0.0f},
      vec4{0.0f, std::cos(degree), -std::sin(degree), 0.0f},
      vec4{0.0f, std::sin(degree), std::cos(degree), 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

constexpr mat4x4 rotate_y(f32 degree) {
  return mat4x4{
      vec4{std::cos(degree), 0.0f, std::sin(degree), 0.0f},
      vec4{0.0f, 1.0f, 0.0f, 0.0f},
      vec4{-std::sin(degree), 0, std::cos(degree), 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

constexpr mat4x4 rotate_z(f32 degree) {
  return mat4x4{
      vec4{std::cos(degree), -std::sin(degree), 0.0f, 0.0f},
      vec4{std::sin(degree), std::cos(degree), 0.0f, 0.0f},
      vec4{0.0f, 0.0f, 1.0f, 0.0f},
      vec4{0.0f, 0.0f, 0.0f, 1.0f},
  };
}

};  // namespace transforms

// TODO(lamarrr): what about positioning?
struct DrawCommand {
  u32 indices_offset = 0;
  u32 num_triangles = 0;
  f32 opacity = 1.0f;
  mat4x4 transform;  //  transform contains position (translation from origin)
  Color color = colors::BLACK;
  stx::Option<Image> texture;
  Shader vert_shader;
  Shader frag_shader;  // - clip options will apply in the fragment
                       // and vertex shaders
                       // - blending
};

struct DrawList {
  stx::Vec<vec3> vertices{stx::os_allocator};
  stx::Vec<u32> indices{stx::os_allocator};
  stx::Vec<DrawCommand> commands{stx::os_allocator};
};

// TODO(lamarrr): deduplicate points
// TODO(lamarrr): this will mean we need to also be able to render lines using
// points, requiring us to smoothen the edges of the lines
//
// points are specified in counter-clockwise direction
//
// using Polygon = stx::Vec<vec2>;

// TODO(lamarrr): how do we handle selection of transformed widgets?
// Topleft origin coordinate system
struct Canvas {
  vec3 position;
  vec2 extent;
  Brush brush;

  mat4x4 transform = mat4x4::identity();
  stx::Vec<mat4x4> transform_state_stack{stx::os_allocator};

  stx::Vec<stx::Vec<vec2>> clipping_polygons{stx::os_allocator};
  stx::Vec<stx::Vec<stx::Vec<vec2>>> clipping_polygons_state_stack{
      stx::os_allocator};

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

  Canvas& move_to(f32 x, f32 y, f32 z) {
    position = vec3{x, y, z};
    return *this;
  }

  Canvas& move_to(f32 x, f32 y) { return move_to(x, y, position.z); }

  // TODO(lamarrr): we can reuse this for others without needing to
  // allocate anything
  u32 __reserve_rect() {
    u32 start = draw_list.vertices.size();

    draw_list.vertices.push(vec3{0.0f, 0.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{1.0f, 0.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{1.0f, 1.0f, 0.0f}).unwrap();
    draw_list.vertices.push(vec3{0.0f, 1.0f, 0.0f}).unwrap();

    draw_list.indices.push_inplace(start).unwrap();
    draw_list.indices.push_inplace(start + 1).unwrap();
    draw_list.indices.push_inplace(start + 2).unwrap();
    draw_list.indices.push_inplace(start + 2).unwrap();
    draw_list.indices.push_inplace(start).unwrap();
    draw_list.indices.push_inplace(start + 3).unwrap();

    return start;
  }

  Canvas& clear() {
    u32 start = __reserve_rect();

    draw_list.commands
        .push(DrawCommand{
            .indices_offset = start,
            .num_triangles = 2,
            .transform = transforms::scale(vec3{extent.x, extent.y, 1.0f}),
            .color = brush.color,
            .texture = stx::None,
            .vert_shader = {},
            .frag_shader = {}})
        .unwrap();

    return *this;
  }


  Canvas& clip_rect();
  Canvas& clip_round_rect();
  Canvas& clip_slanted_rect();
  Canvas& clip_circle();
  Canvas& clip_ellipse();

  // TODO(lamarrr): clipping line polygons doesn't work this way
  // TODO(lamarrr): path closing for lines?
  // polygon will be closed
  // this is just stroke line really?
  Canvas& draw_line(stx::Span<vec2 const> line) {
    ASR_ENSURE(line.size() >= 2);

    for (usize i = 0; i < line.size(); i++) {
      usize j = i + 1;

      if (i == clipped_polygon.size()) j = 0;

      vec2 p1 = clipped_polygon[i];
      vec2 p2 = clipped_polygon[j];

      vec2 d = p2 - p1;

      {
        float d2 = dot(d, d);
        if (d2 > 0.0f) {
          float inv_len = 1 / std::sqrt(d2);
          d.x *= inv_len;
          d.y *= inv_len;
        }
      }

      d.x *= brush.line_width * 0.5f;
      d.y *= brush.line_width * 0.5f;

      u32 start = draw_list.indices.size();

      vec3 vertices[] = {{p1.x + d.y, p1.y - d.x, 0.0f},
                         {p2.x + d.y, p2.y - d.x, 0.0f},
                         {p2.x - d.y, p2.y + d.x, 0.0f},
                         {p1.x - d.y, p1.y + d.x, 0.0f}};

      u32 indices[] = {start,     start + 1, start + 2,
                       start + 3, start + 4, start + 5};

      draw_list.vertices.extend(vertices).unwrap();
      draw_list.indices.extend(indices).unwrap();

      draw_list.commands
          .push(DrawCommand{.color,
                            .frag_shader,
                            .indices_offset = start,
                            .num_triangles = 2,
                            .opacity,
                            .texture,
                            .transform,
                            .vert_shader})
          .unwrap();
    }

    return *this;
  }

  Canvas& draw_filled_polygon(stx::Span<vec2 const> polygon) {
    ASR_ENSURE(polygon.size() >= 3);

    stx::Vec<vec2> clipped_polygon{stx::os_allocator};

    clipped_polygon.extend(polygon).unwrap();

    for (stx::Vec<vec2> const& clip : clipping_polygons) {
      clipped_polygon = clip_polygon(clipped_polygon, clip);
    }

    stx::Vec<vec2> polygon_vertices = triangulate_polygon(clipped_polygon);

    for (vec2 vertex : polygon_vertices) {
      draw_list.vertices.push(vec3{vertex.x, vertex.y, 0.0f}).unwrap();
    }

    for (usize index = draw_list.indices.size();
         index < (draw_list.indices.size() + polygon_vertices.size());
         index++) {
      draw_list.indices.push_inplace(index).unwrap();
      index++;
    }

    u32 num_triangles = polygon_vertices.size() / 3U;

    draw_list.commands
        .push(DrawCommand{.color,
                          .frag_shader,
                          .indices_offset,
                          .num_triangles = num_triangles,
                          .opacity,
                          .texture,
                          .transform,
                          .vert_shader})
        .unwrap();

    return *this;
  }

  Canvas& draw_stroke_polygon();

  Canvas& draw_line_to(vec2 point) {
    position;
    point;

    u32 start = draw_list.vertices.size();
    u64 line_width = brush.line_width;
    vec3 p1 = position;
    vec3 p2{point.x, point.y, position.z};

    vec3 vertices[] = {{0.0f, 0.0f, 0.0f},
                       {1.0f, 0.0f, 0.0f},
                       {1.0f, 1.0f, 0.0f},
                       {0.0f, 1.0f, 0.0f}};

    draw_list.vertices.extend(vertices).unwrap();

    u32 indices[] = {start, start + 1, start + 2, start + 2, start, start + 3};

    draw_list.indices.extend(indices).unwrap();

    draw_list.commands.push(DrawCommand{.color,
                                        .frag_shader,
                                        .indices_offset,
                                        .num_triangles,
                                        .opacity,
                                        .texture,
                                        .transform,
                                        .vert_shader});

    return *this;
  }

  Canvas& draw_rect(RectF area) {
    // TODO(lamarrr): this will only work for filled rects
    u32 start = __reserve_rect();

    mat4x4 transforms =
        transform *
        (transforms::translate(vec3{area.offset.x, area.offset.y, 0.0f}) *
         transforms::scale(vec3{area.extent.w, area.extent.h, 1.0f}));

    brush.pattern;  // TODO(lamarrr): what about textured backgrounds?
    draw_list.commands
        .push(DrawCommand{.indices_offset = start,
                          .num_triangles = 2,
                          .transform = transforms,
                          .color = brush.color,
                          .texture = stx::None,
                          .vert_shader = {},
                          .frag_shader = {}})
        .unwrap();

    return *this;
  }

  Canvas& draw_round_rect(RectF area, vec4 radii);
  Canvas& draw_slanted_rect(RectF area);
  // within circle and within a rect that comtains
  // that circle (for filled arc)
  Canvas& draw_circle(OffsetF center, f32 radius);
  Canvas& draw_ellipse(OffsetF center, ExtentF radius, f32 rotation,
                       f32 start_angle, f32 end_angle);

  // Text API
  Canvas& draw_text(stx::StringView text, OffsetF position);

  // Image API
  Canvas& draw_image(Image image, OffsetF position);
  Canvas& draw_image(Image image, RectF target);
  Canvas& draw_image(Image image, RectF portion, RectF target);
};

void sample(Canvas& canvas) {
  canvas.save()
      .rotate(45)
      .draw_circle({0, 0}, 20.0f)
      .draw_image(Image{}, {{0.0, 0.0}, {20, 40}})
      .restore()
      .move_to(0, 0)
      .scale(2.0f, 2.0f)
      .draw_line_to({200, 200})
      .draw_text("Hello World, こんにちは世界", {10.0f, 10.0f})
      .draw_rect({0, 0, 20, 20})
      .draw_round_rect({{0.0f, 0.0f}, {20.0f, 20.0f}},
                       {10.0f, 10.0f, 10.0f, 10.0f});
}

void record(DrawList const& draw_list, stx::Rc<vk::Device*> const& device,
            vk::CommandQueueFamilyInfo const& graphics_command_queue,
            VkPhysicalDeviceMemoryProperties const& memory_properties) {
  VkCommandBuffer command_buffer;  // part of recording context

  ASR_VK_CHECK(vkResetCommandBuffer(command_buffer, 0));

  auto vertex_buffer = upload_vertices(device, graphics_command_queue,
                                       memory_properties, draw_list.vertices);
  auto index_buffer = upload_indices(device, graphics_command_queue,
                                     memory_properties, draw_list.indices);
  // texture uploads
  // buffer uploads
  // - upload vertex buffer
  // - upload index buffer
  // - upload textures

  // we need to retain texture uploads, we can't be reuploading them every
  // time

  for (DrawCommand const& draw_command : draw_list.commands) {
    // vkCmdBindPipeline();
    // vkCmdBindDescriptorSets();
    // vkCmdBindVertexBuffers(command_buffer, 0, 1,
    // &vertex_buffer.handle->buffer,
    //    0);
    // vkCmdBindIndexBuffer(command_buffer,
    // index_buffer.handle->buffer, 0,
    //  VK_INDEX_TYPE_UINT32);
    //
    //
    // vkCmdDrawIndexed(command_buffer, 0, 0, 0, 0, 0);
    // vkCmdSetScissor();
    // vkCmdSetViewport();
  }
}

}  // namespace gfx
};  // namespace asr
