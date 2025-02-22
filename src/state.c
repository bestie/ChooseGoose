#include "state.h"

#define MAX_MENU_ITEMS 4096
#define MAX_LINE_LENGTH 255

State* init_state() {
    State* state = malloc(sizeof(State));

    state->title_height = 0;

    state->menu_height = 0;
    state->font_pixel_height = 0;
    state->selected_index = 0;
    state->button_repeat_active = 0;

    state->screen = NULL;
    state->background_image = NULL;
    state->title = NULL;
    state->title_font = NULL;
    state->font = NULL;
    state->joystick = NULL;
    state->menu_items = NULL;

    return state;
}

void cleanup_state(State *state) {
    if (state->log_file) {
        fclose(state->log_file);
    }
    if (state->background_image) {
        SDL_FreeSurface(state->background_image);
    }
    if (state->title) {
        SDL_FreeSurface(state->title);
    }
    if (state->screen) {
        SDL_FreeSurface(state->screen);
    }
    if (state->title_font) {
        TTF_CloseFont(state->title_font);
    }
    if (state->font) {
        TTF_CloseFont(state->font);
    }
    if (state->joystick) {
        SDL_JoystickClose(state->joystick);
    }
    SDL_Quit();
}

BunchOfLines* read_lines_from_stdin(int max_lines, int max_line_length) {
  size_t total_memory =
      max_lines * sizeof(char *) + max_lines * max_line_length;
  char **lines = malloc(total_memory);
  BunchOfLines *bunch = malloc(sizeof(BunchOfLines));

  char *line_memory = (char *)(lines + max_lines);
  for (int i = 0; i < max_lines; i++) {
    lines[i] = line_memory + i * max_line_length;
  }

  int lines_i = 0;
  while (lines_i < max_lines && fgets(lines[lines_i], max_line_length, stdin)) {
    lines[lines_i][strcspn(lines[lines_i], "\n")] = '\0';
    if(strlen(lines[lines_i]) > 0) {
      lines_i++;
    }
  }

  bunch->count = lines_i;
  bunch->max_length = max_line_length;
  bunch->lines = lines;

  return bunch;
}

