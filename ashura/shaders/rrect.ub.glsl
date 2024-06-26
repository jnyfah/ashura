#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "core.glsl"

#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_RIGHT 2
#define BOTTOM_LEFT 3

struct Params
{
  vec4  transform[4];
  vec4  radii;
  vec4  uv;
  vec4  tint[4];
  vec2  aspect_ratio;
  float edge_smoothness;
  uint  albedo;
};

layout(set = 0, binding = 0) readonly buffer ParamBuffer
{
  Params params[];
};

layout(set = 1, binding = 0) uniform sampler2D textures[];

const uint INDEX_BUFFER[]  = {0, 1, 2, 2, 3, 0};
const vec2 VERTEX_BUFFER[] = {vec2(-1, -1), vec2(1, -1), vec2(1, 1),
                              vec2(-1, 1)};

// https://www.shadertoy.com/view/fsdyzB
// https://iquilezles.org/articles/distfunctions/
// length(...+ border_radius) - border_radius -> gives the rounding of the
// border
float rrect_sdf(vec2 pos, vec2 half_extent, float border_radius)
{
  return length(max(abs(pos) - half_extent + border_radius, 0)) - border_radius;
}

#ifdef VERTEX_SHADER

layout(location = 0) out vec2 o_pos;
layout(location = 1) out vec4 o_color;
layout(location = 2) flat out uint o_instance;

void main()
{
  Params p      = params[gl_InstanceIndex];
  uint   vertex = INDEX_BUFFER[gl_VertexIndex];
  vec2   i_pos  = VERTEX_BUFFER[vertex];
  gl_Position   = to_mat4(p.transform) * vec4(i_pos, 0, 1);
  o_pos         = i_pos * p.aspect_ratio;
  o_color       = p.tint[vertex];
  o_instance    = gl_InstanceIndex;
}
#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec2 i_pos;
layout(location = 1) in vec4 i_color;
layout(location = 2) flat in uint i_instance;

layout(location = 0) out vec4 o_color;

void main()
{
  Params p    = params[i_instance];
  bool   left = i_pos.x < 0;
  bool   top  = i_pos.y < 0;
  uint   corner =
      left ? (top ? TOP_LEFT : BOTTOM_LEFT) : (top ? TOP_RIGHT : BOTTOM_RIGHT);
  float radius      = p.radii[corner];
  vec2  half_extent = p.aspect_ratio;
  float dist        = rrect_sdf(i_pos, half_extent, radius);
  float alpha       = 1 - smoothstep(0, p.edge_smoothness, dist);
  vec2  xy          = i_pos * 0.5 + 0.5;
  vec2  uv          = mix(p.uv.xy, p.uv.zw, xy);
  vec4  color       = texture(textures[nonuniformEXT(p.albedo)], uv) * i_color;
  o_color           = vec4(color.rgb, color.a * alpha);
}
#endif