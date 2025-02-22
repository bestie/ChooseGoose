#ifndef STATE_H
#define STATE_H
#include "SDL/SDL_ttf.h"
#include <SDL/SDL.h>
#include <stdio.h>

typedef struct {
  int count;
  int max_length;
  char **lines;
} BunchOfLines;

typedef struct {
    FILE *log_file;
    int title_height;
    int menu_item_height;
    int menu_height;
    int menu_max_items;
    int font_pixel_height;
    int selected_index;
    int button_repeat_active;

    Uint32 background_color;
    SDL_Surface *screen;
    SDL_Surface *background_image;
    SDL_Surface *title;
    TTF_Font *title_font;
    TTF_Font *font;
    SDL_Joystick *joystick;
    BunchOfLines* menu_items;
} State;

State* init_state();
void cleanup_state(State *state);
BunchOfLines* read_lines_from_stdin(int max_lines, int max_line_length);

#endif
