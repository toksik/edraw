#ifndef __EDRAW_INTERNAL_H
#define __EDRAW_INTERNAL_H
#include "edraw.h"

struct edraw_operation;
typedef struct edraw_operation edraw_operation;

struct edraw_operation
{
  char type;
#define EDRAW_LINE 0
#define EDRAW_RECTANGLE 1
  int flags;
#define EDRAW_YGRADIENT 1
  int x;
  int y;
  union
  {
    /* EDRAW_LINE */
    struct
    {
      int x2;
      int y2;
      /* f(x) = gradient * x + start */
      int gradient; /* gradient * 1000 to avoid use of floats */
      int start;
    };
    /* EDRAW_RECTANGLE */
    struct
    {
      int width;
      int height;
    };
  };
};

struct edraw_state
{
  int colormode;
  int clip_x;
  int clip_y;
  unsigned int clip_width;
  unsigned int clip_height;
  int path_x;
  int path_y;
  int upd_x1;
  int upd_y1;
  int upd_x2;
  int upd_y2;
  edraw_operation *ops;
  int op_len;
  int op_count;
#if (EDRAW_COLOR == EDRAW_TRUECOLOR)
  char r, g, b, a;
#elif (EDRAW_COLOR == EDRAW_MONO) || (EDRAW_COLOR == EDRAW_GRAY4) || \
  (EDRAW_COLOR == EDRAW_GRAY)
  char gs;
#endif
  edraw_state *prev;
};

#define DELTA(a, b) ((a) < (b) ? ((b)-(a)) : ((a)-(b)))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define EDRAW_INIT_SURFACE(sr)                                         \
{                                                                      \
  sr->flags = 0;                                                       \
  if ((sr->state = malloc(sizeof(edraw_state))) == NULL)               \
  {                                                                    \
    edraw_fatal("EDRAW_INIT_SURFACE", "Out of memory");                \
    return(-EDRAW_MEMORY);                                             \
  }                                                                    \
  sr->state->colormode = EDRAW_MONO;                                          \
  sr->state->clip_x = 0;                                               \
  sr->state->clip_y = 0;                                               \
  sr->state->clip_width = sr->width;                                   \
  sr->state->clip_height = sr->height;                                 \
  sr->state->path_x = 0;                                               \
  sr->state->path_y = 0;                                               \
  sr->state->upd_x1 = 0;                                               \
  sr->state->upd_y1 = 0;                                               \
  sr->state->upd_x2 = 0;                                               \
  sr->state->upd_y2 = 0;                                               \
  if ((sr->state->ops = malloc(10 * sizeof(edraw_operation))) == NULL) \
  {                                                                    \
    edraw_fatal("EDRAW_INIT_SURFACE", "Out of memory");                \
    return(-EDRAW_MEMORY);                                             \
  }                                                                    \
  sr->state->op_len = 10;                                              \
  sr->state->op_count = 0;                                             \
  sr->state->prev = NULL;                                              \
}

#if (EDRAW_COLOR == EDRAW_TRUECOLOR)
# define EDRAW_INIT_COLOR(state)                             \
{                                                            \
  ((state)->r = (state)->g = (state)->b = (state)->a = 255); \
}
#elif (EDRAW_COLOR == EDRAW_MONO)
# define EDRAW_INIT_COLOR(state) ((state)->gs = 1)
#elif (EDRAW_COLOR == EDRAW_GRAY4)
# define EDRAW_INIT_COLOR(state) ((state)->gs = 15)
#elif (EDRAW_COLOR == EDRAW_GRAY)
# define EDRAW_INIT_COLOR(state) ((state)->gs = 255)
#endif

#define edraw_rgba2gs16(r, g, b, a) (0)
#define edraw_rgba2gs(r, g, b, a) (0) // fixme: implement it

#endif

