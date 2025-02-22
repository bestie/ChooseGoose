#ifndef MAIN_H
#define MAIN_H

#include <SDL/SDL_events.h>
#include <SDL/SDL_events.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

#include "cli_opts.h"
#include "state.h"

// SDL function pointers for abstraction and testing
typedef struct {
    int (*init)(Uint32 flags);
    void (*quit)(void);
    SDL_Surface* (*set_video_mode)(int w, int h, int bpp, Uint32 flags);
    void (*wm_set_caption)(const char *title, const char *icon);
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
} SDL_Interface;

/*// Function to return the default implementation (real SDL functions)*/
/*SDL_Interface get_real_sdl_interface(void);*/

void init_sdl(Config* config, State* state);
void cleanup();

void goose_setup(Config *config, State *state);
void event_loop(Config *config, State *state);
void set_log_file_pointer(FILE* file);
void set_sdl_interface(SDL_Interface* interface);
SDL_Interface* get_sdl_interface(void);

#endif
