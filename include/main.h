#ifndef MAIN_H
#define MAIN_H

#include <SDL/SDL_events.h>
#include <SDL/SDL_events.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

#include "cli_opts.h"

void initSDL(void);
void cleanup(void);
SDL_Surface* create_text_surface(char* text, Color color, TTF_Font* font);
TTF_Font* load_font(char *font_filepath, int font_size);
int setup(int argc, char **argv);

#endif
