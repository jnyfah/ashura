/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
#include "ashura/rect_pack.h"

#include <stdlib.h>

#include "ashura/utils.h"

#define STBRP__NOTUSED(v) (void)v

enum { STBRP__INIT_skyline = 1 };

void stbrp_setup_heuristic(stbrp_context *context, stbrp_heuristic heuristic) {
  switch (context->init_mode) {
    case STBRP__INIT_skyline:
      ASR_CHECK(heuristic == STBRP_HEURISTIC_Skyline_BL_sortHeight ||
                heuristic == STBRP_HEURISTIC_Skyline_BF_sortHeight);
      context->heuristic = heuristic;
      break;
    default:
      ASR_UNREACHABLE();
  }
}

void stbrp_setup_allow_out_of_mem(stbrp_context *context,
                                  int32_t allow_out_of_mem) {
  if (allow_out_of_mem) {
    // if it's ok to run out of memory, then don't bother aligning them;
    // this gives better packing, but may fail due to OOM (even though
    // the rectangles easily fit). @TODO a smarter approach would be to only
    // quantize once we've hit OOM, then we could get rid of this parameter.
    context->align = 1;
  } else {
    // if it's not ok to run out of memory, then quantize the widths
    // so that num_nodes is always enough nodes.
    //
    // I.e. num_nodes * align >= width
    //                  align >= width / num_nodes
    //                  align = ceil(width/num_nodes)

    context->align =
        (context->width + context->num_nodes - 1) / context->num_nodes;
  }
}

void stbrp_init_target(stbrp_context *context, int32_t width, int32_t height,
                       stbrp_node *nodes, int32_t num_nodes) {
  int32_t i;

  for (i = 0; i < num_nodes - 1; ++i) {
    nodes[i].next = &nodes[i + 1];
  }
  nodes[i].next = nullptr;
  context->init_mode = STBRP__INIT_skyline;
  context->heuristic = STBRP_HEURISTIC_Skyline_default;
  context->free_head = &nodes[0];
  context->active_head = &context->extra[0];
  context->width = width;
  context->height = height;
  context->num_nodes = num_nodes;
  stbrp_setup_allow_out_of_mem(context, 0);

  // node 0 is the full width, node 1 is the sentinel (lets us not store width
  // explicitly)
  context->extra[0].x = 0;
  context->extra[0].y = 0;
  context->extra[0].next = &context->extra[1];
  context->extra[1].x = width;
  context->extra[1].y = (1 << 30);
  context->extra[1].next = nullptr;
}

// find minimum y position if it starts at x1
inline int32_t stbrp__skyline_find_min_y(stbrp_context *c, stbrp_node *first,
                                         int32_t x0, int32_t width,
                                         int32_t *pwaste) {
  stbrp_node *node = first;
  int32_t x1 = x0 + width;
  int32_t min_y, visited_width, waste_area;

  STBRP__NOTUSED(c);

  ASR_CHECK(first->x <= x0);

#if 0
   // skip in case we're past the node
   while (node->next->x <= x0)
      ++node;
#else
  ASR_CHECK(node->next->x >
            x0);  // we ended up handling this in the caller for efficiency
#endif

  ASR_CHECK(node->x <= x0);

  min_y = 0;
  waste_area = 0;
  visited_width = 0;
  while (node->x < x1) {
    if (node->y > min_y) {
      // raise min_y higher.
      // we've accounted for all waste up to min_y,
      // but we'll now add more waste for everything we've visted
      waste_area += visited_width * (node->y - min_y);
      min_y = node->y;
      // the first time through, visited_width might be reduced
      if (node->x < x0)
        visited_width += node->next->x - x0;
      else
        visited_width += node->next->x - node->x;
    } else {
      // add waste area
      int32_t under_width = node->next->x - node->x;
      if (under_width + visited_width > width)
        under_width = width - visited_width;
      waste_area += under_width * (min_y - node->y);
      visited_width += under_width;
    }
    node = node->next;
  }

  *pwaste = waste_area;
  return min_y;
}

typedef struct {
  int32_t x, y;
  stbrp_node **prev_link;
} stbrp__findresult;

inline stbrp__findresult stbrp__skyline_find_best_pos(stbrp_context *c,
                                                      int32_t width,
                                                      int32_t height) {
  int32_t best_waste = (1 << 30), best_x, best_y = (1 << 30);
  stbrp__findresult fr;
  stbrp_node **prev, *node, *tail, **best = nullptr;

  // align to multiple of c->align
  width = (width + c->align - 1);
  width -= width % c->align;
  ASR_CHECK(width % c->align == 0);

  // if it can't possibly fit, bail immediately
  if (width > c->width || height > c->height) {
    fr.prev_link = nullptr;
    fr.x = fr.y = 0;
    return fr;
  }

  node = c->active_head;
  prev = &c->active_head;
  while (node->x + width <= c->width) {
    int32_t y, waste;
    y = stbrp__skyline_find_min_y(c, node, node->x, width, &waste);
    if (c->heuristic ==
        STBRP_HEURISTIC_Skyline_BL_sortHeight) {  // actually just want to test
                                                  // BL
      // bottom left
      if (y < best_y) {
        best_y = y;
        best = prev;
      }
    } else {
      // best-fit
      if (y + height <= c->height) {
        // can only use it if it first vertically
        if (y < best_y || (y == best_y && waste < best_waste)) {
          best_y = y;
          best_waste = waste;
          best = prev;
        }
      }
    }
    prev = &node->next;
    node = node->next;
  }

  best_x = (best == nullptr) ? 0 : (*best)->x;

  // if doing best-fit (BF), we also have to try aligning right edge to each
  // node position
  //
  // e.g, if fitting
  //
  //     ____________________
  //    |____________________|
  //
  //            into
  //
  //   |                         |
  //   |             ____________|
  //   |____________|
  //
  // then right-aligned reduces waste, but bottom-left BL is always chooses
  // left-aligned
  //
  // This makes BF take about 2x the time

  if (c->heuristic == STBRP_HEURISTIC_Skyline_BF_sortHeight) {
    tail = c->active_head;
    node = c->active_head;
    prev = &c->active_head;
    // find first node that's admissible
    while (tail->x < width) tail = tail->next;
    while (tail) {
      int32_t xpos = tail->x - width;
      int32_t y, waste;
      ASR_CHECK(xpos >= 0);
      // find the left position that matches this
      while (node->next->x <= xpos) {
        prev = &node->next;
        node = node->next;
      }
      ASR_CHECK(node->next->x > xpos && node->x <= xpos);
      y = stbrp__skyline_find_min_y(c, node, xpos, width, &waste);
      if (y + height <= c->height) {
        if (y <= best_y) {
          if (y < best_y || waste < best_waste ||
              (waste == best_waste && xpos < best_x)) {
            best_x = xpos;
            ASR_CHECK(y <= best_y);
            best_y = y;
            best_waste = waste;
            best = prev;
          }
        }
      }
      tail = tail->next;
    }
  }

  fr.prev_link = best;
  fr.x = best_x;
  fr.y = best_y;
  return fr;
}

inline stbrp__findresult stbrp__skyline_pack_rectangle(stbrp_context *context,
                                                       int32_t width,
                                                       int32_t height) {
  // find best position according to heuristic
  stbrp__findresult res = stbrp__skyline_find_best_pos(context, width, height);
  stbrp_node *node, *cur;

  // bail if:
  //    1. it failed
  //    2. the best node doesn't fit (we don't always check this)
  //    3. we're out of memory
  if (res.prev_link == nullptr || res.y + height > context->height ||
      context->free_head == nullptr) {
    res.prev_link = nullptr;
    return res;
  }

  // on success, create new node
  node = context->free_head;
  node->x = res.x;
  node->y = res.y + height;

  context->free_head = node->next;

  // insert the new node into the right starting point, and
  // let 'cur' point to the remaining nodes needing to be
  // stiched back in

  cur = *res.prev_link;
  if (cur->x < res.x) {
    // preserve the existing one, so start testing with the next one
    stbrp_node *next = cur->next;
    cur->next = node;
    cur = next;
  } else {
    *res.prev_link = node;
  }

  // from here, traverse cur and free the nodes, until we get to one
  // that shouldn't be freed
  while (cur->next && cur->next->x <= res.x + width) {
    stbrp_node *next = cur->next;
    // move the current node to the free list
    cur->next = context->free_head;
    context->free_head = cur;
    cur = next;
  }

  // stitch the list back in
  node->next = cur;

  if (cur->x < res.x + width) cur->x = res.x + width;

#ifdef _DEBUG
  cur = context->active_head;
  while (cur->x < context->width) {
    ASR_CHECK(cur->x < cur->next->x);
    cur = cur->next;
  }
  ASR_CHECK(cur->next == nullptr);

  {
    int32_t count = 0;
    cur = context->active_head;
    while (cur) {
      cur = cur->next;
      ++count;
    }
    cur = context->free_head;
    while (cur) {
      cur = cur->next;
      ++count;
    }
    ASR_CHECK(count == context->num_nodes + 2);
  }
#endif

  return res;
}

inline int32_t rect_height_compare(void const *a, void const *b) {
  stbrp_rect const *p = (stbrp_rect const *)a;
  stbrp_rect const *q = (stbrp_rect const *)b;
  if (p->h > q->h) return -1;
  if (p->h < q->h) return 1;
  return (p->w > q->w) ? -1 : (p->w < q->w);
}

inline int32_t rect_original_order(void const *a, void const *b) {
  stbrp_rect const *p = (stbrp_rect const *)a;
  stbrp_rect const *q = (stbrp_rect const *)b;
  return (p->was_packed < q->was_packed) ? -1 : (p->was_packed > q->was_packed);
}

int32_t stbrp_pack_rects(stbrp_context *context, stbrp_rect *rects,
                         int32_t num_rects) {
  int32_t i, all_rects_packed = 1;

  // we use the 'was_packed' field internally to allow sorting/unsorting
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = i;
  }

  // sort according to heuristic
  std::qsort(rects, num_rects, sizeof(rects[0]), rect_height_compare);

  for (i = 0; i < num_rects; ++i) {
    if (rects[i].w == 0 || rects[i].h == 0) {
      rects[i].x = rects[i].y = 0;  // empty rect needs no space
    } else {
      stbrp__findresult fr =
          stbrp__skyline_pack_rectangle(context, rects[i].w, rects[i].h);
      if (fr.prev_link) {
        rects[i].x = (int32_t)fr.x;
        rects[i].y = (int32_t)fr.y;
      } else {
        rects[i].x = rects[i].y = STBRP__MAXVAL;
      }
    }
  }

  // unsort
  std::qsort(rects, num_rects, sizeof(rects[0]), rect_original_order);

  // set was_packed flags and all_rects_packed status
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed =
        !(rects[i].x == STBRP__MAXVAL && rects[i].y == STBRP__MAXVAL);
    if (!rects[i].was_packed) all_rects_packed = 0;
  }

  // return the all_rects_packed status
  return all_rects_packed;
}
