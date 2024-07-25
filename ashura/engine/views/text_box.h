/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/render_text.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/std/text.h"
#include "ashura/std/types.h"

namespace ash
{

struct TextBox : View, Pin<>
{
  bool               copyable : 1           = false;
  ColorGradient      highlight_color        = {};
  Vec4               highlight_corner_radii = {0, 0, 0, 0};
  mutable RenderText text                   = {};

  TextBox()                   = default;
  virtual ~TextBox() override = default;

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    // [ ] if(events.drag_update )
    return ViewState{.draggable = copyable};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) const override
  {
    text.calculate_layout(allocated.x);
    return text.inner.layout.extent;
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) const override
  {
    text.render(region, clip, canvas);
  }

  virtual Cursor cursor(CRect const &, Vec2) const override
  {
    return copyable ? Cursor::Text : Cursor::Default;
  }
};

struct TextInput : View, Pin<>
{
  bool               disabled : 1           = false;
  bool               editing : 1            = false;
  bool               submit : 1             = false;
  bool               focus_in : 1           = false;
  bool               focus_out : 1          = false;
  bool               is_multiline : 1       = false;
  bool               enter_submits : 1      = false;
  bool               tab_input : 1          = false;
  ColorGradient      highlight_color        = {};
  Vec4               highlight_corner_radii = {0, 0, 0, 0};
  u32                lines_per_page         = 1;
  mutable RenderText content                = {};
  mutable RenderText placeholder            = {};
  TextCompositor     compositor             = {};
  Fn<void()>         on_edit                = fn([] {});
  Fn<void()>         on_submit              = fn([] {});
  Fn<void()>         on_focus_in            = fn([] {});
  Fn<void()>         on_focus_out           = fn([] {});

  TextInput() = default;
  virtual ~TextInput() override
  {
    content.reset();
    placeholder.reset();
  }

  constexpr TextCommand command(ViewContext const &ctx) const
  {
    if (ctx.key_down(KeyCode::Escape))
    {
      return TextCommand::Unselect;
    }
    if (ctx.key_down(KeyCode::Backspace))
    {
      return TextCommand::BackSpace;
    }
    if (ctx.key_down(KeyCode::Delete))
    {
      return TextCommand::Delete;
    }
    if (ctx.key_down(KeyCode::Left))
    {
      return TextCommand::Left;
    }
    if (ctx.key_down(KeyCode::Right))
    {
      return TextCommand::Right;
    }
    if (ctx.key_down(KeyCode::Home))
    {
      return TextCommand::LineStart;
    }
    if (ctx.key_down(KeyCode::End))
    {
      return TextCommand::LineEnd;
    }
    if (ctx.key_down(KeyCode::Up))
    {
      return TextCommand::Up;
    }
    if (ctx.key_down(KeyCode::Down))
    {
      return TextCommand::Down;
    }
    if (ctx.key_down(KeyCode::PageUp))
    {
      return TextCommand::PageUp;
    }
    if (ctx.key_down(KeyCode::PageDown))
    {
      return TextCommand::PageDown;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::Left))
    {
      return TextCommand::SelectLeft;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::Right))
    {
      return TextCommand::SelectRight;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::Up))
    {
      return TextCommand::SelectUp;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::Down))
    {
      return TextCommand::SelectDown;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::PageUp))
    {
      return TextCommand::SelectPageUp;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::PageDown))
    {
      return TextCommand::SelectPageDown;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::A))
    {
      return TextCommand::SelectAll;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::X))
    {
      return TextCommand::Cut;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::C))
    {
      return TextCommand::Copy;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::V))
    {
      return TextCommand::Paste;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::Z))
    {
      return TextCommand::Undo;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::Y))
    {
      return TextCommand::Redo;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::Left) &&
        has_bits(ctx.mouse_buttons, MouseButtons::Primary))
    {
      return TextCommand::HitSelect;
    }
    if (is_multiline && !enter_submits && ctx.key_down(KeyCode::Return))
    {
      return TextCommand::NewLine;
    }
    if (tab_input && ctx.key_down(KeyCode::Tab))
    {
      return TextCommand::Tab;
    }
    return TextCommand::None;
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    bool edited = false;
    auto erase  = [this, &edited](Slice32 s) {
      this->content.inner.text.erase(s);
      edited |= s.is_empty();
      this->content.flush_text();
    };

    auto insert = [this, &edited](u32 pos, Span<u32 const> t) {
      CHECK(this->content.inner.text.insert_span_copy(pos, t));
      edited |= t.is_empty();
      this->content.flush_text();
    };

    editing   = false;
    submit    = false;
    focus_in  = false;
    focus_out = false;

    TextCommand cmd = TextCommand::None;
    if (!disabled)
    {
      if (events.text_input)
      {
        cmd = TextCommand::InputText;
      }
      else if (events.key_down)
      {
        cmd = command(ctx);
      }
      else if (events.drag_start)
      {
        cmd = TextCommand::Hit;
      }
      else if (events.drag_update)
      {
        cmd = TextCommand::HitSelect;
      }
    }

    Vec2 offset = region.begin() - ctx.mouse_position;

    compositor.command(span(content.inner.text), content.inner.layout,
                       region.extent.x, content.inner.alignment, cmd,
                       fn(&insert), fn(&erase), ctx.text, *ctx.clipboard,
                       lines_per_page, offset);

    content.set_highlight(
        compositor.get_cursor().as_slice(content.inner.text.size32()));
    content.set_highlight_style(highlight_color, highlight_corner_radii);

    if (edited)
    {
      editing = true;
    }

    if (events.focus_in)
    {
      focus_in = true;
    }
    else if (events.focus_out)
    {
      compositor.unselect();
      focus_out = true;
    }

    if (enter_submits && ctx.key_down(KeyCode::Return))
    {
      submit = true;
    }

    if (focus_in)
    {
      on_focus_in();
    }

    if (focus_out)
    {
      on_focus_out();
    }

    if (submit)
    {
      on_submit();
    }

    if (edited)
    {
      on_edit();
    }

    return ViewState{.draggable  = !disabled,
                     .focusable  = !disabled,
                     .text_input = !disabled,
                     .tab_input  = tab_input,
                     .lose_focus = ctx.key_down(KeyCode::Escape)};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) const override
  {
    placeholder.calculate_layout(allocated.x);
    content.calculate_layout(allocated.x);
    if (content.inner.text.is_empty())
    {
      return placeholder.inner.layout.extent;
    }
    return content.inner.layout.extent;
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) const override
  {
    if (content.inner.text.is_empty())
    {
      placeholder.render(region, clip, canvas);
    }
    else
    {
      content.render(region, clip, canvas);
    }
  }

  virtual Cursor cursor(CRect const &, Vec2) const override
  {
    return Cursor::Text;
  }
};

// [ ] selection for copy and paste, copyable attribute
// [ ] font hashmap and loader
// [ ] using strings for font ids and its atlases

}        // namespace ash