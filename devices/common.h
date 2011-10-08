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

int edraw_create_image(edraw_surface *sr, int color, unsigned int width,
    unsigned int height)
{
  edraw_called("edraw_create_image");
  sr->flags = 0;
  sr->dev = NULL;
#if (EDRAW_COLOR == EDRAW_MONO)
  if ((sr->ptr = malloc((width * height) / 8)) == NULL)
#elif (EDRAW_COLOR == EDRAW_GRAY4)
  if ((sr->ptr = malloc((width * height) / 2)) == NULL)
#elif (EDRAW_COLOR == EDRAW_GRAY)
  if ((sr->ptr = malloc(width * height)) == NULL)
#elif (EDRAW_COLOR == EDRAW_TRUECOLOR)
  if ((sr->ptr = malloc((width * height) * 4)) == NULL)
#endif
  {
    edraw_fatal("edraw_create_image", "Out of memory");
    return(-EDRAW_MEMORY);
  }
  sr->width = width;
  sr->height = height;
  EDRAW_INIT_SURFACE(sr);
  EDRAW_INIT_COLOR(sr->state);
  sr->state->colormode = color;
  sr->flags |= EDRAW_READY;
  return(EDRAW_SUCCESS);
}

#ifndef __EDRAW_HAS_DOT
#define __EDRAW_HAS_DOT
#if (EDRAW_COLOR == EDRAW_GRAY4)
void edraw_dot(edraw_surface *sr, int x, int y)
{
  char *byte;
  int offset;
  int gs;
  gs = sr->state->gs;
  if ((x < sr->state->clip_x) || (y < sr->state->clip_y) ||
      (x >= sr->state->clip_width+sr->state->clip_x) ||
      (y >= sr->state->clip_height+sr->state->clip_y))
    return;
  offset = x * 0.5;
  byte = &(sr->ptr[((y * sr->width) / 2) + offset]);
  if (x % 2 == 0)
    *byte = (gs << 4) | (*byte & 0b00001111);
  else
    *byte = gs | (*byte & 0b11110000);
}
#else
void edraw_dot(edraw_surface *sr, int x, int y)
{
  edraw_warn("edraw_dot", "Operation not implemented");
}
#endif
#endif

#ifndef __EDRAW_HAS_COPY
#define __EDRAW_HAS_COPY
#if (EDRAW_COLOR == EDRAW_GRAY4)
int edraw_copy(edraw_surface *target, edraw_surface *src, int x1, int y1,
    unsigned int width, unsigned int height, int x2, int y2)
{
  edraw_called("edraw_copy");
  char mod1, mod2;
  int x, y;
  int i, start1, start2, range, linelen1, linelen2, offset1, offset2;
  if ((~src->flags & EDRAW_READY) || (~target->flags & EDRAW_READY))
  {
    edraw_fatal("edraw_copy", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  if (x1 < 0)
  {
    width = width - (0 - x1);
    x1 = 0;
  }
  if ((x1 > src->width) || (y1 > src->height) || (x2 > target->state->clip_x +
        target->state->clip_width) || (y2 > target->state->clip_x +
          target->state->clip_width))
  {
    edraw_debug("edraw_copy", "Area is not on the surface");
    return(EDRAW_NOOP);
  }
  if (x1 + width > src->width)
    width = width - ((x + width) - src->width);
  if (y1 < 0)
  {
    height = height - (0 - y1);
    y1 = 0;
  }
  if (y1 + height > src->height)
    height = height - ((y + height) - src->height);
  if (x2 < target->state->clip_x)
  {
    width = width - (target->state->clip_x - x2);
    x1 = x1 + (target->state->clip_x - x2);
    x2 = target->state->clip_x;
  }
  if (x2 + width > target->state->clip_x + target->state->clip_width)
    width = width - ((x2 + width) - target->state->clip_x +
        target->state->clip_width);
  if (y2 < target->state->clip_y)
  {
    height = height - (target->state->clip_y - y2);
    y1 = y1 + (target->state->clip_y - y2);
    y2 = target->state->clip_y;
  }
  if (y2 + height > target->state->clip_x + target->state->clip_width)
  {
    height = height - ((y2 + height) - (target->state->clip_y +
        target->state->clip_height));
  }
  mod1 = ((y1 * src->width) + x1) % 2;
  mod2 = ((y2 * target->width) + x2) % 2;
  start1 = ((y1 * src->width) + x1) >> 1;
  start2 = ((y2 * target->width) + x2) >> 1;
  linelen1 = src->width >> 1 + src->width % 2;
  linelen2 = target->width >> 1 + target->width % 2;
  if (mod1 == mod2)
  {
    if (mod1 == 1)
      width --;
    range = width >> 1;
    for (y = 0; y < height; y ++)
    {
      offset1 = start1;
      offset2 = start2;
      if (mod1 == 1)
      {
        target->ptr[offset2] = (target->ptr[offset2] & 0b11110000) |
          (src->ptr[offset1] & 0b00001111);
        offset1 ++;
        offset2 ++;
      }
      for (i = 0; i < range; i ++)
        target->ptr[offset2 + i] = src->ptr[offset1 + i];
      if (width % 2)
        target->ptr[offset2 + i] = (src->ptr[offset1 + i] & 0b11110000) |
          (target->ptr[offset2 + i] & 0b00001111);
      start1 += linelen1;
      start2 += linelen2;
    }
  }
  else
  {
    range = width >> 1;
    for (y = 0; y < height; y ++)
    {
      offset1 = start1;
      offset2 = start2;
      if (mod2 == 1)
      {
        target->ptr[offset2] = (target->ptr[offset2] & 0b11110000) |
          ((src->ptr[offset1] >> 4) & 0b00001111);
        offset2 --;
      }
      for (i = 0; i < range; i ++)
      {
        target->ptr[offset2 + i] = ((src->ptr[offset1 + i] << 4) & 0b11110000)
          | ((src->ptr[offset1 + i + 1] >> 4) & 0b00001111);
      }
      if (mod1 == 1)
      {
        target->ptr[offset2 + i] = ((src->ptr[offset1 + i] << 4) & 0b11110000)
          | (target->ptr[offset2 + 1] & 0b00001111);
      }
      start1 += linelen1;
      start2 += linelen2;
    }
  }
  edraw_mark_dirty(target, x2, y2, x2+width, y2+height);
  return(EDRAW_SUCCESS);
}
#else
int edraw_copy(edraw_surface *target, edraw_surface *src, int x1, int y1,
    unsigned int width, unsigned int height, int x2, int y2)
{
  edraw_warn("edraw_copy", "Operation not implemented");
}
#endif
#endif

#ifndef __EDRAW_HAS_CLEAR
#define __EDRAW_HAS_CLEAR
#if (EDRAW_COLOR == EDRAW_GRAY4)
int edraw_clear(edraw_surface *sr)
{
  char value;
  int linelen, offset, x, y, width;
  int mod_start, mod_end;
  edraw_called("edraw_clear");
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_clear", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  linelen = sr->width >> 1;
  if (sr->width % 2)
    linelen ++;
  value = (sr->state->gs & 0b00001111) | ((sr->state->gs << 4) & 0b11110000);
  if ((sr->state->clip_x == 0) && (sr->state->clip_y == 0) &&
      (sr->state->clip_width == sr->width) &&
      (sr->state->clip_height == sr->height))
  {
    memset(sr->ptr, value, linelen * sr->height);
    edraw_mark_dirty(sr, sr->state->clip_x, sr->state->clip_y,
        sr->state->clip_x + sr->state->clip_width,
        sr->state->clip_y + sr->state->clip_height);
    return(EDRAW_SUCCESS);
  }
  width = sr->state->clip_width;
  offset = (linelen * sr->state->clip_y) + (sr->state->clip_x >> 1);
  mod_start = sr->state->clip_x % 2;
  if (mod_start)
  {
    width --;
    offset ++;
  }
  mod_end = width % 2;
  width = width >> 1;
  for (y = 0; y < sr->state->clip_height; y ++)
  {
    if (mod_start)
      sr->ptr[offset] = (sr->ptr[offset] & 0b11110000) | (value & 0b00001111);
    for (x = 0; x < width; x ++)
      sr->ptr[offset + x] = value;
    if (mod_end)
      sr->ptr[offset] = (value & 0b11110000) | (sr->ptr[offset] & 0b00001111);
    offset += linelen;
  }
  edraw_mark_dirty(sr, sr->state->clip_x, sr->state->clip_y,
      sr->state->clip_x + sr->state->clip_width,
      sr->state->clip_y + sr->state->clip_height);
  return(EDRAW_SUCCESS);
}
#else
int edraw_clear(edraw_surface *sr)
{
  edraw_called("edraw_clear");
  int x, y, width, height;
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_clear", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  width = sr->state->clip_x + sr->state->clip_width;
  height = sr->state->clip_y + sr->state->clip_height;
  for (y = sr->state->clip_y; y < height; y++)
    for (x = sr->state->clip_x; x < width; x++)
      edraw_dot(sr, x, y);
  edraw_mark_dirty(sr, sr->state->clip_x, sr->state->clip_y, width, height);
  return(EDRAW_SUCCESS);
}
#endif
#endif

#ifndef __EDRAW_HAS_BLANK
#define __EDRAW_HAS_BLANK
int edraw_blank(edraw_surface *sr)
{
  edraw_called("edraw_blank");
  edraw_debug("edraw_blank", "Blank stub");
  return(EDRAW_NOOP);
}
#endif

