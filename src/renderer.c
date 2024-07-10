#include "SDL_pixels.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include <assert.h>
#include <ft2build.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#include FT_OUTLINE_H
#include FT_SYSTEM_H

#ifdef _WIN32
#include "utfconv.h"
#include <windows.h>
#endif

#include "renderer.h"
#include "renwindow.h"

#include <hb-ft.h>
#include <hb.h>

#define MAX_UNICODE 0x100000
#define GLYPHSET_SIZE 8
#define MAX_LOADABLE_GLYPHSETS (MAX_UNICODE / GLYPHSET_SIZE)
#define SUBPIXEL_BITMAPS_CACHED 3

RenWindow window_renderer = {0};
static FT_Library library;

// draw_rect_surface is used as a 1x1 surface to simplify ren_draw_rect with
// blending
static SDL_Surface *draw_rect_surface;

static void *check_alloc(void *ptr) {
  if (!ptr) {
    fprintf(stderr, "Fatal error: memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

static const SDL_Color glyphPaletteColor[256] = {
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x00}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x01},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x02}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x03},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x04}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x05},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x06}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x07},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x08}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x09},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x0A}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x0B},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x0C}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x0D},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x0E}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x0F},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0x10}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x11},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x12}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x13},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x14}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x15},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x16}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x17},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x18}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x19},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x1A}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x1B},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x1C}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x1D},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x1E}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x1F},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0x00}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x21},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x22}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x23},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x24}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x25},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x26}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x27},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x28}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x29},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x2A}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x2B},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x2C}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x2D},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x2E}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x2F},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0x30}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x31},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x32}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x33},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x34}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x35},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x36}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x37},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x38}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x39},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x3A}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x3B},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x3C}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x3D},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x3E}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x3F},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0x40}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x41},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x42}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x43},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x44}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x45},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x46}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x47},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x48}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x49},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x4A}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x4B},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x4C}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x4D},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x4E}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x4F},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0x50}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x51},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x52}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x53},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x54}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x55},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x56}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x57},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x58}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x59},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x5A}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x5B},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x5C}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x5D},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x5E}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x5F},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0x60}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x61},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x62}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x63},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x64}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x65},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x66}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x67},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x68}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x69},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x6A}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x6B},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x6C}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x6D},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x6E}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x6F},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0x70}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x71},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x72}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x73},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x74}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x75},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x76}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x77},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x78}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x79},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x7A}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x7B},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x7C}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x7D},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x7E}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x7F},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0x80}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x81},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x82}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x83},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x84}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x85},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x86}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x87},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x88}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x89},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x8A}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x8B},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x8C}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x8D},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x8E}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x8F},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0x90}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x91},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x92}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x93},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x94}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x95},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x96}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x97},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x98}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x99},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x9A}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x9B},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x9C}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x9D},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0x9E}, (SDL_Color){0xFF, 0xFF, 0xFF, 0x9F},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0xA0}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xA1},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xA2}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xA3},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xA4}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xA5},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xA6}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xA7},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xA8}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xA9},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xAA}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xAB},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xAC}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xAD},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xAE}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xAF},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0xB0}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xB1},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xB2}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xB3},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xB4}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xB5},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xB6}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xB7},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xB8}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xB9},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xBA}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xBB},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xBC}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xBD},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xBE}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xBF},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0xC0}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xC1},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xC2}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xC3},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xC4}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xC5},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xC6}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xC7},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xC8}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xC9},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xCA}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xCB},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xCC}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xCD},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xCE}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xCF},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0xD0}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xD1},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xD2}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xD3},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xD4}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xD5},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xD6}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xD7},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xD8}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xD9},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xDA}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xDB},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xDC}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xDD},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xDE}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xDF},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0xE0}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xE1},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xE2}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xE3},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xE4}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xE5},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xE6}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xE7},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xE8}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xE9},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xEA}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xEB},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xEC}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xED},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xEE}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xEF},

    (SDL_Color){0xFF, 0xFF, 0xFF, 0xF0}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xF1},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xF2}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xF3},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xF4}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xF5},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xF6}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xF7},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xF8}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xF9},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xFA}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xFB},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xFC}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xFD},
    (SDL_Color){0xFF, 0xFF, 0xFF, 0xFE}, (SDL_Color){0xFF, 0xFF, 0xFF, 0xFF},
};

/************************* Fonts *************************/

typedef struct {
  unsigned int x0, x1, y0, y1, loaded;
  int bitmap_left, bitmap_top;
  float xadvance;
} GlyphMetric;

typedef struct {
  SDL_Surface *surface;
  GlyphMetric metrics[GLYPHSET_SIZE];
} GlyphSet;

typedef struct RenFont {
  FT_Face face;
  FT_StreamRec stream;
  hb_font_t *font;
  GlyphSet *sets[SUBPIXEL_BITMAPS_CACHED][MAX_LOADABLE_GLYPHSETS];
  float size, space_advance;
  int tab_size;
  unsigned short max_height, baseline, height;
  ERenFontAntialiasing antialiasing;
  ERenFontHinting hinting;
  unsigned char style;
  unsigned short underline_thickness;
  char path[];
} RenFont;

static int font_set_load_options(RenFont *font) {
  int load_target =
      font->antialiasing == FONT_ANTIALIASING_NONE
          ? FT_LOAD_TARGET_MONO
          : (font->hinting == FONT_HINTING_SLIGHT ? FT_LOAD_TARGET_LIGHT
                                                  : FT_LOAD_TARGET_NORMAL);
  int hinting = font->hinting == FONT_HINTING_NONE ? FT_LOAD_NO_HINTING
                                                   : FT_LOAD_FORCE_AUTOHINT;
  return load_target | hinting;
}

static int font_set_render_options(RenFont *font) {
  if (font->antialiasing == FONT_ANTIALIASING_NONE)
    return FT_RENDER_MODE_MONO;
  if (font->antialiasing == FONT_ANTIALIASING_SUBPIXEL) {
    unsigned char weights[] = {0x10, 0x40, 0x70, 0x40, 0x10};
    switch (font->hinting) {
    case FONT_HINTING_NONE:
      FT_Library_SetLcdFilter(library, FT_LCD_FILTER_NONE);
      break;
    case FONT_HINTING_SLIGHT:
    case FONT_HINTING_FULL:
      FT_Library_SetLcdFilterWeights(library, weights);
      break;
    }
    return FT_RENDER_MODE_LCD;
  } else {
    switch (font->hinting) {
    case FONT_HINTING_NONE:
      return FT_RENDER_MODE_NORMAL;
      break;
    case FONT_HINTING_SLIGHT:
      return FT_RENDER_MODE_LIGHT;
      break;
    case FONT_HINTING_FULL:
      return FT_RENDER_MODE_LIGHT;
      break;
    }
  }
  return 0;
}

static int font_set_style(FT_Outline *outline, int x_translation,
                          unsigned char style) {
  FT_Outline_Translate(outline, x_translation, 0);
  if (style & FONT_STYLE_SMOOTH)
    FT_Outline_Embolden(outline, 1 << 5);
  if (style & FONT_STYLE_BOLD)
    FT_Outline_EmboldenXY(outline, 1 << 5, 0);
  if (style & FONT_STYLE_ITALIC) {
    FT_Matrix matrix = {1 << 16, 1 << 14, 0, 1 << 16};
    FT_Outline_Transform(outline, &matrix);
  }
  return 0;
}

static void font_load_glyphset(RenFont *font, unsigned int idx) {
  unsigned int render_option = font_set_render_options(font),
               load_option = font_set_load_options(font);
  int bitmaps_cached = font->antialiasing == FONT_ANTIALIASING_SUBPIXEL
                           ? SUBPIXEL_BITMAPS_CACHED
                           : 1;
  unsigned int byte_width =
      font->antialiasing == FONT_ANTIALIASING_SUBPIXEL ? 3 : 1;
  for (int j = 0, pen_x = 0; j < bitmaps_cached; ++j) {
    GlyphSet *set = check_alloc(calloc(1, sizeof(GlyphSet)));
    font->sets[j][idx] = set;
    for (int i = 0; i < GLYPHSET_SIZE; ++i) {
      int glyph_index = i + idx * GLYPHSET_SIZE;
      if (!glyph_index ||
          FT_Load_Glyph(font->face, glyph_index,
                        load_option | FT_LOAD_BITMAP_METRICS_ONLY) ||
          font_set_style(&font->face->glyph->outline,
                         j * (64 / SUBPIXEL_BITMAPS_CACHED), font->style) ||
          FT_Render_Glyph(font->face->glyph, render_option)) {
        continue;
      }
      FT_GlyphSlot slot = font->face->glyph;
      unsigned int glyph_width = slot->bitmap.width / byte_width;
      if (font->antialiasing == FONT_ANTIALIASING_NONE)
        glyph_width *= 8;
      set->metrics[i] = (GlyphMetric){
          pen_x,
          pen_x + glyph_width,
          0,
          slot->bitmap.rows,
          true,
          slot->bitmap_left,
          slot->bitmap_top,
          (slot->advance.x + slot->lsb_delta - slot->rsb_delta) / 64.0f};
      pen_x += glyph_width;
      font->max_height = slot->bitmap.rows > font->max_height
                             ? slot->bitmap.rows
                             : font->max_height;
      // In order to fix issues with monospacing; we need the unhinted xadvance;
      // as FreeType doesn't correctly report the hinted advance for spaces on
      // monospace fonts (like RobotoMono). See #843.
      if (FT_Load_Glyph(
              font->face, glyph_index,
              (load_option | FT_LOAD_BITMAP_METRICS_ONLY | FT_LOAD_NO_HINTING) &
                  ~FT_LOAD_FORCE_AUTOHINT) ||
          font_set_style(&font->face->glyph->outline,
                         j * (64 / SUBPIXEL_BITMAPS_CACHED), font->style) ||
          FT_Render_Glyph(font->face->glyph, render_option)) {
        continue;
      }
      slot = font->face->glyph;
      set->metrics[i].xadvance = slot->advance.x / 64.0f;
    }
    if (pen_x == 0)
      continue;
    set->surface = check_alloc(SDL_CreateRGBSurface(
        0, pen_x, font->max_height,
        font->antialiasing == FONT_ANTIALIASING_SUBPIXEL ? 24 : 8, 0, 0, 0, 0));
    uint8_t *pixels = set->surface->pixels;
    for (int i = 0; i < GLYPHSET_SIZE; ++i) {
      int glyph_index = i + idx * GLYPHSET_SIZE;
      if (!glyph_index || FT_Load_Glyph(font->face, glyph_index, load_option))
        continue;
      FT_GlyphSlot slot = font->face->glyph;
      font_set_style(&slot->outline, (64 / bitmaps_cached) * j, font->style);
      if (FT_Render_Glyph(slot, render_option))
        continue;
      for (unsigned int line = 0; line < slot->bitmap.rows; ++line) {
        int target_offset =
            set->surface->pitch * line + set->metrics[i].x0 * byte_width;
        int source_offset = line * slot->bitmap.pitch;
        if (font->antialiasing == FONT_ANTIALIASING_NONE) {
          for (unsigned int column = 0; column < slot->bitmap.width; ++column) {
            int current_source_offset = source_offset + (column / 8);
            int source_pixel = slot->bitmap.buffer[current_source_offset];
            pixels[++target_offset] =
                ((source_pixel >> (7 - (column % 8))) & 0x1) * 0xFF;
          }
        } else
          memcpy(&pixels[target_offset], &slot->bitmap.buffer[source_offset],
                 slot->bitmap.width);
      }
    }
  }
}

static GlyphSet *font_get_glyphset(RenFont *font, unsigned int codepoint,
                                   int subpixel_idx) {
  int idx = (codepoint / GLYPHSET_SIZE) % MAX_LOADABLE_GLYPHSETS;
  if (!font->sets[font->antialiasing == FONT_ANTIALIASING_SUBPIXEL
                      ? subpixel_idx
                      : 0][idx])
    font_load_glyphset(font, idx);
  return font
      ->sets[font->antialiasing == FONT_ANTIALIASING_SUBPIXEL ? subpixel_idx
                                                              : 0][idx];
}

static RenFont *font_group_get_glyph(GlyphSet **set, GlyphMetric **metric,
                                     RenFont **fonts, unsigned int codepoint,
                                     unsigned fb_codepoint, int bitmap_index) {
  if (!metric) {
    return NULL;
  }
  if (bitmap_index < 0)
    bitmap_index += SUBPIXEL_BITMAPS_CACHED;
  for (int i = 0; i < FONT_FALLBACK_MAX && fonts[i]; ++i) {
    unsigned cp =
        i == 0 ? codepoint : FT_Get_Char_Index(fonts[i]->face, fb_codepoint);
    *set = font_get_glyphset(fonts[i], cp, bitmap_index);
    *metric = &(*set)->metrics[cp % GLYPHSET_SIZE];
    if ((*metric)->loaded || fb_codepoint == 0)
      return fonts[i];
  }
  if (*metric && !(*metric)->loaded && fb_codepoint > 0xFF &&
      fb_codepoint != 0x25A1)
    return font_group_get_glyph(set, metric, fonts, 0x25A1, 0x25A1,
                                bitmap_index);
  return fonts[0];
}

static void font_clear_glyph_cache(RenFont *font) {
  for (int i = 0; i < SUBPIXEL_BITMAPS_CACHED; ++i) {
    for (int j = 0; j < MAX_LOADABLE_GLYPHSETS; ++j) {
      if (font->sets[i][j]) {
        if (font->sets[i][j]->surface)
          SDL_FreeSurface(font->sets[i][j]->surface);
        free(font->sets[i][j]);
        font->sets[i][j] = NULL;
      }
    }
  }
}

// based on
// https://github.com/libsdl-org/SDL_ttf/blob/2a094959055fba09f7deed6e1ffeb986188982ae/SDL_ttf.c#L1735
static unsigned long font_file_read(FT_Stream stream, unsigned long offset,
                                    unsigned char *buffer,
                                    unsigned long count) {
  uint64_t amount;
  SDL_RWops *file = (SDL_RWops *)stream->descriptor.pointer;
  SDL_RWseek(file, (int)offset, RW_SEEK_SET);
  if (count == 0)
    return 0;
  amount = SDL_RWread(file, buffer, sizeof(char), count);
  if (amount <= 0)
    return 0;
  return (unsigned long)amount;
}

static void font_file_close(FT_Stream stream) {
  if (stream && stream->descriptor.pointer) {
    SDL_RWclose((SDL_RWops *)stream->descriptor.pointer);
    stream->descriptor.pointer = NULL;
  }
}

RenFont *ren_font_load(RenWindow *window_renderer, const char *path, float size,
                       ERenFontAntialiasing antialiasing,
                       ERenFontHinting hinting, unsigned char style) {
  RenFont *font = NULL;
  FT_Face face = NULL;
  SDL_RWops *file = SDL_RWFromFile(path, "rb");
  if (!file)
    goto rwops_failure;

  int len = strlen(path);
  font = check_alloc(calloc(1, sizeof(RenFont) + len + 1));
  font->stream.read = font_file_read;
  font->stream.close = font_file_close;
  font->stream.descriptor.pointer = file;
  font->stream.pos = 0;
  font->stream.size = (unsigned long)SDL_RWsize(file);

  if (FT_Open_Face(
          library,
          &(FT_Open_Args){.flags = FT_OPEN_STREAM, .stream = &font->stream}, 0,
          &face))
    goto failure;
  const int surface_scale = renwin_get_surface(window_renderer).scale;
  if (FT_Set_Pixel_Sizes(face, 0, (int)(size * surface_scale)))
    goto failure;

  strcpy(font->path, path);
  font->face = face;
  font->size = size;
  font->height =
      (short)((face->height / (float)face->units_per_EM) * font->size);
  font->baseline =
      (short)((face->ascender / (float)face->units_per_EM) * font->size);
  font->antialiasing = antialiasing;
  font->hinting = hinting;
  font->style = style;

  if (FT_IS_SCALABLE(face))
    font->underline_thickness = (unsigned short)((face->underline_thickness /
                                                  (float)face->units_per_EM) *
                                                 font->size);
  if (!font->underline_thickness)
    font->underline_thickness = ceil((double)font->height / 14.0);

  if (FT_Load_Char(face, ' ', font_set_load_options(font)))
    goto failure;

  font->font = hb_ft_font_create_referenced(face);
  if (font->font == 0)
    goto failure;
  font->space_advance = face->glyph->advance.x / 64.0f;
  font->tab_size = 0;
  return font;

failure:
  if (face)
    FT_Done_Face(face);
  if (font && font->font)
    hb_font_destroy(font->font);
  if (font)
    free(font);
  return NULL;

rwops_failure:
  if (file)
    SDL_RWclose(file);
  return NULL;
}

RenFont *ren_font_copy(RenWindow *window_renderer, RenFont *font, float size,
                       ERenFontAntialiasing antialiasing,
                       ERenFontHinting hinting, int style) {
  antialiasing = antialiasing == -1 ? font->antialiasing : antialiasing;
  hinting = hinting == -1 ? font->hinting : hinting;
  style = style == -1 ? font->style : style;

  return ren_font_load(window_renderer, font->path, size, antialiasing, hinting,
                       style);
}

const char *ren_font_get_path(RenFont *font) { return font->path; }

void ren_font_free(RenFont *font) {
  font_clear_glyph_cache(font);
  FT_Done_Face(font->face);
  hb_font_destroy(font->font);
  free(font);
}

void ren_font_group_set_tab_size(RenFont **fonts, int n) {
  for (int j = 0; j < FONT_FALLBACK_MAX && fonts[j]; ++j) {
    if (fonts[j]->tab_size == n)
      continue;
    unsigned codepoint = 0;
    if (j == 0) {
      hb_buffer_t *buff = hb_buffer_create();
      hb_buffer_set_direction(buff, HB_DIRECTION_LTR);
      hb_buffer_set_script(buff, HB_SCRIPT_LATIN);
      hb_buffer_add_utf8(buff, "\t", 1, 0, -1);
      hb_shape(fonts[j]->font, buff, NULL, 0);
      unsigned counter = 0;
      hb_glyph_info_t *info = hb_buffer_get_glyph_infos(buff, &counter);
      codepoint = info[0].codepoint;
      hb_buffer_destroy(buff);
    } else {
      codepoint = FT_Get_Char_Index(fonts[j]->face, '\t');
    }
    fonts[j]->tab_size = n;
    float f = (n - 1) * fonts[j]->space_advance;
    unsigned index = codepoint % GLYPHSET_SIZE;
    for (int i = 0; i < (fonts[j]->antialiasing == FONT_ANTIALIASING_SUBPIXEL
                             ? SUBPIXEL_BITMAPS_CACHED
                             : 1);
         ++i) {
      GlyphSet *set = font_get_glyphset(fonts[j], codepoint, i);
      if (set != 0 && index < GLYPHSET_SIZE)
        set->metrics[index].xadvance = f;
    }
  }
}

int ren_font_group_get_tab_size(RenFont **fonts) { return fonts[0]->tab_size; }

float ren_font_group_get_size(RenFont **fonts) { return fonts[0]->size; }

void ren_font_group_set_size(RenWindow *window_renderer, RenFont **fonts,
                             float size) {
  const int surface_scale = renwin_get_surface(window_renderer).scale;
  for (int i = 0; i < FONT_FALLBACK_MAX && fonts[i]; ++i) {
    font_clear_glyph_cache(fonts[i]);
    FT_Face face = fonts[i]->face;
    FT_Set_Pixel_Sizes(face, 0, (int)(size * surface_scale));
    fonts[i]->size = size;
    fonts[i]->height =
        (short)((face->height / (float)face->units_per_EM) * size);
    fonts[i]->baseline =
        (short)((face->ascender / (float)face->units_per_EM) * size);
    FT_Load_Char(face, ' ', font_set_load_options(fonts[i]));
    fonts[i]->space_advance = face->glyph->advance.x / 64.0f;
  }
}

int ren_font_group_get_height(RenFont **fonts) { return fonts[0]->height; }

static const unsigned utf8_to_codepoint(const char *p) {
  const unsigned char *up = (unsigned char *)p;
  unsigned res, n;
  switch (*p & 0xf0) {
  case 0xf0:
    res = *up & 0x07;
    n = 3;
    break;
  case 0xe0:
    res = *up & 0x0f;
    n = 2;
    break;
  case 0xd0:
  case 0xc0:
    res = *up & 0x1f;
    n = 1;
    break;
  default:
    res = *up;
    n = 0;
    break;
  }
  while (n--) {
    res = (res << 6) | (*(++up) & 0x3f);
  }
  return res;
}

double ren_font_group_get_width(RenWindow *window_renderer, RenFont **fonts,
                                const char *text, size_t len) {
  double width = 0;
  GlyphMetric *metric = NULL;
  GlyphSet *set = NULL;
  hb_buffer_t *buf;
  buf = hb_buffer_create();
  hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
  hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
  hb_buffer_add_utf8(buf, text, -1, 0, -1);
  RenFont *font = fonts[0];
  hb_shape(font->font, buf, NULL, 0);
  unsigned int glyph_count;
  hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
  for (unsigned int i = 0; i < glyph_count; i++) {
    unsigned int codepoint = glyph_info[i].codepoint;
    unsigned fb_codepoint = utf8_to_codepoint(&text[glyph_info[i].cluster]);
    RenFont *font =
        font_group_get_glyph(&set, &metric, fonts, codepoint, fb_codepoint, 0);
    if (!metric)
      break;
    width += (!font || metric->xadvance) ? metric->xadvance
                                         : fonts[0]->space_advance;
  }
  hb_buffer_destroy(buf);
  const int surface_scale = renwin_get_surface(window_renderer).scale;
  return width / surface_scale;
}

SDL_Texture *CreateTextureFromFT_Bitmap(SDL_Renderer *renderer,
                                        FT_Bitmap *bitmap) {
  if (bitmap->width == 0 || bitmap->rows == 0) {
    return NULL;
  }

  SDL_Palette *palette = SDL_AllocPalette(256);
  SDL_SetPaletteColors(palette, glyphPaletteColor, 0, 256);

  SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(
      bitmap->buffer, bitmap->width, bitmap->rows, 8, bitmap->pitch,
      SDL_PIXELFORMAT_INDEX8);

  SDL_SetSurfacePalette(surface, palette);

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  SDL_FreePalette(palette);

  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

  return texture;
}

double ren_draw_text(RenSurface *rs, RenFont **fonts, const char *text,
                     size_t len, float x, int y, RenColor color) {
  const int surface_scale = rs->scale;
  x *= surface_scale;
  y *= surface_scale;
  // const char* end = text + len;

  // convert text in glyphs
  hb_buffer_t *buf;
  buf = hb_buffer_create();
  hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
  hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
  hb_buffer_add_utf8(buf, text, len, 0, -1);

  SDL_Renderer *renderer = window_renderer.renderer;
  RenFont *font = fonts[0];
  hb_shape(font->font, buf, NULL, 0);
  unsigned int glyph_count;
  hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
  hb_glyph_position_t *glyph_positions =
      hb_buffer_get_glyph_positions(buf, NULL);

  for (unsigned int i = 0; i < glyph_count; i++) {
    FT_Load_Glyph(font->face, glyph_info[i].codepoint, FT_LOAD_RENDER);

    SDL_Texture *glyph_texture =
        CreateTextureFromFT_Bitmap(renderer, &font->face->glyph->bitmap);

    if (glyph_texture != NULL) {
      SDL_Rect dest;
      SDL_QueryTexture(glyph_texture, NULL, NULL, &dest.w, &dest.h);

      // FreeType's metrics is in 26.6 fixed-point format.
      // shift the number to the right by 6 bit to round it off to integer.
      dest.x = round(x + (font->face->glyph->metrics.horiBearingX >> 6) +
                     (glyph_positions[i].x_offset >> 6));
      // dest.y = round(y - (font->face->glyph->metrics.horiBearingY >> 6));
      //          //(glyph_positions[i].y_offset >> 6));
      dest.y =
          y - (font->face->glyph->metrics.horiBearingY >> 6) + font->baseline;
      SDL_SetTextureBlendMode(glyph_texture, SDL_BLENDMODE_BLEND);
      SDL_SetTextureColorMod(glyph_texture, color.r, color.g, color.b);
      SDL_RenderCopy(renderer, glyph_texture, NULL, &dest);
      SDL_DestroyTexture(glyph_texture);
    }

    x += (glyph_positions[i].x_advance >> 6);
    // unsigned int r, g, b;
    // unsigned fb_codepoint = utf8_to_codepoint(&text[glyph_info[i].cluster]);
    // hb_codepoint_t codepoint = glyph_info[i].codepoint;
    // GlyphSet *set = NULL;
    // GlyphMetric *metric = NULL;
    // RenFont *font =
    //     font_group_get_glyph(&set, &metric, fonts, codepoint, fb_codepoint,
    //                          (int)(fmod(pen_x, 1.0) *
    //                          SUBPIXEL_BITMAPS_CACHED));
    // if (!metric)
    //   break;
    // int start_x = floor(pen_x) + metric->bitmap_left;
    // int end_x = (metric->x1 - metric->x0) + start_x;
    // int glyph_end = metric->x1, glyph_start = metric->x0;
    // if (!metric->loaded && fb_codepoint > 0xFF)
    //   ren_draw_rect(rs,
    //                 (RenRect){start_x + 1, y, font->space_advance - 1,
    //                           ren_font_group_get_height(fonts)},
    //                 color);

    // if (set->surface && color.a > 0 && end_x >= clip.x &&
    //     start_x < clip_end_x) {
    //   uint8_t *source_pixels = set->surface->pixels;
    //   for (int line = metric->y0; line < metric->y1; ++line) {
    //     int target_y =
    //         line + y - metric->bitmap_top + fonts[0]->baseline *
    //         surface_scale;
    //     if (target_y < clip.y)
    //       continue;
    //     if (target_y >= clip_end_y)
    //       break;
    //     if (start_x + (glyph_end - glyph_start) >= clip_end_x)
    //       glyph_end = glyph_start + (clip_end_x - start_x);
    //     if (start_x < clip.x) {
    //       int offset = clip.x - start_x;
    //       start_x += offset;
    //       glyph_start += offset;
    //     }
    //     uint32_t *destination_pixel =
    //         (uint32_t *)&(destination_pixels[surface->pitch * target_y +
    //                                          start_x * bytes_per_pixel]);
    //     uint8_t *source_pixel =
    //         &source_pixels[line * set->surface->pitch +
    //                        glyph_start *
    //                            (font->antialiasing ==
    //                            FONT_ANTIALIASING_SUBPIXEL
    //                                 ? 3
    //                                 : 1)];
    //     for (int x = glyph_start; x < glyph_end; ++x) {
    //       uint32_t destination_color = *destination_pixel;
    //       // the standard way of doing this would be SDL_GetRGBA, but that
    //       // introduces a performance regression. needs to be investigated
    //       SDL_Color dst = {(destination_color & surface->format->Rmask) >>
    //                            surface->format->Rshift,
    //                        (destination_color & surface->format->Gmask) >>
    //                            surface->format->Gshift,
    //                        (destination_color & surface->format->Bmask) >>
    //                            surface->format->Bshift,
    //                        (destination_color & surface->format->Amask) >>
    //                            surface->format->Ashift};
    //       SDL_Color src;

    //       if (font->antialiasing == FONT_ANTIALIASING_SUBPIXEL) {
    //         src.r = *(source_pixel++);
    //         src.g = *(source_pixel++);
    //       } else {
    //         src.r = *(source_pixel);
    //         src.g = *(source_pixel);
    //       }

    //       src.b = *(source_pixel++);
    //       src.a = 0xFF;

    //       r = (color.r * src.r * color.a + dst.r * (65025 - src.r * color.a)
    //       +
    //            32767) /
    //           65025;
    //       g = (color.g * src.g * color.a + dst.g * (65025 - src.g * color.a)
    //       +
    //            32767) /
    //           65025;
    //       b = (color.b * src.b * color.a + dst.b * (65025 - src.b * color.a)
    //       +
    //            32767) /
    //           65025;
    //       // the standard way of doing this would be SDL_GetRGBA, but that
    //       // introduces a performance regression. needs to be investigated
    //       *destination_pixel++ =
    //           dst.a << surface->format->Ashift | r << surface->format->Rshift
    //           | g << surface->format->Gshift | b << surface->format->Bshift;
    //     }
    //   }
    // }

    // float adv = metric->xadvance ? metric->xadvance : font->space_advance;

    // if (!last)
    //   last = font;
    // else if (font != last || i == glyph_count - 1) {
    //   double local_pen_x = i == glyph_count - 1 ? pen_x + adv : pen_x;
    //   if (underline)
    //     ren_draw_rect(rs,
    //                   (RenRect){last_pen_x,
    //                             y / surface_scale + last->height - 1,
    //                             (local_pen_x - last_pen_x) / surface_scale,
    //                             last->underline_thickness * surface_scale},
    //                   color);
    //   if (strikethrough)
    //     ren_draw_rect(rs,
    //                   (RenRect){last_pen_x,
    //                             y / surface_scale + last->height / 2,
    //                             (local_pen_x - last_pen_x) / surface_scale,
    //                             last->underline_thickness * surface_scale},
    //                   color);
    //   last = font;
    //   last_pen_x = pen_x;
    // }

    // pen_x += adv;
  }
  hb_buffer_destroy(buf);
  return x / surface_scale;
}

/******************* Rectangles **********************/
// static inline RenColor blend_pixel(RenColor dst, RenColor src) {
//   int ia = 0xff - src.a;
//   dst.r = ((src.r * src.a) + (dst.r * ia)) >> 8;
//   dst.g = ((src.g * src.a) + (dst.g * ia)) >> 8;
//   dst.b = ((src.b * src.a) + (dst.b * ia)) >> 8;
//   return dst;
// }

void ren_draw_rect(RenSurface *rs, RenRect rect, RenColor color) {
  if (color.a == SDL_ALPHA_TRANSPARENT) {
    return;
  }

  const int surface_scale = rs->scale;

  SDL_Rect dest_rect = {rect.x * surface_scale, rect.y * surface_scale,
                        rect.width * surface_scale,
                        rect.height * surface_scale};

  SDL_SetRenderDrawColor(window_renderer.renderer, color.r, color.g, color.b,
                         color.a);
  SDL_RenderFillRect(window_renderer.renderer, &dest_rect);
}

void ren_draw_outline_rect(RenSurface *rs, RenRect rect, RenColor color) {
  if (color.a == SDL_ALPHA_TRANSPARENT) {
    return;
  }

  const int surface_scale = rs->scale;

  SDL_Rect dest_rect = {rect.x * surface_scale, rect.y * surface_scale,
                        rect.width * surface_scale,
                        rect.height * surface_scale};

  SDL_SetRenderDrawColor(window_renderer.renderer, color.r, color.g, color.b,
                         color.a);
  SDL_RenderDrawRect(window_renderer.renderer, &dest_rect);
}

/*************** Window Management ****************/
void ren_free_window_resources(RenWindow *window_renderer) {
  renwin_free(window_renderer);
  SDL_FreeSurface(draw_rect_surface);
  free(window_renderer->command_buf);
  window_renderer->command_buf = NULL;
  window_renderer->command_buf_size = 0;
}

// TODO remove global and return RenWindow*
void ren_init(SDL_Window *win) {
  assert(win);
  int error = FT_Init_FreeType(&library);
  if (error) {
    fprintf(stderr, "internal font error when starting the application\n");
    return;
  }
  window_renderer.window = win;
  renwin_init_surface(&window_renderer);
  renwin_init_command_buf(&window_renderer);
  renwin_clip_to_surface(&window_renderer);
  draw_rect_surface = SDL_CreateRGBSurface(0, 1, 1, 32, 0xFF000000, 0x00FF0000,
                                           0x0000FF00, 0x000000FF);
}

void ren_resize_window(RenWindow *window_renderer) {
  renwin_resize_surface(window_renderer);
}

void ren_update_rects(RenWindow *window_renderer) {
  static bool initial_frame = true;
  if (initial_frame) {
    renwin_show_window(window_renderer);
    initial_frame = false;
  }
  renwin_update(window_renderer);
}

void ren_set_clip_rect(RenWindow *window_renderer, RenRect rect) {
  renwin_set_clip_rect(window_renderer, rect);
}

void ren_get_size(RenWindow *window_renderer, int *x, int *y) {
  RenSurface rs = renwin_get_surface(window_renderer);
  *x = rs.surface->w / rs.scale;
  *y = rs.surface->h / rs.scale;
}
