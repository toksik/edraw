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

#ifndef __EDRAW_EDRAW_H
#define __EDRAW_EDRAW_H
/* Values for EDRAW_DEVICE */
#define EINKFB 1
/* Values for EDRAW_COLOR */
#define EDRAW_MONO 0 /* 1-bit grayscale (monocrome) 1->black 0->white */
#define EDRAW_GRAY4 1 /* 4-bit grayscale (16 levels) */
#define EDRAW_GRAY 2 /* 8-bit grayscale (256 levels) */
#define EDRAW_TRUECOLOR 3 /* 32-bit rgba: "True Color" with alpha channel */

struct edraw_surface;
typedef struct edraw_surface edraw_surface;
struct edraw_state;
typedef struct edraw_state edraw_state;
struct edraw_device;
typedef struct edraw_device edraw_device;

struct edraw_surface
{
  unsigned int width;
  unsigned int height;
  unsigned int flags;
#define EDRAW_READY 1
#define EDRAW_UPDATE 2
#define EDRAW_BUFFER 3 /* Surface is an image buffer */
  edraw_device *dev;
  edraw_state *state;
  char *ptr;
};

struct edraw_image;
typedef struct edraw_image edraw_image;

/* Return values */
#define EDRAW_SUCCESS 0
#define EDRAW_INVAL 1
#define EDRAW_MEMORY 2
#define EDRAW_SURFACE 3
#define EDRAW_ERRNO 4
#define EDRAW_NOOP 5

/*
 * Initialises a surface connected to the device the library is compiled for
 */
int edraw_create(edraw_surface *sr, int colormode);

/*
 * Initialises a surface for drawing into an image buffer
 */
int edraw_create_image(edraw_surface *sr, int color, unsigned int width,
    unsigned int height);

/*
 * Initialises an einkfb device surface for the framebuffer 'fb_path'
 */
int edraw_create_einkfb(edraw_surface *sr, int color, char *fb_path);

/*
 * Destroys a surface
 */
int edraw_destroy(edraw_surface *sr);

/*
 * Saves or restores the drawing status (like path operations, colormode, etc)
 */
int edraw_save(edraw_surface *sr);
int edraw_restore(edraw_surface *sr);

/*
 * Copys the area specified by x1, y1, width and height from src to x2, y2 into
 * target.
 */
int edraw_copy(edraw_surface *target, edraw_surface *src, int x1, int y1,
    unsigned int width, unsigned int height, int x2, int y2);

/*
 * Sets the clipping area (the area that drawing operations have effect to)
 */
int edraw_clip(edraw_surface *sr, int x, int y, unsigned int width,
    unsigned int height);

/*
 * Sets the colormode to use
 *
 * The colormode is used to convert the values defined by edraw_set_grayscale
 * and edraw_set_rgba to the color used by the surface device.
 */
int edraw_set_colormode(edraw_surface *sr, int mode);

/*
 * Sets the gray level for drawing
 *
 * colormode==EDRAW_GRAY4
 *   Values between 0 and 15 (0->white, 15->black)
 *
 * colormode==EDRAW_GRAY
 *   Values between 0 and 255 (0->white, 255->black)
 */
int edraw_set_grayscale(edraw_surface *sr, char gs);

/*
 * Only works partially at the moment
 */
int edraw_set_rgba(edraw_surface *sr, char r, char g, char b, char a);

/*
 * Extents the area that needs to get updated
 *
 * All of the built-in drawing functions do this automatically (except of
 * edraw_dot).
 */
void edraw_mark_dirty(edraw_surface *sr, int x1, int y1, int x2, int y2);

/*
 * Updates the area marked for update
 */
int edraw_update(edraw_surface *sr);

/*
 * Forces a full update of the screen
 */
int edraw_fullupdate(edraw_surface *sr);

/*
 * Blanks the display
 *
 * This does not mean that the surface will be white after calling it. Instead
 * it is needed for really clearing epaper/eink displays. So it may do nothing
 * on some surfaces.
 */
int edraw_blank(edraw_surface *sr);

/*
 * Draws the current color over the currently clipped area
 */
int edraw_clear(edraw_surface *sr);

/*
 * Draws a single dot at x,y with the current color
 *
 * Note that this does not mark anything for update
 */
void edraw_dot(edraw_surface *sr, int x, int y);

/*
 * Some path operations
 *
 * All these functions do not modify the image data on the surface. Call
 * edraw_stroke or edraw_fill to actually draw it.
 */
int edraw_move_to(edraw_surface *sr, int x, int y);
int edraw_line_to(edraw_surface *sr, int x, int y);
int edraw_rectangle(edraw_surface *sr, int x, int y, unsigned int width,
    unsigned int height);

/*
 * Draws the outline of the current path onto the surface
 */
int edraw_stroke(edraw_surface *sr);

/*
 * Fills the closed parts of the current path and draws it onto the surface
 */
int edraw_fill(edraw_surface *sr);

#endif
