#include "renwindow.h"
#include "SDL_blendmode.h"
#include "SDL_render.h"
#include <assert.h>
#include <stdio.h>

static int query_surface_scale(RenWindow *ren) {
  int w_pixels, h_pixels;
  int w_points, h_points;
  SDL_GL_GetDrawableSize(ren->window, &w_pixels, &h_pixels);
  SDL_GetWindowSize(ren->window, &w_points, &h_points);
  /* We consider that the ratio pixel/point will always be an integer and
     it is the same along the x and the y axis. */
  // assert(w_pixels % w_points == 0 && h_pixels % h_points == 0 && w_pixels /
  // w_points == h_pixels / h_points);
  return w_pixels / w_points;
}

static void setup_renderer(RenWindow *ren, int w, int h) {
  /* Note that w and h here should always be in pixels and obtained from
     a call to SDL_GL_GetDrawableSize(). */
  if (!ren->renderer) {
    ren->renderer =
        SDL_CreateRenderer(ren->window, -1, SDL_RENDERER_ACCELERATED);
  }
  ren->rensurface.scale = query_surface_scale(ren);
  SDL_SetRenderDrawBlendMode(ren->renderer, SDL_BLENDMODE_BLEND);
}

void renwin_init_surface(UNUSED RenWindow *ren) {
  if (ren->rensurface.surface) {
    SDL_FreeSurface(ren->rensurface.surface);
  }
  int w, h;
  SDL_GL_GetDrawableSize(ren->window, &w, &h);
  ren->rensurface.surface =
      SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA8888);
  if (!ren->rensurface.surface) {
    fprintf(stderr, "Error creating surface: %s", SDL_GetError());
    exit(1);
  }
  setup_renderer(ren, w, h);
}

void renwin_init_command_buf(RenWindow *ren) {
  ren->command_buf = NULL;
  ren->command_buf_idx = 0;
  ren->command_buf_size = 0;
}

static RenRect scaled_rect(const RenRect rect, const int scale) {
  return (RenRect){rect.x * scale, rect.y * scale, rect.width * scale,
                   rect.height * scale};
}

void renwin_clip_to_surface(RenWindow *ren) {
  SDL_SetClipRect(renwin_get_surface(ren).surface, NULL);
}

void renwin_set_clip_rect(RenWindow *ren, RenRect rect) {
  RenSurface rs = renwin_get_surface(ren);
  RenRect sr = scaled_rect(rect, rs.scale);
  SDL_SetClipRect(
      rs.surface,
      &(SDL_Rect){.x = sr.x, .y = sr.y, .w = sr.width, .h = sr.height});
}

RenSurface renwin_get_surface(RenWindow *ren) { return ren->rensurface; }

void renwin_resize_surface(UNUSED RenWindow *ren) {
  int new_w, new_h;
  SDL_GL_GetDrawableSize(ren->window, &new_w, &new_h);
  /* Note that (w, h) may differ from (new_w, new_h) on retina displays. */
  if (new_w != ren->rensurface.surface->w ||
      new_h != ren->rensurface.surface->h) {
    renwin_init_surface(ren);
    renwin_clip_to_surface(ren);
    setup_renderer(ren, new_w, new_h);
  }
}

void renwin_show_window(RenWindow *ren) { SDL_ShowWindow(ren->window); }

void renwin_update(RenWindow *ren) {
  SDL_RenderPresent(ren->renderer);
}

void renwin_free(RenWindow *ren) {
  SDL_DestroyRenderer(ren->renderer);
  SDL_FreeSurface(ren->rensurface.surface);
  SDL_DestroyWindow(ren->window);
  ren->window = NULL;
}
