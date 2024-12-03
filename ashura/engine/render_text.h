/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/canvas.h"
#include "ashura/engine/text.h"
#include "ashura/engine/view.h"
#include "ashura/std/error.h"
#include "ashura/std/text.h"
#include "ashura/std/types.h"

namespace ash
{

struct TextHighlightStyle
{
  ColorGradient color        = {};
  CornerRadii   corner_radii = {};
};

struct TextHighlight
{
  Slice32            slice = {};
  TextHighlightStyle style = {};
};

/// @brief Controls and manages GUI text state for rendering
/// - manages runs and run styling
/// - manages and checks for text layout invalidation
/// - recalculate text layout when it changes if necessary
/// - renders the text using the computed style information
/// @param runs  Run-End encoded sequences of the runs
struct RenderText
{
  struct
  {
    bool               dirty         = true;
    bool               use_kerning   = true;
    bool               use_ligatures = true;
    TextDirection      direction     = TextDirection::LeftToRight;
    f32                alignment     = -1;
    Vec<c32>           text          = {};
    Vec<u32>           runs          = {};
    Vec<TextStyle>     styles        = {};
    Vec<FontStyle>     fonts         = {};
    Span<char const>   language      = {};
    TextLayout         layout        = {};
    Vec<TextHighlight> highlights    = {};

    /// @brief  Styles specified runs of text, performing run merging and
    /// splitting in the process. If there's previously no runs, the first added
    /// run will be the default and span the whole of the text.
    /// @param first first codepoint index to be patched
    /// @param count range of the number of codepoints to be patched
    /// @param style font style to be applied
    /// @param font font configuration to be applied
    void style(TextStyle const &style, FontStyle const &font, u32 first = 0,
               u32 count = U32_MAX)
    {
      if (count == 0)
      {
        return;
      }

      if (runs.is_empty())
      {
        runs.push(U32_MAX).unwrap();
        styles.push(style).unwrap();
        fonts.push(font).unwrap();
        dirty = true;
        return;
      }

      u32 const end = sat_add(first, count);

      u32 first_run = 0;
      for (; first_run < runs.size32(); first_run++)
      {
        if (runs[first_run] > first)
        {
          break;
        }
      }

      /// should never happen since there's always a U32_MAX run end
      CHECK(first_run < runs.size32());

      u32 last_run = first_run;

      for (; last_run < runs.size32(); last_run++)
      {
        if (runs[last_run] >= end)
        {
          break;
        }
      }

      /// should never happen since there's always a U32_MAX run end
      CHECK(last_run < runs.size32());

      u32 const first_run_begin = (first_run == 0) ? 0 : runs[first_run - 1];
      u32 const last_run_end    = runs[last_run];

      /// run merging

      /// merge middle

      if (last_run > (first_run + 1))
      {
        u32 const first_erase = first_run + 1;
        u32 const num_erase   = last_run - first_erase;
        runs.erase(first_erase, num_erase);
        styles.erase(first_erase, num_erase);
        fonts.erase(first_erase, num_erase);
        last_run -= num_erase;
      }

      /// merge left
      if (first_run_begin == first)
      {
        u32 const first_erase = first_run;
        u32 const num_erase   = last_run - first_run;
        runs.erase(first_erase, num_erase);
        styles.erase(first_erase, num_erase);
        fonts.erase(first_erase, num_erase);
        last_run -= num_erase;
      }

      /// merge right
      if (last_run_end == end)
      {
        u32 const first_erase = first_run + 1;
        u32 const num_erase   = (last_run + 1) - first_erase;
        runs.erase(first_erase, num_erase);
        styles.erase(first_erase, num_erase);
        fonts.erase(first_erase, num_erase);
        last_run -= num_erase;
      }

      (void) last_run;

      /// run splitting
      if (first_run_begin == first && last_run_end == end)
      {
        styles[first_run] = style;
        fonts[first_run]  = font;
      }
      else
      {
        if (first_run_begin == first)
        {
          // split with new on left
          runs.insert(first_run, end).unwrap();
          styles.insert(first_run, style).unwrap();
          fonts.insert(first_run, font).unwrap();
        }
        else if (last_run_end == end)
        {
          // split with new on right
          runs[first_run] = first;
          runs.insert(first_run + 1, end).unwrap();
          styles.insert(first_run + 1, style).unwrap();
          fonts.insert(first_run + 1, font).unwrap();
        }
        else
        {
          // split with new in the middle of the run
          runs[first_run] = first;
          runs.insert(first_run + 1, end).unwrap();
          styles.insert(first_run + 1, style).unwrap();
          fonts.insert(first_run + 1, font).unwrap();
          runs.insert(first_run + 2, last_run_end).unwrap();
          styles.insert(first_run + 2, styles[first_run]).unwrap();
          fonts.insert(first_run + 2, fonts[first_run]).unwrap();
        }
      }

      dirty = true;
    }
  } inner = {};

  void flush_text()
  {
    inner.dirty = true;
  }

  void highlight(TextHighlight const &highlight)
  {
    inner.highlights.push(highlight).unwrap();
  }

  void clear_highlights()
  {
    inner.highlights.clear();
  }

  void set_direction(TextDirection direction)
  {
    if (inner.direction == direction)
    {
      return;
    }
    inner.direction = direction;
    flush_text();
  }

  void set_language(Span<char const> language)
  {
    if (range_eq(inner.language, language))
    {
      return;
    }
    inner.language = language;
    flush_text();
  }

  void set_alignment(f32 alignment)
  {
    if (inner.alignment == alignment)
    {
      return;
    }
    inner.alignment = alignment;
    flush_text();
  }

  Span<c32 const> get_text() const
  {
    return span(inner.text);
  }

  void set_text(Span<c32 const> utf32, TextStyle const &style,
                FontStyle const &font)
  {
    set_text(utf32);
    inner.style(style, font);
    flush_text();
  }

  void set_text(Span<c32 const> utf32)
  {
    inner.text.clear();
    inner.text.extend_copy(utf32).unwrap();
    flush_text();
  }

  void set_text(Span<c8 const> utf8, TextStyle const &style,
                FontStyle const &font)
  {
    inner.style(style, font);
    set_text(utf8);
    flush_text();
  }

  void set_text(Span<c8 const> utf8)
  {
    inner.text.clear();
    utf8_decode(utf8, inner.text).unwrap();
    flush_text();
  }

  void style(TextStyle const &style, FontStyle const &font, u32 first = 0,
             u32 count = U32_MAX)
  {
    inner.style(style, font, first, count);
    flush_text();
  }

  TextBlock block() const
  {
    return TextBlock{.text          = span(inner.text),
                     .runs          = span(inner.runs),
                     .fonts         = span(inner.fonts),
                     .direction     = inner.direction,
                     .language      = inner.language,
                     .use_kerning   = inner.use_kerning,
                     .use_ligatures = inner.use_ligatures};
  }

  TextBlockStyle block_style(f32 aligned_width) const
  {
    return TextBlockStyle{.runs        = span(inner.styles),
                          .alignment   = inner.alignment,
                          .align_width = aligned_width};
  }

  void layout(f32 max_width)
  {
    if (!inner.dirty && max_width == inner.layout.extent.x)
    {
      return;
    }

    layout_text(block(), max_width, inner.layout);
  }

  void render(Canvas &canvas, CRect const &region, CRect const &clip,
              f32 zoom) const
  {
    (void) zoom;
    canvas.text({.center = region.center}, block(), inner.layout,
                block_style(region.extent.x), clip);
    // [ ] zoom
    // [ ] render highlights
    // [ ] are the cursor indexes correct?
    // [ ] use overlays on intersecting graphemes
    // [ ] scaling
  }
};

}        // namespace ash