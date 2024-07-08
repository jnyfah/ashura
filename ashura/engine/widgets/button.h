
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/widget.h"
#include "ashura/engine/widgets/text_box.h"
#include "ashura/std/types.h"

namespace ash
{

struct Button : public Widget
{
  SizeConstraint width      = {};
  SizeConstraint height     = {};
  bool           disabled   = false;
  Fn<void()>     on_clicked = to_fn([] {});
  Fn<void()>     on_hovered = to_fn([] {});

  virtual Widget *child(u32 i) override final
  {
    return child_iter({item()}, i);
  }

  virtual Widget *item()
  {
    return nullptr;
  }
};

// TODO(lamarrr):
struct TextButton : public Button
{
  TextBox text = {};
};

}        // namespace ash