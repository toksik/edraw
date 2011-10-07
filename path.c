#include "edraw.h"
#include "debug.h"
#include "internal.h"

#include <stdlib.h>

int edraw_move_to(edraw_surface *sr, int x, int y)
{
  edraw_called("edraw_move_to");
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_move_to", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  sr->state->path_x = x;
  sr->state->path_y = y;
  return(EDRAW_SUCCESS);
}

edraw_operation *edraw_addop(edraw_surface *sr)
{
  edraw_called("edraw_addop");
  if (sr->state->ops == NULL)
  {
    edraw_warn("edraw_addop", "Update array is not initialised");
    return(NULL);
  }
  if (sr->state->op_count >= sr->state->op_len)
  {
    sr->state->op_len += 5;
    if ((sr->state->ops = realloc(sr->state->ops, sr->state->op_len
            * sizeof(edraw_operation))) == NULL)
    {
      sr->state->op_len = 0;
      sr->state->op_count = 0;
      edraw_warn("edraw_addop", "Memory allocation failed");
      return(NULL);
    }
  }
  return(&sr->state->ops[sr->state->op_count ++]);
}

int edraw_line_to(edraw_surface *sr, int x, int y)
{
  edraw_called("edraw_line_to");
  edraw_operation *op;
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_line_to", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  if ((op = edraw_addop(sr)) == NULL)
  {
    sr->state->path_x = x;
    sr->state->path_y = y;
    return(EDRAW_MEMORY);
  }
  op->type = EDRAW_LINE;
  op->flags = 0;
  if (DELTA(x, sr->state->path_x) >= DELTA(y, sr->state->path_y))
  {
    if (x < sr->state->path_x)
    {
      op->x = x;
      op->y = y;
      op->x2 = sr->state->path_x;
      op->y2 = sr->state->path_y;
    }
    else
    {
      op->x = sr->state->path_x;
      op->y = sr->state->path_y;
      op->x2 = x;
      op->y2 = y;
    }
    op->gradient = ((op->y2 - op->y) * 1000) / DELTA(op->x, op->x2);
    op->start = (((op->gradient * op->x) / 1000) * -1) + op->y;
  }
  else
  {
    if (y < sr->state->path_y)
    {
      op->x = x;
      op->y = y;
      op->x2 = sr->state->path_x;
      op->y2 = sr->state->path_y;
    }
    else
    {
      op->x = sr->state->path_x;
      op->y = sr->state->path_y;
      op->x2 = x;
      op->y2 = y;
    }
    op->gradient = ((op->x2 - op->x) * 1000) / DELTA(op->y, op->y2);
    op->start = (((op->gradient * op->y) / 1000) * -1) + op->x;
    op->flags |= EDRAW_YGRADIENT;
  }
  sr->state->path_x = x;
  sr->state->path_y = y;
  return(EDRAW_SUCCESS);
}

int edraw_rectangle(edraw_surface *sr, int x, int y, unsigned int width,
    unsigned int height)
{
  edraw_called("edraw_rectangle");
  edraw_operation *op;
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_rectangle", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  if ((op = edraw_addop(sr)) == NULL)
  {
    sr->state->path_x = x + (width / 2);
    sr->state->path_y = y + (height / 2);
    return(EDRAW_MEMORY);
  }
  op->type = EDRAW_RECTANGLE;
  op->x = x;
  op->y = y;
  op->width = width;
  op->height = height;
  sr->state->path_x = x + (width / 2);
  sr->state->path_y = y + (height / 2);
  return(EDRAW_SUCCESS);
}

int edraw_stroke(edraw_surface *sr)
{
  edraw_called("edraw_stroke");
  int i, x, y, max;
  edraw_operation *op;
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_stroke", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  for (i = 0; i < sr->state->op_count; i ++)
  {
    op = &sr->state->ops[i];
    if (op->type == EDRAW_LINE)
      if (op->flags & EDRAW_YGRADIENT)
      {
        max = MAX(op->y, op->y2);
        for (y = MIN(op->y, op->y2); y < max; y ++)
          edraw_dot(sr, ((op->gradient * y) / 1000) + op->start, y);
        edraw_mark_dirty(sr, MIN(op->x, op->x2), MIN(op->y, op->y2),
            DELTA(op->x, op->x2), DELTA(op->y, op->y2)+1);
      }
      else
      {
        max = MAX(op->x, op->x2);
        for (x = MIN(op->x, op->x2); x < max; x ++)
          edraw_dot(sr, x, ((op->gradient * x) / 1000) + op->start);
        edraw_mark_dirty(sr, MIN(op->x, op->x2), MIN(op->y, op->y2),
            DELTA(op->x, op->x2), DELTA(op->y, op->y2)+1);
      }
    else if (op->type == EDRAW_RECTANGLE)
    {
     for (x = op->x; x < op->x + op->width; x ++)
     {
       edraw_dot(sr, x, op->y);
       edraw_dot(sr, x, op->y + op->height);
     }
     for (y = op->y; y < op->y + op->height; y ++)
     {
       edraw_dot(sr, op->x, y);
       edraw_dot(sr, op->x + op->width, y);
     }
     edraw_mark_dirty(sr, op->x, op->y, op->x + op->width, op->y + op->height);
    }
    else
      edraw_debug("edraw_stroke", "Unknown operation");
  }
  sr->state->op_count = 0;
  return(EDRAW_SUCCESS);
}

int edraw_compar(const void *_a, const void *_b)
{
  int a, b;
  a = *((int *)_a);
  b = *((int *)_b);
  if (a < b)
    return(-1);
  else if (a == b)
    return(0);
  else
    return(1);
}

#define EDRAW_BUFFAPPEND(num)                                   \
{                                                               \
  if (buff_count >= buff_len)                                   \
  {                                                             \
    buff_len += 10;                                             \
    if ((buff = realloc(buff, sizeof(int) * buff_len)) == NULL) \
    {                                                           \
      edraw_warn("edraw_get_isecs", "Out of memory");           \
      return(0);                                                \
    }                                                           \
  }                                                             \
  buff[buff_count++] = (num);                                   \
}
int edraw_get_isecs(edraw_operation *ops, int count, int y, int **res)
{
  edraw_called("edraw_get_isecs");
  int i, *buff, buff_len, buff_count;
  edraw_operation *op;
  if ((buff = malloc(sizeof(int) * 10)) == NULL)
  {
    edraw_warn("edraw_get_isecs", "Out of memory");
    return(0);
  }
  buff_len = 10;
  buff_count = 0;
  for (i = 0; i < count; i ++)
  {
    op = &ops[i];
    if ((op->type == EDRAW_LINE) && (MIN(op->y, op->y2) <= y) &&
        (MAX(op->y, op->y2) >= y))
      if (op->flags & EDRAW_YGRADIENT)
      {
        EDRAW_BUFFAPPEND(((op->gradient * y) / 1000) + op->start);
      }
      else
      {
        if (op->y == op->y2)
        {
          EDRAW_BUFFAPPEND(op->x);
          EDRAW_BUFFAPPEND(op->x2);
        }
        else
        {
          EDRAW_BUFFAPPEND(((y - op->start) * 1000) / op->gradient);
        }
      }
    else
      edraw_debug("edraw_get_isecs", "Ignoring operation");
  }
  if (buff_count == 0)
    return(0);
  qsort(buff, buff_count, sizeof(int), edraw_compar);
  *res = buff;
  return(buff_count);
}

int edraw_fill(edraw_surface *sr)
{
  edraw_called("edraw_fill");
  int i, *buff, len;
  int x, _x, y, x1, y1, x2, y2, state;
  edraw_operation *op;
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_fill", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  if (sr->state->op_count == 0)
  {
    edraw_debug("edraw_fill", "Operation buffer empty");
    return(EDRAW_NOOP);
  }
  for (i = 0; i < sr->state->op_count; i ++)
  {
    op = &sr->state->ops[i];
    if (op->type == EDRAW_LINE)
    {
      if (i == 0)
      {
        x1 = MIN(op->x, op->x2);
        y1 = MIN(op->y, op->y2);
        x2 = MAX(op->x, op->x2);
        y2 = MAX(op->y, op->y2);
        continue;
      }
      x1 = MIN(x1, MIN(op->x, op->x2));
      y1 = MIN(y1, MIN(op->y, op->y2));
      x2 = MAX(x2, MAX(op->x, op->x2));
      y2 = MAX(y2, MAX(op->y, op->y2));
    }
    else
    {
      if (i == 0)
      {
        y1 = op->y;
        y2 = op->y+op->height;
      }
      y1 = MIN(y1, op->y);
      y2 = MAX(y2, op->y+op->height);
    }
  }
  edraw_mark_dirty(sr, x1, y1, x2, y2);
  buff = NULL;
  x = 0;
  for (y = y1; y <= y2; y ++)
  {
    free(buff);
    len = edraw_get_isecs(sr->state->ops, sr->state->op_count, y, &buff);
    state = 0;
    for (i = 0; i < len; i ++)
      if (state == 0)
      {
        _x = buff[i];
        state = 1;
      }
      else
      {
        if (buff[i] == _x)
          continue;
        for (x = _x; x <= buff[i]; x ++)
          edraw_dot(sr, x, y);
        state = 0;
      }
  }
  return(EDRAW_SUCCESS);
}

