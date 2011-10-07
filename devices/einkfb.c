#define EDRAW_COLOR EDRAW_GRAY4
#include "edraw.h"
#include "debug.h"
#include "internal.h"

#include <linux/fb.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Extracted from linux/einkfb.h */
struct update_area_t
{
  int x1, y1, x2, y2;
  int fx;
  void *buff;
};
typedef struct update_area_t update_area_t;

#define FBIO_EINK_UPDATE_DISPLAY 0x46db
#define FBIO_EINK_UPDATE_DISPLAY_AREA 0x46dd
/* * * * * * * * * * * * * * * * */

#ifndef EDRAW_FBPATH
#define EDRAW_FBPATH "/dev/fb0"
#endif

struct edraw_device
{
  int fd;
  int size;
};

int edraw_create(edraw_surface *sr, int color)
{
  edraw_called("einkfb:edraw_create");
  return edraw_create_einkfb(sr, color, EDRAW_FBPATH);
}

int edraw_create_einkfb(edraw_surface *sr, int color, char *fbpath)
{
  edraw_called("einkfb:edraw_create_einkfb");
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
  sr->flags = 0;
  if ((sr->dev = malloc(sizeof(edraw_device))) == NULL)
  {
    edraw_fatal("edraw_create_einkfb", "Out of memory");
    return(-EDRAW_MEMORY);
  }
  if ((sr->dev->fd = open(fbpath, O_RDWR)) == -1)
  {
    edraw_fatal("edraw_create_einkfb", "Cannot open framebuffer device");
    return(-EDRAW_ERRNO);
  }
  if (ioctl(sr->dev->fd, FBIOGET_FSCREENINFO, &finfo) == -1)
  {
    close(sr->dev->fd);
    edraw_fatal("edraw_create_einkfb", "Error while getting fix screen \
        information");
    return(-EDRAW_ERRNO);
  }
  if (ioctl(sr->dev->fd, FBIOGET_VSCREENINFO, &vinfo) == -1)
  {
    close(sr->dev->fd);
    edraw_fatal("edraw_create_einkfb", "Error while getting variable screen \
        information");
    return(-EDRAW_ERRNO);
  }
  sr->dev->size = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
  sr->width = vinfo.xres;
  sr->height = vinfo.yres;
  sr->ptr = (char *) mmap(0, sr->dev->size, PROT_READ | PROT_WRITE,
      MAP_SHARED, sr->dev->fd, 0);
  if (sr->ptr == MAP_FAILED)
  {
    close(sr->dev->fd);
    edraw_fatal("edraw_create_einkfb",
        "Mapping framebuffer into memory failed");
    return(-EDRAW_ERRNO);
  }
  EDRAW_INIT_SURFACE(sr);
  EDRAW_INIT_COLOR(sr->state);
  sr->state->colormode = color;
  sr->flags |= EDRAW_READY;
  return(EDRAW_SUCCESS);
}

int edraw_destroy(edraw_surface *sr)
{
  edraw_called("einkfb:edraw_destroy");
  edraw_state *state, *_state;
  if (sr->flags & EDRAW_BUFFER)
    free(sr->ptr);
  else
  {
    if (sr->dev != NULL)
    {
      munmap(sr->ptr, sr->dev->size);
      sr->ptr = NULL;
      close(sr->dev->fd);
      sr->dev->fd = -1;
    }
    free(sr->dev);
  }
  sr->flags = 0;
  state = sr->state;
  while (state != NULL)
  {
    free(sr->state->ops);
    _state = state;
    state = state->prev;
    free(_state);
  }
  return(EDRAW_SUCCESS);
}

int edraw_update(edraw_surface *sr)
{
  edraw_called("einkfb:edraw_update");
  update_area_t area;
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_update", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  if (sr->dev == NULL)
  {
    edraw_fatal("edraw_update", "Surface is not a device");
    return(-EDRAW_INVAL);
  }
  if (~sr->flags & EDRAW_UPDATE)
  {
    edraw_debug("edraw_update", "No update needed");
    return(EDRAW_NOOP);
  }
  area.x1 = sr->state->upd_x1;
  area.y1 = sr->state->upd_y1;
  area.x2 = sr->state->upd_x2;
  area.y2 = sr->state->upd_y2;
  area.fx = 0;
  area.buff = NULL;
  if (ioctl(sr->dev->fd, FBIO_EINK_UPDATE_DISPLAY_AREA, &area) == -1)
  {
    edraw_warn("edraw_update", "Updating display failed");
    return(EDRAW_ERRNO);
  }
  sr->flags &= ~EDRAW_UPDATE;
  return(EDRAW_SUCCESS);
}

int edraw_fullupdate(edraw_surface *sr)
{
  edraw_called("einkfb:edraw_fullupdate");
  int fd;
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_fullupdate", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  if (sr->dev == NULL)
  {
    edraw_fatal("edraw_fullupdate", "Surface is not a device");
    return(-EDRAW_INVAL);
  }
  if (~sr->flags & EDRAW_UPDATE)
  {
    edraw_debug("edraw_fullupdate", "No update needed");
    return(EDRAW_NOOP);
  }
  if (ioctl(sr->dev->fd, FBIO_EINK_UPDATE_DISPLAY, 1) == -1)
  {
    edraw_warn("edraw_fullupdate", "Updating display failed");
    return(EDRAW_ERRNO);
  }
  sr->flags &= ~EDRAW_UPDATE;
  return(EDRAW_SUCCESS);
}

int edraw_blank(edraw_surface *sr)
{
  edraw_called("einkfb:edraw_blank");
  if (~sr->flags & EDRAW_READY)
  {
    edraw_fatal("edraw_blank", "Surface not ready");
    return(-EDRAW_SURFACE);
  }
  if (sr->dev == NULL)
  {
    edraw_fatal("edraw_blank", "Surface is not a device");
    return(-EDRAW_INVAL);
  }
  if (ioctl(sr->dev->fd, FBIOBLANK, 1) == -1)
  {
    edraw_warn("edraw_blank", "Blanking display failed");
    return(EDRAW_ERRNO);
  }
  return(EDRAW_SUCCESS);
}
#define __EDRAW_HAS_BLANK

#include "common.h"
