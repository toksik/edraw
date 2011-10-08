/*
 * Copyright (c) 2011, Julian Rother <julian@toksik.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "edraw.h"
#include "debug.h"
#include "internal.h"

#include <stdlib.h>

int edraw_set_grayscale(edraw_surface *sr, char gs)
{
  edraw_called("edraw_set_grayscale");
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_set_grayscale", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  if (sr->state->colormode == EDRAW_MONO)
  {
#if (EDRAW_COLOR == EDRAW_MONO)
    if (gs > 1)
      gs = 1;
    sr->state->gs = gs;
#elif (EDRAW_COLOR == EDRAW_GRAY4)
    if (gs >= 1)
      sr->state->gs = 15;
    else
      sr->state->gs = 0;
#elif (EDRAW_COLOR == EDRAW_GRAY)
    if (gs >= 1)
      sr->state->gs = 255;
    else
      sr->state->gs = 0;
#elif (EDRAW_COLOR == EDRAW_TRUECOLOR)
    if (gs >= 1)
      sr->state->r = sr->state->g = sr->state->b = 255;
    else
      sr->state->r = sr->state->g = sr->state->b = 0;
    sr->state->a = 255;
#endif
  }
  else if (sr->state->colormode == EDRAW_GRAY4)
  {
#if (EDRAW_COLOR == EDRAW_MONO)
    if (gs < 8)
      sr->state->gs = 0;
    else
      sr->state->gs = 1;
#elif (EDRAW_COLOR == EDRAW_GRAY4)
    if (gs >= 15)
      sr->state->gs = 15;
    else
      sr->state->gs = gs;
#elif (EDRAW_COLOR == EDRAW_GRAY)
    if (gs >= 15)
      sr->state->gs = 255;
    else
      sr->state->gs = gs*16;
#elif (EDRAW_COLOR == EDRAW_TRUECOLOR)
    sr->state->r = sr->state->g = sr->state->b = (gs * 255) / 16;
#endif
  }
  else if (sr->state->colormode == EDRAW_GRAY)
  {
#if (EDRAW_COLOR == EDRAW_MONO)
    if (gs < 128)
      sr->state->gs = 0;
    else
      sr->state->gs = 1;
#elif (EDRAW_COLOR == EDRAW_GRAY4)
    if (gs >= 255)
      sr->state->gs = 15;
    else
      sr->state->gs = (gs*16)/255;
#elif (EDRAW_COLOR == EDRAW_GRAY)
    if (gs >= 255)
      sr->state->gs = 255;
    else
      sr->state->gs = gs;
#elif (EDRAW_COLOR == EDRAW_TRUECOLOR)
    sr->state->r = sr->state->g = sr->state->b = gs;
#endif
  }
  else if (sr->state->colormode == EDRAW_TRUECOLOR)
    return(EDRAW_INVAL);
  return(EDRAW_SUCCESS);
}

int edraw_set_rgba(edraw_surface *sr, char r, char g, char b, char a)
{
  edraw_called("edraw_set_rgba");
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_set_rgba", "Surface not ready");
    return(EDRAW_SURFACE);
  }
  if (sr->state->colormode == EDRAW_MONO)
    return(EDRAW_INVAL);
  else if (sr->state->colormode == EDRAW_GRAY4)
    return(EDRAW_INVAL);
  else if (sr->state->colormode == EDRAW_TRUECOLOR)
  {
#if (EDRAW_COLOR == EDRAW_MONO)
    if (edraw_rgba2gs16(r, g, b, a) < 8)
      sr->state->gs = 0;
    else
      sr->state->gs = 1;
#elif (EDRAW_COLOR == EDRAW_GRAY4)
    sr->state->gs = edraw_rgba2gs16(r, g, b, a);
#elif (EDRAW_COLOR == EDRAW_GRAY)
    sr->state->gs = edraw_rgba2gs(r, g, b, a);
#elif (EDRAW_COLOR == EDRAW_TRUECOLOR)
    sr->state->r = r;
    sr->state->g = g;
    sr->state->b = b;
    sr->state->a = a;
#endif
  }
  return(EDRAW_SUCCESS);
}


int edraw_clip(edraw_surface *sr, int x, int y, unsigned int width,
    unsigned int height)
{
  edraw_called("edraw_clip");
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_clip", "Surface not ready");
    return(EDRAW_SURFACE);
  }
  if (x < 0)
    sr->state->clip_x = 0;
  else if (x > sr->width)
    sr->state->clip_x = sr->width;
  else
    sr->state->clip_x = x;
  if (y < 0)
    sr->state->clip_y = 0;
  else if (y > sr->height)
    sr->state->clip_y = sr->height;
  else
    sr->state->clip_y = y;
  if (x + width > sr->width)
    sr->state->clip_width = 0;
  else
    sr->state->clip_width = width;
  if (y + height > sr->height)
    sr->state->clip_height = 0;
  else
    sr->state->clip_height = height;
  return(EDRAW_SUCCESS);
}

void edraw_mark_dirty(edraw_surface *sr, int x1, int y1, int x2, int y2)
{
  if (sr->flags & EDRAW_UPDATE)
  {
    sr->state->upd_x1 = MIN(sr->state->upd_x1, x1);
    sr->state->upd_y1 = MIN(sr->state->upd_y1, y1);
    sr->state->upd_x2 = MAX(sr->state->upd_x2, x2);
    sr->state->upd_y2 = MAX(sr->state->upd_y2, y2);
  }
  else
  {
    sr->state->upd_x1 = x1;
    sr->state->upd_y1 = y1;
    sr->state->upd_x2 = x2;
    sr->state->upd_y2 = y2;
    sr->flags |= EDRAW_UPDATE;
  }
}

int edraw_save(edraw_surface *sr)
{
  edraw_called("edraw_save");
  edraw_state *state;
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_save", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  if ((state = malloc(sizeof(edraw_state))) == NULL)
  {
    edraw_fatal("edraw_save", "Out of memory");
    return(-EDRAW_MEMORY);
  }
  memcpy(state, sr->state, sizeof(edraw_state));
  if ((state->ops = malloc(10 * sizeof(edraw_operation))) == NULL)
  {
    edraw_fatal("edraw_save", "Out of memory");
    return(-EDRAW_MEMORY);
  }
  state->op_len = 10;
  state->op_count = 0;
  state->prev = sr->state;
  sr->state = state;
  return(EDRAW_SUCCESS);
}

int edraw_restore(edraw_surface *sr)
{
  edraw_called("edraw_restore");
  edraw_state *state;
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_restore", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  state = sr->state;
  sr->state = state->prev;
  sr->state->upd_x1 = state->upd_x1;
  sr->state->upd_y1 = state->upd_y1;
  sr->state->upd_x2 = state->upd_x2;
  sr->state->upd_y2 = state->upd_y2;
  free(state->ops);
  free(state);
  return(EDRAW_SUCCESS);
}

int edraw_set_colormode(edraw_surface *sr, int mode)
{
  edraw_called("edraw_set_colormode");
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_set_colormode", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  sr->state->colormode = mode;
  return(EDRAW_SUCCESS);
}
