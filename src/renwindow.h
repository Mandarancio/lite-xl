#include <SDL.h>
#include "renderer.h"

struct RenWindow {
  SDL_Window *window;
  uint8_t *command_buf;
  size_t command_buf_idx;
  size_t command_buf_size;
  SDL_Renderer *renderer;
  RenSurface rensurface;
};
typedef struct RenWindow RenWindow;

void renwin_init_surface(RenWindow *ren);
void renwin_init_command_buf(RenWindow *ren);
void renwin_clip_to_surface(RenWindow *ren);
void renwin_set_clip_rect(RenWindow *ren, RenRect rect);
void renwin_resize_surface(RenWindow *ren);
void renwin_show_window(RenWindow *ren);
void renwin_update(RenWindow *ren);
void renwin_free(RenWindow *ren);
RenSurface renwin_get_surface(RenWindow *ren);

