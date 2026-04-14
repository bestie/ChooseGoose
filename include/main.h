#ifndef MAIN_H
#define MAIN_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "cli_opts.h"
#include "state.h"

// SDL function pointers for abstraction and testing
typedef struct {
    int (*init)(Uint32 flags);
    void (*quit)(void);
    SDL_Surface* (*set_video_mode)(int w, int h, int bpp, Uint32 flags);
    int (*enable_key_repeat)(int delay, int interval);
    Uint32 (*get_ticks)(void);
    int (*poll_event)(SDL_Event *event);
    int (*flip)(SDL_Surface *screen);
    int (*fill_rect)(SDL_Surface *dst, SDL_Rect *rect, Uint32 color);
    void (*free_surface)(SDL_Surface *surface);
    void (*delay)(Uint32 ms);
    int (*blit_surface)(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
    int (*show_cursor)(int toggle);
    int (*num_joysticks)(void);
    char* (*get_error)(void);
    char* (*joystick_name)(int device_index);
    SDL_Joystick* (*joystick_open)(int device_index);
    int (*ttf_init)(void);
    void (*ttf_quit)(void);
    SDL_Surface* (*ttf_rendertext_blended)(TTF_Font *font, const char *text, SDL_Color fg);
    TTF_Font* (*ttf_open_font)(const char *file, int ptsize);
    TTF_Font* (*ttf_open_font_rw)(SDL_RWops *src, int freesrc, int ptsize);
    int (*ttf_font_height)(const TTF_Font *font);
    void (*ttf_close_font)(TTF_Font *font);
    void (*img_quit)(void);
    SDL_Surface* (*img_load)(const char *file);
    SDL_Surface* (*img_load_rw)(SDL_RWops *src, int freesrc);
    SDL_RWops* (*rw_from_mem)(void *mem, int size);
    SDL_Surface* (*create_rgb_surface)(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
    Uint32 (*map_rgb)(const SDL_PixelFormat *fmt, Uint8 r, Uint8 g, Uint8 b);
    void (*joystick_close)(SDL_Joystick *joystick);
} SDL_Interface;

SDL_Interface* get_sdl_interface(void);
void set_sdl_interface(SDL_Interface* interface);

void init_sdl(Config* config, State* state);
void cleanup();

void goose_setup(Config *config, State *state);
void event_loop(Config *config, State *state);
void set_log_file_pointer(FILE* file);
void set_output(FILE*);

#endif
