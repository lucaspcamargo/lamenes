//#define SHOW_FPS

/* fps check interval */
#define FPS_INTERVAL 1.0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "lamenes.h"
#include "palette.h"

#include "system/display.h"

static SDL_Window *window;
static SDL_Surface *screen;

/* the last recorded time */
Uint32 fps_lasttime;

/* fps trackers */
Uint32 fps_current;
Uint32 fps_previous;

/* frames passed since the last recorded fps */
Uint32 fps_frames = 0;

/* standard fps target is pal speed */
int fps_target = 50;

/* this is for the speed adjustment calculations */
int fps_target_deviation = 3;
int fps_target_min;
int fps_target_max;

void display_init(uint16_t width, uint16_t height, DisplayType display_type, bool fullscreen) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("error: unable to init SDL %s\n", SDL_GetError());
    exit(1);
  }

  atexit(SDL_Quit);

  uint32_t flags = (fullscreen) ? (SDL_WINDOW_FULLSCREEN) : 0;

  window = SDL_CreateWindow("lamenes",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            width,
                            height,
                            flags);

  if (window == NULL) {
    printf("error: unable to create window %s\n", SDL_GetError());
    exit(1);
  }

  screen = SDL_GetWindowSurface(window);

  if (screen == NULL) {
    printf("error: unable to create screen %s\n", SDL_GetError());
    exit(1);
  }

  /* init fps start values */
  fps_lasttime = SDL_GetTicks();

  /* reset the fps target if needed */
  if (ntsc == 1)
    fps_target = 60;

  /* and give it some slack */
  fps_target_min = fps_target - fps_target_deviation;
  fps_target_max = fps_target + fps_target_deviation;
}

void display_lock(void) {
  if (SDL_MUSTLOCK(screen)) {
    if (SDL_LockSurface(screen) < 0) {
      return;
    }
  }
}

void display_set_pixel(uint16_t x, uint16_t y, uint8_t nes_color) {
  /* don't fail on attempts to draw outside the screen. */
  if (x >= screen->w) {
    #ifdef PPU_DEBUG
    printf("[%d]: warning attempt to draw outside the screen! [x = %d]\n", debug_cnt, x);
    #endif

    return;
  }

  if (y >= screen->h) {
    #ifdef PPU_DEBUG
    printf("[%d]: warning attempt to draw outside the screen! [y = %d]\n", debug_cnt, y);
    #endif

    return;
  }

  uint8_t R = palette[nes_color].r;
  uint8_t G = palette[nes_color].g;
  uint8_t B = palette[nes_color].b;

  /* pixel tranparency */
  if(nes_color == 0)
    return;

  uint32_t color = SDL_MapRGB(screen->format, R, G, B);

  switch(screen->format->BytesPerPixel) {
    /* Assuming 8-bpp */
    case 1: {
      uint8_t *bufp;
      bufp = (uint8_t *)screen->pixels + y*screen->pitch + x;
      *bufp = color;
    }
    break;

    /* Probably 15-bpp or 16-bpp */
    case 2: {
      uint16_t *bufp;
      bufp = (uint16_t *)screen->pixels + y*screen->pitch/2 + x;
      *bufp = color;
    }
    break;

    /* Slow 24-bpp mode, usually not used */
    case 3: {
      uint8_t *bufp;
      bufp = (uint8_t *)screen->pixels + y*screen->pitch + x * 3;
      if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {
        bufp[0] = color;
        bufp[1] = color >> 8;
        bufp[2] = color >> 16;
      } else {
        bufp[2] = color;
        bufp[1] = color >> 8;
        bufp[0] = color >> 16;
      }
    }
    break;

    /* Probably 32-bpp */
    case 4: {
      uint32_t *bufp;
      bufp = (uint32_t *)screen->pixels + y*screen->pitch/4 + x;
      *bufp = color;
    }
    break;
  }
}

void display_update(void) {
  uint8_t nes_color = ppu_memory[0x3f00];

  /* update the screen */
  SDL_UpdateWindowSurface(window);

  /* clear the surface and set it to nes_color 0x10 */
  SDL_FillRect(screen,
               NULL,
               SDL_MapRGB(screen->format,
                          palette[nes_color].r,
                          palette[nes_color].g,
                          palette[nes_color].b));
}

void display_unlock(void) {
  if (SDL_MUSTLOCK(screen)) {
    SDL_UnlockSurface(screen);
  }
}

void update_fps_data(void) {
  fps_frames++;

  if (fps_lasttime < SDL_GetTicks() - FPS_INTERVAL*1000) {
    fps_lasttime = SDL_GetTicks();
    fps_current = fps_frames;
    fps_frames = 0;
  }

#ifdef SHOW_FPS
  printf("fps = %d / frameskip = %d / delay = %d\n", fps_current, frameskip, sdl_delay);
#endif

  /* only adjust speed when dynamic_speed is set. */
  if (dynamic_speed == 1) {
    /* check if the emulator speed needs to be adjusted. */
    if (fps_current > 0) {
      /* if the framerate is the same as before we can skip the adjustments. */
      if (fps_current != fps_previous) {
        if (fps_current > fps_target_max) {
          /* the framerate is very fast on todays cpus, so we need to adjust the speeds quickly. */
          sdl_delay += fps_current / 30;
        } else if (fps_current < fps_target_min) {
          if (sdl_delay > 0) {
            /* if the system is slower then todays cpus, we adjust it a little slower. */
            sdl_delay -= fps_current / 10;
          } else {
            printf("TODO: need to set frameskips!\n");
          }
        }

        /* save the fps_current for above check, this prevents that the sdl_delay is adjusted every time this function is executed. */
        fps_previous = fps_current;
      } 
    }
  }
}
