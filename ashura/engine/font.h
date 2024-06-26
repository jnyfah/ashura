#pragma once

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIRDEF extern "C" inline

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string_view>
#include <utility>

#include "ashura/image.h"
#include "ashura/loggers.h"
#include "ashura/primitives.h"
#include "ashura/rect_pack.h"
#include "ashura/sdf.h"
#include "ashura/stb_image_resize.h"
#include "ashura/unicode.h"
#include "ashura/utils.h"
#include "freetype/freetype.h"
#include "harfbuzz/hb.h"
#include "stx/enum.h"
#include "stx/limits.h"
#include "stx/span.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace ash
{
constexpr Vec2U DEFAULT_MAX_ATLAS_BIN_EXTENT = Vec2U{1024, 1024};

enum class FontLoadError : u8
{
  PathNotExist,
  InvalidFont,
  UnrecognizedFontName
};

struct Font
{
  stx::String  postscript_name;        // ASCII. i.e. RobotoBold
  stx::String  family_name;            // ASCII. i.e. Roboto
  stx::String  style_name;             // ASCII. i.e. Bold
  hb_blob_t   *hb_blob       = nullptr;
  hb_face_t   *hb_face       = nullptr;
  hb_font_t   *hb_font       = nullptr;
  u32          nfaces        = 0;
  u32          selected_face = 0;
  stx::Vec<u8> data;

  Font(stx::String ipostscript_name, stx::String ifamily_name,
       stx::String istyle_name, hb_blob_t *ihb_blob, hb_face_t *ihb_face,
       hb_font_t *ihb_font, u32 infaces, u32 iselected_face,
       stx::Vec<u8> idata) :
      postscript_name{std::move(ipostscript_name)},
      family_name{std::move(ifamily_name)},
      style_name{std::move(istyle_name)},
      hb_blob{ihb_blob},
      hb_face{ihb_face},
      hb_font{ihb_font},
      nfaces{infaces},
      selected_face{iselected_face},
      data{std::move(idata)}
  {
  }

  STX_MAKE_PINNED(Font)

  ~Font()
  {
    hb_blob_destroy(hb_blob);
    hb_face_destroy(hb_face);
    hb_font_destroy(hb_font);
  }
};

inline stx::Result<stx::Rc<Font *>, FontLoadError>
    load_font_from_memory(stx::Vec<u8> data, u32 selected_face)
{
  hb_blob_t *hb_blob = hb_blob_create(
      data.span().as_char().data(), static_cast<u32>(data.size()),
      HB_MEMORY_MODE_READONLY, nullptr, nullptr);

  ASH_CHECK(hb_blob != nullptr);

  u32 nfaces = hb_face_count(hb_blob);

  hb_face_t *hb_face = hb_face_create(hb_blob, selected_face);

  if (hb_face == nullptr)
  {
    hb_blob_destroy(hb_blob);
    return stx::Err(FontLoadError::InvalidFont);
  }

  hb_font_t *hb_font = hb_font_create(hb_face);

  if (hb_font == nullptr)
  {
    hb_blob_destroy(hb_blob);
    hb_face_destroy(hb_face);
    return stx::Err(FontLoadError::InvalidFont);
  }

  FT_Library ft_lib;
  ASH_CHECK(FT_Init_FreeType(&ft_lib) == 0);

  FT_Face ft_face;

  if (FT_New_Memory_Face(ft_lib, data.data(), static_cast<FT_Long>(data.size()),
                         0, &ft_face) != 0)
  {
    hb_blob_destroy(hb_blob);
    hb_face_destroy(hb_face);
    ASH_CHECK(FT_Done_FreeType(ft_lib) == 0);
    return stx::Err(FontLoadError::InvalidFont);
  }

  char const *p_postscript_name = FT_Get_Postscript_Name(ft_face);
  stx::String postscript_name =
      p_postscript_name == nullptr ?
          "" :
          stx::string::make(stx::os_allocator, p_postscript_name).unwrap();
  stx::String family_name =
      ft_face->family_name == nullptr ?
          "" :
          stx::string::make(stx::os_allocator, ft_face->family_name).unwrap();
  stx::String style_name =
      ft_face->style_name == nullptr ?
          "" :
          stx::string::make(stx::os_allocator, ft_face->style_name).unwrap();

  ASH_CHECK(FT_Done_Face(ft_face) == 0);
  ASH_CHECK(FT_Done_FreeType(ft_lib) == 0);

  return stx::Ok(stx::rc::make_inplace<Font>(
                     stx::os_allocator, std::move(postscript_name),
                     std::move(family_name), std::move(style_name), hb_blob,
                     hb_face, hb_font, nfaces, selected_face, std::move(data))
                     .unwrap());
}

inline stx::Result<stx::Rc<Font *>, FontLoadError>
    load_font_from_file(stx::CStringView path, u32 selected_face)
{
  if (!std::filesystem::exists(path.c_str()))
  {
    return stx::Err(FontLoadError::PathNotExist);
  }

  std::FILE *file = std::fopen(path.c_str(), "rb");
  ASH_CHECK(file != nullptr);

  ASH_CHECK(std::fseek(file, 0, SEEK_END) == 0);

  long file_size = std::ftell(file);
  ASH_CHECK(file_size >= 0);

  stx::Vec<u8> data;
  data.unsafe_resize_uninitialized((usize) file_size).unwrap();

  ASH_CHECK(std::fseek(file, 0, SEEK_SET) == 0);

  ASH_CHECK(std::fread(data.data(), 1, file_size, file) == file_size);

  ASH_CHECK(std::fclose(file) == 0);

  return load_font_from_memory(std::move(data), selected_face);
}

/// atlas containing the packed glyphs
/// This enables support for large glyphs. We load all glyphs of a font into
/// memory. GPUs have texture size limits so we try to bin the font atlas.
///
struct FontAtlasBin
{
  gfx::image texture = 0;
  Vec2U      extent;
  usize      used_area = 0;
};

/// Metrics are normalized
/// @bearing: offset from cursor baseline to start drawing glyph from
/// @descent: distance from baseline to the bottom of the glyph
/// @advance: advancement of the cursor after drawing this glyph
/// @extent: glyph extent
struct GlyphMetrics
{
  Vec2 bearing;
  f32  descent = 0;
  f32  advance = 0;
  Vec2 extent;
};

/// see:
/// https://stackoverflow.com/questions/62374506/how-do-i-align-glyphs-along-the-baseline-with-freetype
///
/// NOTE: using stubs enables us to perform fast constant lookups of glyph
/// indices by ensuring the array is filled and sorted by glyph index from 0 ->
/// nglyphs_found_in_font-1
/// @is_valid: if the glyph was found in the font and loaded
// successfully
/// @is_needed: if the texture is a texture that is needed. i.e. if the
/// unicode ranges are empty then this is always true,
/// otherwise it is set to true if the config unicode ranges
/// contains it, note that special glyphs like replacement
/// unicode codepoint glyph (0xFFFD) will always be true
/// @metrics: normalized font metrics
/// @bin: atlas bin this glyph belongs to
/// @bin_offset: area in the atlas this glyph's cache data is placed
/// @uv0, uv1: normalized texture coordinates of this
/// glyph in the atlas bin
struct Glyph
{
  bool         is_valid  = false;
  bool         is_needed = false;
  GlyphMetrics metrics;
  u32          bin = 0;
  Vec2U        bin_offset;
  Vec2U        bin_extent;
  Vec2         uv0;
  Vec2         uv1;
};

/// stores codepoint glyphs for a font at a specific font height
/// For info on SDF Text Rendering,
/// See:
/// - https://www.youtube.com/watch?v=1b5hIMqz_wM
/// -
/// https://cdn.cloudflare.steamstatic.com/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
///
/// In SDFs each pixel is encoded with its distance to the edge of a shape.
/// The inner portion of the glyph has a value at the midpoint of the text, i.e.
/// encoded 127 + distance away from the glyph boundary. The outer portion of
/// the glyph however is encoded with a value lower than the midpoint. i.e.
/// encoded 0-127.
///
/// @replacement_glyph: glyph for the replacement glyph 0xFFFD if
/// found, otherwise glyph index 0
/// @ellipsis_glyph: glyph for the ellipsis character '…'
/// @font_height: font height at which the this atlas was rendered
/// @ascent: normalized maximum ascent of the font's glyphs
/// @descent: normalized maximum descent of the font's glyphs
/// @advance: normalized maximum advance of the font's glyphs
struct FontAtlas
{
  stx::Vec<Glyph>        glyphs;
  u32                    replacement_glyph = 0;
  u32                    space_glyph       = 0;
  u32                    ellipsis_glyph    = 0;
  u32                    font_height       = 0;
  f32                    ascent            = 0;
  f32                    descent           = 0;
  f32                    advance           = 0;
  stx::Vec<FontAtlasBin> bins;
};

struct BundledFont
{
  stx::String     name;
  stx::Rc<Font *> font;
  FontAtlas       atlas;
};

/// @name: name to use in font matching
/// @path: local file system path of the typeface resource
/// @use_caching: whether to try to load or save font atlas from the cache
/// directory. the font is identified in the cache directory
/// by its postscript name, which is different from its font
/// matching name
/// @face: font face to use
/// @font_height: the height at which the SDF texture is cached at
/// @max_atlas_bin_extent: maximum extent of each atlas bin
/// @ranges: if set only the specified unicode ranges will be loaded,
/// otherwise glyphs in the font will be loaded. Note that this
/// means during font ligature glyph substitution where scripts
/// might change, if the replacement glyph is not in the unicode
/// range, it won't result in a valid glyph.
struct FontSpec
{
  stx::String name;
  stx::String path;
  bool        use_caching              = true;
  u32         face                     = 0;
  u32         font_height              = 40;
  Vec2U       max_atlas_bin_extent     = DEFAULT_MAX_ATLAS_BIN_EXTENT;
  stx::Span<UnicodeRange const> ranges = {};
};

inline std::pair<FontAtlas, stx::Vec<ImageBuffer>>
    render_font_atlas(Font const &font, FontSpec const &spec)
{
  // NOTE: all *64 or << 6, /64 or >> 6 are to convert to and from 26.6 pixel
  // format used in Freetype and Harfbuzz metrics

  if (!spec.ranges.is_empty())
  {
    ASH_LOG_INFO(FontRenderer, "Font: {}'s Needed Unicode Ranges: ",
                 font.postscript_name.c_str());
    for (UnicodeRange range : spec.ranges)
    {
      ASH_LOG_INFO(FontRenderer, "Unicode Range {:x} - {:x}", range.first,
                   range.last);
    }
  }

  FT_Library ft_lib;
  ASH_CHECK(FT_Init_FreeType(&ft_lib) == 0);

  FT_Face ft_face;
  ASH_CHECK(FT_New_Memory_Face(ft_lib, font.data.data(),
                               (FT_Long) font.data.size(),
                               (FT_Long) font.selected_face, &ft_face) == 0);

  ASH_CHECK(FT_Set_Char_Size(ft_face, 0, (FT_F26Dot6) (spec.font_height << 6),
                             72, 72) == 0);

  stx::Vec<Glyph> glyphs;

  u32 const nglyphs           = (u32) ft_face->num_glyphs;
  u32 const replacement_glyph = FT_Get_Char_Index(
      ft_face,
      HB_BUFFER_REPLACEMENT_CODEPOINT_DEFAULT);        // glyph index 0 is
                                                       // selected if the glyph
                                                       // for the replacement
                                                       // codepoint is not found
  u32 const ellipsis_glyph = FT_Get_Char_Index(ft_face, 0x2026);
  u32 const space_glyph    = FT_Get_Char_Index(ft_face, ' ');
  f32 const ascent =
      (ft_face->size->metrics.ascender / 64.0f) / spec.font_height;
  f32 const descent =
      (ft_face->size->metrics.descender / -64.0f) / spec.font_height;
  f32 const advance =
      (ft_face->size->metrics.max_advance / 64.0f) / spec.font_height;

  ASH_LOG_INFO(FontRenderer, "Fetching {} Glyph Metrics For Font: {}", nglyphs,
               font.postscript_name.c_str());

  for (u32 glyph_index = 0; glyph_index < nglyphs; glyph_index++)
  {
    bool const is_needed =
        glyph_index == replacement_glyph ? true : spec.ranges.is_empty();

    if (FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT) == 0) [[likely]]
    {
      FT_GlyphSlot slot = ft_face->glyph;

      GlyphMetrics metrics;

      // convert from 26.6 pixel format
      metrics.bearing.x =
          (slot->metrics.horiBearingX / 64.0f) / spec.font_height;
      metrics.bearing.y =
          (slot->metrics.horiBearingY / 64.0f) / spec.font_height;
      metrics.advance  = (slot->metrics.horiAdvance / 64.0f) / spec.font_height;
      metrics.extent.x = (slot->metrics.width / 64.0f) / spec.font_height;
      metrics.extent.y = (slot->metrics.height / 64.0f) / spec.font_height;
      metrics.descent  = std::max(metrics.extent.y - metrics.bearing.y, 0.0f);

      // bin offsets are determined after binning and during rect packing
      glyphs
          .push(
              Glyph{.is_valid   = true,
                    .is_needed  = is_needed,
                    .metrics    = metrics,
                    .bin        = 0,
                    .bin_offset = Vec2U{0, 0},
                    .bin_extent = Vec2U{slot->bitmap.width, slot->bitmap.rows}})
          .unwrap();
    }
    else
    {
      glyphs
          .push(Glyph{.is_valid   = false,
                      .is_needed  = is_needed,
                      .metrics    = {},
                      .bin        = 0,
                      .bin_offset = {},
                      .bin_extent = {}})
          .unwrap();
    }
  }

  {
    // Iterate through all the characters in the font's CMAP
    FT_UInt  glyph_index  = 0;
    FT_ULong unicode_char = FT_Get_First_Char(ft_face, &glyph_index);
    do
    {
      for (UnicodeRange range : spec.ranges)
      {
        if (unicode_char >= range.first && unicode_char <= range.last)
        {
          glyphs[glyph_index].is_needed = true;
          break;
        }
      }

      unicode_char = FT_Get_Next_Char(ft_face, unicode_char, &glyph_index);
    } while (glyph_index != 0);
  }

  u32 nloaded_glyphs = 0;

  for (Glyph const &glyph : glyphs)
  {
    if (glyph.is_valid && glyph.is_needed) [[likely]]
    {
      nloaded_glyphs++;
    }
  }

  ASH_LOG_INFO(FontRenderer, "Bin Packing Glyphs For Font: {}",
               font.postscript_name.c_str());

  stx::Vec<FontAtlasBin> bins;
  usize                  total_used_area = 0;
  usize                  total_area      = 0;

  {
    stx::Vec<rect_packer::rect> rects;
    rects.unsafe_resize_uninitialized(nloaded_glyphs).unwrap();

    for (u32 glyph_index = 0, irect = 0; glyph_index < nglyphs; glyph_index++)
    {
      Glyph const &glyph = glyphs[glyph_index];
      // only assign packing rects to the valid and needed glyphs
      if (glyph.is_valid && glyph.is_needed) [[likely]]
      {
        rects[irect].glyph_index = glyph_index;
        rects[irect].x           = 0;
        rects[irect].y           = 0;

        // added padding to avoid texture spilling due to accumulated uv
        // interpolation errors
        rects[irect].w = static_cast<i32>(glyphs[glyph_index].bin_extent.x + 2);
        rects[irect].h = static_cast<i32>(glyphs[glyph_index].bin_extent.y + 2);
        irect++;
      }
    }

    stx::Vec<rect_packer::Node> nodes;
    nodes.unsafe_resize_uninitialized(spec.max_atlas_bin_extent.x).unwrap();

    u32                          bin            = 0;
    stx::Span<rect_packer::rect> unpacked_rects = rects;
    bool                         was_all_packed = rects.is_empty();

    while (!unpacked_rects.is_empty())
    {
      rect_packer::Context pack_context = rect_packer::init(
          spec.max_atlas_bin_extent.x, spec.max_atlas_bin_extent.y,
          nodes.data(), spec.max_atlas_bin_extent.x, true);
      // tries to pack all the glyph rects into the provided extent
      was_all_packed =
          rect_packer::pack_rects(pack_context, unpacked_rects.data(),
                                  static_cast<i32>(unpacked_rects.size()));
      auto [just_packed, unpacked] = unpacked_rects.partition(
          [](rect_packer::rect const &r) { return r.was_packed; });
      unpacked_rects = unpacked;

      // NOTE: vulkan doesn't allow zero-extent images
      Vec2U bin_extent{1, 1};
      usize used_area = 0;

      for (rect_packer::rect const &rect : just_packed)
      {
        bin_extent.x = std::max(bin_extent.x, (u32) (rect.x + rect.w));
        bin_extent.y = std::max(bin_extent.y, (u32) (rect.y + rect.h));
        used_area += rect.w * rect.h;
      }

      for (rect_packer::rect const &rect : just_packed)
      {
        Glyph &glyph       = glyphs[rect.glyph_index];
        glyph.bin_offset.x = static_cast<u32>(rect.x) + 1;
        glyph.bin_offset.y = static_cast<u32>(rect.y) + 1;
        glyph.bin          = bin;
        glyph.uv0          = Vec2{(f32) glyph.bin_offset.x / (f32) bin_extent.x,
                         (f32) glyph.bin_offset.y / (f32) bin_extent.y};
        glyph.uv1 = Vec2{((f32) glyph.bin_offset.x + (f32) glyph.bin_extent.x) /
                             (f32) bin_extent.x,
                         ((f32) glyph.bin_offset.y + (f32) glyph.bin_extent.y) /
                             (f32) bin_extent.y};
      }

      bins.push(FontAtlasBin{.texture   = gfx::WHITE_IMAGE,
                             .extent    = bin_extent,
                             .used_area = used_area})
          .unwrap();
      bin++;

      total_used_area += used_area;
      total_area += bin_extent.x * (usize) bin_extent.y;
    }

    // sanity check. ideally all should have been packed
    ASH_CHECK(was_all_packed);
  }

  f32 const packing_efficiency = static_cast<f32>(total_used_area) / total_area;
  usize const total_wasted_area = total_area - total_used_area;

  ASH_LOG_INFO(FontRenderer,
               "Finished Bin Packing Glyphs For Font: {} Into {} Bins With {} "
               "Efficiency (Wasted "
               "Area = {}, Used Area = {}, Total Area = {}) ",
               font.postscript_name.c_str(), bins.size(), packing_efficiency,
               total_wasted_area, total_used_area, total_area);

  stx::Vec<ImageBuffer> bin_buffers;

  for (FontAtlasBin const &bin : bins)
  {
    ImageBuffer buffer =
        ImageBuffer::make(bin.extent, ImageFormat::R8).unwrap();
    // ensures glyphs that failed to load are filled with transparent values
    // also ensures the padded areas are filled with transparent values
    buffer.span().fill(0);
    bin_buffers.push(std::move(buffer)).unwrap();
  }

  for (u32 glyph_index = 0; glyph_index < nglyphs; glyph_index++)
  {
    Glyph const &glyph = glyphs[glyph_index];
    if (glyph.is_valid && glyph.is_needed) [[likely]]
    {
      FT_Error ft_error = FT_Load_Glyph(ft_face, glyph_index, FT_LOAD_DEFAULT);
      if (ft_error != 0) [[unlikely]]
      {
        ASH_LOG_ERR(
            FontRenderer,
            "Failed To Load Glyph At Index: {} For Font: {} (FT_Error = {})",
            glyph_index, font.postscript_name.c_str(), ft_error);
        continue;
      }

      FT_GlyphSlot slot = ft_face->glyph;
      ft_error          = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
      if (ft_error != 0) [[unlikely]]
      {
        ASH_LOG_ERR(FontRenderer,
                    "Failed To Render Glyph At Index: {} for font: {}",
                    glyph_index, font.postscript_name.c_str(), ft_error);
        continue;
      }

      ASH_CHECK(slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);

      bin_buffers[glyph.bin]
          .view()
          .slice(glyph.bin_offset, glyph.bin_extent)
          .copy(ImageSpan<u8>{
              .span   = stx::Span<u8>{slot->bitmap.buffer,
                                      slot->bitmap.rows * slot->bitmap.pitch},
              .extent = Vec2U{slot->bitmap.width, slot->bitmap.rows},
              .pitch  = (usize) slot->bitmap.pitch,
              .format = ImageFormat::R8});
    }
  }

  ASH_LOG_INFO(FontRenderer, "Finished Caching SDF Atlas Bins For Font: {}",
               font.postscript_name.c_str());

  ASH_CHECK(FT_Done_Face(ft_face) == 0);
  ASH_CHECK(FT_Done_FreeType(ft_lib) == 0);

  return std::make_pair(FontAtlas{.glyphs            = std::move(glyphs),
                                  .replacement_glyph = replacement_glyph,
                                  .space_glyph       = space_glyph,
                                  .ellipsis_glyph    = ellipsis_glyph,
                                  .font_height       = spec.font_height,
                                  .ascent            = ascent,
                                  .descent           = descent,
                                  .advance           = advance,
                                  .bins              = std::move(bins)},
                        std::move(bin_buffers));
}

inline usize match_font(std::string_view             font,
                        stx::Span<BundledFont const> font_bundle)
{
  usize i = 0;
  for (BundledFont const &f : font_bundle)
  {
    if (f.name == font)
    {
      return i;
    }
    else
    {
      i++;
    }
  }

  return i;
}

inline usize match_font(std::string_view                  font,
                        stx::Span<std::string_view const> fallback_fonts,
                        stx::Span<BundledFont const>      font_bundle)
{
  usize pos = match_font(font, font_bundle);
  if (pos < font_bundle.size())
  {
    return pos;
  }

  for (std::string_view fallback_font : fallback_fonts)
  {
    pos = match_font(fallback_font, font_bundle);
    if (pos < font_bundle.size())
    {
      return pos;
    }
  }

  return pos;
}

}        // namespace ash
