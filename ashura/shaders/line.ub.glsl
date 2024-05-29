
// todo(lamarrr):
// add color interp (with thickness accounted for), macros, and config

// render arcs and line segments

struct Line
{
  float thickness;
  float edge_smoothness;
  float cap_roundness;
  float cap_alpha;
  vec4  color[2];
  int   color_space_transform;
};

//
// get bary coords using hw interp, in object's space only:
// https://stackoverflow.com/questions/25397586/accessing-barycentric-coordinates-inside-fragment-shader
//
// use loop-blinn for curve.
// using curve get sdf.
//
//
// Problem: when cps are same, the line becomes invincible
//

struct Arc
{
  float begin;
  float end;
  vec2  radius;
  vec4  center;
  float thickness;
  float edge_smoothness;
  float cap_roundness;
  float cap_alpha;
  int   color_space_transform;
  vec4  color[2];
};

/// TODO(lamarrr):
/// cap has to be a sphere in 3d space.
/// arcs must be in 3d space as well and be 3d volumes

/// @param position relative to center
/// @see https://www.shadertoy.com/view/wl23RK,
/// https://www.shadertoy.com/view/lftXz4
float arc_line_sdf(float begin, float end, vec2 radius, vec2 position)
{
  float ellipse_sdf = length(position - radius);
  // combine with sdf of angle
  // if within the arc, return sdf to center, otherwise return sdf to arc
  return 0;
}

/// TODO(lamarrr): we can combine multiple sdfs to achieve certain effects
/// masking, exclusion, etc.
/// we can classify the sdfs similar to slug