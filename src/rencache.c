#include "SDL_blendmode.h"
#include "renderer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#ifdef _MSC_VER
#ifndef alignof
#define alignof _Alignof
#endif
/* max_align_t is a compiler defined type, but
** MSVC doesn't provide it, so we'll have to improvise */
typedef long double max_align_t;
#else
#include <stdalign.h>
#endif

#include "rencache.h"
#include "renwindow.h"
#include <lauxlib.h>

/* a cache over the software renderer -- all drawing operations are stored as
** commands when issued. At the end of the frame we write the commands to a grid
** of hash values, take the cells that have changed since the previous frame,
** merge them into dirty rectangles and redraw only those regions */

#define CELLS_X 80
#define CELLS_Y 50
#define CELL_SIZE 96
#define CMD_BUF_RESIZE_RATE 1.2
#define CMD_BUF_INIT_SIZE (1024 * 512)
#define COMMAND_BARE_SIZE offsetof(Command, command)

enum CommandType { SET_CLIP, DRAW_TEXT, DRAW_RECT };


static bool resize_issue;
static RenRect screen_rect;
static RenRect last_clip_rect;
static bool show_debug ;


/* 32bit fnv-1a hash */
#define HASH_INITIAL 2166136261



void rencache_show_debug(bool enable) { show_debug = enable; }


void rencache_set_clip_rect(RenWindow *window_renderer, RenRect rect) {
  ren_set_clip_rect(window_renderer, rect);
}


void rencache_draw_rect(RenWindow *window_renderer, RenRect rect,
                        RenColor color) {
  ren_draw_rect(&window_renderer->rensurface, rect, color);
}

double rencache_draw_text(RenWindow *window_renderer, RenFont **fonts,
                          const char *text, size_t len, double x, int y,
                          RenColor color) {
  return ren_draw_text(&window_renderer->rensurface, fonts, text, len, x, y,
                       color);
}

void rencache_invalidate(void) {  }

void rencache_begin_frame(RenWindow *window_renderer) {
  /* reset all cells if the screen width/height has changed */
  int w, h;
  resize_issue = false;
  ren_get_size(window_renderer, &w, &h);
  if (screen_rect.width != w || h != screen_rect.height) {
    screen_rect.width = w;
    screen_rect.height = h;
    rencache_invalidate();
  }
  last_clip_rect = screen_rect;
  SDL_SetRenderDrawColor(window_renderer->renderer, 0, 0, 0, 255);
  SDL_RenderClear(window_renderer->renderer);
  SDL_SetRenderDrawBlendMode(window_renderer->renderer, SDL_BLENDMODE_BLEND);
}


void rencache_end_frame(RenWindow *window_renderer) {
  ren_update_rects(window_renderer);
}
