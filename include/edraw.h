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

int edraw_create(edraw_surface *sr, int colormode);
int edraw_create_image(edraw_surface *sr, int color, unsigned int width,
    unsigned int height);
int edraw_create_einkfb(edraw_surface *sr, int color, char *fb_path);
int edraw_destroy(edraw_surface *sr);

int edraw_save(edraw_surface *sr);
int edraw_restore(edraw_surface *sr);

int edraw_copy(edraw_surface *target, edraw_surface *src, int x1, int y1,
    unsigned int width, unsigned int height, int x2, int y2);

int edraw_clip(edraw_surface *sr, int x, int y, unsigned int width,
    unsigned int height);

int edraw_set_colormode(edraw_surface *sr, int mode);
int edraw_set_grayscale(edraw_surface *sr, char gs);
int edraw_set_rgba(edraw_surface *sr, char r, char g, char b, char a);

int edraw_mark_dirrty(edraw_surface *sr, int x1, int y1, int x2, int y2);
int edraw_update(edraw_surface *sr);
int edraw_fullupdate(edraw_surface *sr);
int edraw_blank(edraw_surface *sr);
int edraw_clear(edraw_surface *sr);

void edraw_dot(edraw_surface *sr, int x, int y);

int edraw_move_to(edraw_surface *sr, int x, int y);
int edraw_line_to(edraw_surface *sr, int x, int y);
int edraw_rectangle(edraw_surface *sr, int x, int y, unsigned int width,
    unsigned int height);

#endif
