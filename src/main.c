#include <SDL/SDL_events.h>
#include <SDL/SDL_events.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "main.h"
#include "background_image.c"
#include "cli_opts.h"
#include "font.c"
#include "state.h"

#define MAX_MENU_ITEMS 4096
#define MAX_LINE_LENGTH 255
#define MENU_ITEM_PADDING 3

#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_SELECT 7
#define BUTTON_START 8
#define BUTTON_MENU 9
#define BUTTON_REPEAT_DELAY_MS 250
#define BUTTON_REPEAT_INTERVAL 150
#define SDL_UNUSED 0

// limited use global state for signal handling and logging
State* global_state;
FILE* log_file;

void cleanup(void) {
  /*cleanup_state(global_state);*/
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

void quit(int exit_code) {
  cleanup();
  exit(exit_code);
}

void terminate_at_file_extension(char *filename) {
  char *dot = strrchr(filename, '.');
  if (dot) {
    *dot = '\0';
  }
}

void set_log_target_by_filepath(char log_filepath[]) {
  if (strlen(log_filepath) == 0) {
    return;
  }

  if (strcmp(log_filepath, "stderr") == 0) {
    log_file = stderr;
  } else if (strcmp(log_filepath, "stdout") == 0) {
    log_file = stdout;
  } else {
    log_file = fopen(log_filepath, "a");
  }
}

void set_log_file_pointer(FILE* file) {
  log_file = file;
}

void log_event(const char *format, ...) {
  if (!log_file) {
    return;
  }

  char message[255];
  time_t current_time;
  struct tm *time_data;
  char time_buffer[20];

  time(&current_time);
  time_data = localtime(&current_time);
  strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_data);

  va_list args;
  va_start(args, format);
  vsprintf(message, format, args);
  va_end(args);

  pid_t processID = getpid();

  fprintf(log_file, "[%s][PID: %d] %s\n", time_buffer, processID, message);
  fflush(log_file);
}

TTF_Font* load_font(char *font_filepath, int font_size) {
  if (strlen(font_filepath) > 1 && access(font_filepath, R_OK) != -1) {
    log_event("Font loading from %s", font_filepath);
    return TTF_OpenFont(font_filepath, font_size);
  } else {
    log_event("Font file not set or not readable `%s`", font_filepath);
    SDL_RWops *rw = SDL_RWFromMem(default_font, default_font_len);
    return TTF_OpenFontRW(rw, 1, font_size);
  }
}

SDL_Interface sdl = {
    .init = SDL_Init,
    .quit = SDL_Quit,
    .set_video_mode = SDL_SetVideoMode,
    .wm_set_caption = SDL_WM_SetCaption,
    .enable_key_repeat = SDL_EnableKeyRepeat,
    .get_ticks = SDL_GetTicks,
    .poll_event = SDL_PollEvent,
    .flip = SDL_Flip,
    .fill_rect = SDL_FillRect,
    .free_surface = SDL_FreeSurface,
    .delay = SDL_Delay,
    .blit_surface = SDL_BlitSurface,
    .show_cursor = SDL_ShowCursor,
    .num_joysticks = SDL_NumJoysticks,
    .joystick_open = SDL_JoystickOpen,
    .ttf_init = TTF_Init,
};

void set_sdl_interface(SDL_Interface* interface) {
  sdl = *interface;
}

SDL_Interface* get_sdl_interface(void) {
  return &sdl;
}

void init_sdl(Config *config, State* state) {
  log_event("SDL initialization started.");
  if (sdl.init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    log_event("Unable to init SDL: %s\n", SDL_GetError());
    quit(1);
  }

  log_event("Configuring SDL");
  sdl.show_cursor(SDL_DISABLE);
  sdl.wm_set_caption(config->title, NULL);
  sdl.enable_key_repeat(config->key_repeat_delay_ms, config->key_repeat_interval_ms);

  log_event("SDL_SetVideoMode(SCREEN_WIDTH=%d, SCREEN_HEIGHT=%d, bpp=%d)",
            config->screen_width, config->screen_height, config->bits_per_pixel);
  state->screen = sdl.set_video_mode(config->screen_width, config->screen_height,
                            config->bits_per_pixel, SDL_SWSURFACE);

  log_event("Looking for joysticks");
  if (sdl.num_joysticks() > 0) {
    state->joystick = sdl.joystick_open(0);
    if (state->joystick == NULL) {
      log_event("Unable to open joystick: %s\n", sdl.get_error());
      quit(1);
    } else {
      log_event("Joystick opened: %s\n", SDL_JoystickName(0));
    }
  } else {
    log_event("No joysticks found");
  }

  log_event("Loading fonts");
  sdl.ttf_init();
  state->title_font = load_font(config->font_filepath, config->title_font_size);
  state->font = load_font(config->font_filepath, config->font_size);

  state->font_pixel_height = TTF_FontHeight(state->font);

  log_event("Font loaded size=%d, font_height=%dpx", config->font_size,
            state->font_pixel_height);

}

void timeout(int user_inactivity_timeout_ms) {
  log_event("Inactivity timeout reached - %ds", user_inactivity_timeout_ms / 1000);
  cleanup();
  exit(124);
}

SDL_Surface* create_text_surface(char *text, Color color, TTF_Font *font) {
  SDL_Color text_color = {color.r, color.g, color.b, SDL_UNUSED};

  SDL_Surface *text_surface = TTF_RenderText_Blended(font, text, text_color);
  return text_surface;
}

SDL_Surface* create_menu_item(Config* config, State* state, char *text, int selected) {
  Color text_color;

  if (selected) {
    text_color = config->text_selected_color;
  } else {
    text_color = config->text_color;
  }

  SDL_Surface *text_surface = create_text_surface(text, text_color, state->font);

  if (selected && config->text_selected_background_color.r > -1) {
    Color bg = config->text_selected_background_color;
    Uint32 sdl_bg_color = SDL_MapRGB(state->screen->format, bg.r, bg.g, bg.b);
    int menu_item_width = config->screen_width;
    SDL_Surface *text_bg_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, menu_item_width, state->font_pixel_height, config->bits_per_pixel, 0,0,0,0);
    SDL_Rect fillRect = { 0, 0, text_bg_surface->w, text_bg_surface->h };
    SDL_FillRect(text_bg_surface, &fillRect, sdl_bg_color);

    SDL_BlitSurface(text_surface, NULL, text_bg_surface, NULL);
    SDL_FreeSurface(text_surface);
    text_surface = text_bg_surface;
  }

  return text_surface;
}

void menu_move_selection(State* state, int increment, int cycle) {
  BunchOfLines* menu_items = state->menu_items;

  int from = state->selected_index;
  state->selected_index = state->selected_index + increment;
  if (cycle) {
    state->selected_index = state->selected_index % menu_items->count;
    if (state->selected_index < 0) {
      state->selected_index = menu_items->count - 1;
    }
  }

  if (state->selected_index < 0) {
    state->selected_index = 0;
  } else if (state->selected_index > menu_items->count) {
    state->selected_index = menu_items->count - 1;
  }

  log_event("Moved to from %d to %d", from, state->selected_index);
}

void menu_confirm(State *state) {
  log_event("Selection confirmed item=%d/%d - `%s`", state->selected_index,
            state->menu_items->count, state->menu_items->lines[state->selected_index]);
  fprintf(stdout, "%s\n", state->menu_items->lines[state->selected_index]);
  quit(0);
}

void handle_dpad(State* state, SDL_JoyHatEvent event) {
  log_event("D-Pad movement: hat %d, value: %d", event.hat, event.value);
  switch (event.value) {
  case SDL_HAT_UP:
    menu_move_selection(state, -1, 1);
    state->button_repeat_active = 1;
    break;
  case SDL_HAT_DOWN:
    menu_move_selection(state, 1, 1);
    state->button_repeat_active = 1;
    break;
  case SDL_HAT_LEFT:
    menu_move_selection(state, -state->menu_max_items, 0);
    state->button_repeat_active = 1;
    break;
  case SDL_HAT_RIGHT:
    menu_move_selection(state, state->menu_max_items, 0);
    state->button_repeat_active = 1;
    break;
  case SDL_HAT_CENTERED:
    state->button_repeat_active = 0;
    break;
  default:
    break;
  }
}

void handle_joypad_button(State* state, SDL_JoyButtonEvent event) {
  log_event("Joypad button pressed: button %d", event.button);
  switch (event.button) {
  case BUTTON_A:
    menu_confirm(state);
    break;
  case BUTTON_START:
    menu_confirm(state);
    break;
  case BUTTON_B:
    quit(1);
    break;
  case BUTTON_MENU:
    quit(1);
    break;
  default:
    break;
  }
}

void handle_key_press(State* state, SDL_Event event) {
  log_event("Keyboard keypress: value: %d", event.key.keysym.sym);
  switch (event.key.keysym.sym) {
  case SDLK_UP:
    menu_move_selection(state, -1, 1);
    break;
  case SDLK_DOWN:
    menu_move_selection(state, 1, 1);
    break;
  case SDLK_LEFT:
    menu_move_selection(state, -state->menu_max_items, 0);
    break;
  case SDLK_RIGHT:
    menu_move_selection(state, state->menu_max_items, 0);
    break;
  case SDLK_RETURN:
    menu_confirm(state);
    break;
  case SDLK_ESCAPE:
    quit(0);
    break;
  default:
    break;
  }
}

int handle_input(State* state, SDL_Event event) {
  int event_handled = 1;

  switch (event.type) {
  case SDL_JOYHATMOTION:
    handle_dpad(state, event.jhat);
    break;
  case SDL_JOYBUTTONDOWN:
    handle_joypad_button(state, event.jbutton);
    break;
  case SDL_KEYDOWN:
    handle_key_press(state, event);
    break;
  default:
    event_handled = 0;
  }
  return event_handled;
}

void set_title(Config *config, State* state) {
  if (strlen(config->title)) {
    state->title = create_text_surface(config->title, config->text_color, state->title_font);
    state->title_height = state->title->h + MENU_ITEM_PADDING * 2;
  }
}

void render(Config* config, State* state) {
  BunchOfLines* menu_items = state->menu_items;

  SDL_FillRect(state->screen, NULL, state->background_color);
  if (state->background_image) {
    SDL_BlitSurface(state->background_image, NULL, state->screen, NULL);
  }
  if (state->title) {
    SDL_Rect dest = {config->left_padding, config->top_padding, 0, 0};
    SDL_BlitSurface(state->title, NULL, state->screen, &dest);
  }

  int menu_y_offset = config->top_padding + state->title_height;

  int default_items_above = state->menu_max_items / 2;
  int default_items_below = state->menu_max_items - default_items_above - 1;

  int visible_menu_start = state->selected_index - default_items_above;
  int visible_menu_end = state->selected_index + default_items_below;

  // Not enough items above cursor, lock window to top of list
  if (visible_menu_start < 0) {
    visible_menu_start = 0;
    visible_menu_end = visible_menu_start + state->menu_max_items - 1;
  }

  // Not enough items below cursor lock window to bottom of list
  if (visible_menu_end > menu_items->count - 1) {
    visible_menu_end = menu_items->count - 1;
    visible_menu_start = visible_menu_end - state->menu_max_items + 1;
  }

  // Final constraint check that neither are out of bounds
  if (visible_menu_start < 0 || visible_menu_start > (menu_items->count - 1)) {
    visible_menu_start = 0;
  }
  if (visible_menu_end > menu_items->count - 1 || visible_menu_end < 0) {
    visible_menu_end = menu_items->count - 1;
  }

  log_event("Rendering item range %d-%d", visible_menu_start, visible_menu_end);

  int menu_index = 0;

  for (int i = 0; i < state->menu_items->count; i++) {
    if (visible_menu_end - visible_menu_end > i) {
      log_event("Done rendering items %d", i);
      break;
    }

    menu_index = visible_menu_start + i;

    char text[255];
    if (config->prefix_with_number) {
      sprintf(text, "%3d. %s", menu_index + 1, menu_items->lines[menu_index]);
    } else {
      sprintf(text, "%s", menu_items->lines[menu_index]);
    }
    if (config->hide_file_extensions) {
      terminate_at_file_extension(text);
    }

    int selected_state = state->selected_index == menu_index;

    SDL_Rect dest;
    dest.x = config->left_padding;
    dest.y = menu_y_offset + i * state->menu_item_height;
    dest.w = 0;
    dest.h = 0;

    SDL_Surface *menu_item = create_menu_item(config, state, text, selected_state);
    log_event("blitting menu item %d at %d,%d", menu_index, dest.x, dest.y);
    sdl.blit_surface(menu_item, NULL, state->screen, &dest);
    log_event("blitting menu item %d at %d,%d", menu_index, dest.x, dest.y);
    /*sdl.free_surface(menu_item);*/
  }

  log_event("flipping screen");
  sdl.flip(state->screen);
}

void set_background_image(State* state, char *background_image_filepath) {
  log_event("Background image file path is `%s`\n",
          background_image_filepath);
  if (strlen(background_image_filepath) == 0) {
    log_event("No background image");
  } else if (strcmp(background_image_filepath, "DEFAULT") == 0) {
    log_event("Using default background image\n");
    SDL_RWops *rw =
        SDL_RWFromMem(default_background_image, default_background_image_len);
    state->background_image = IMG_Load_RW(rw, 1);
  } else if (strlen(background_image_filepath) &&
      access(background_image_filepath, R_OK) != -1) {
    log_event("Loading background image `%s`\n",
            background_image_filepath);
    state->background_image = IMG_Load(background_image_filepath);
  }
}

void first_render(Config *config, State* state) {
  set_background_image(state, config->background_image_filepath);
  set_title(config, state);

  state->background_color =
      SDL_MapRGB(state->screen->format, config->background_color.r,
                 config->background_color.g, config->background_color.b);

  state->menu_item_height = state->font_pixel_height + MENU_ITEM_PADDING;
  state->menu_height = config->screen_height - (config->top_padding + config->bottom_padding + state->title_height);
  state->menu_max_items = state->menu_height / state->menu_item_height;

  log_event("top padding= %d", config->top_padding);
  log_event("bottom padding= %d", config->bottom_padding);
  log_event("menu height = %d", state->menu_height);
  log_event("menu_item_height = %d",  state->menu_item_height);
  log_event("max_menu_items = %d",  state->menu_max_items);

  render(config, state);
}

void signal_handler(int signal_number) {
  fprintf(stderr, "Caught signal %d\n", signal_number);
  if (signal_number == SIGINT || signal_number == SIGTERM) {
    quit(signal_number);
  }
}

void event_loop(Config* config, State* state) {
  log_event("SDL waiting for event");
  SDL_Event event;
  int poll_result;

  Uint32 last_event_at = 0;
  Uint32 last_repeat_at = 0;
  Uint32 time_since_last_event = 0;
  Uint32 time_since_last_repeat = 0;

  first_render(config, state);

  while (1) {
    poll_result = sdl.poll_event(&event);

    if (poll_result) {
      last_event_at = sdl.get_ticks();
      log_event("Event type: %d", event.type);
      if (handle_input(state, event)) {
        render(config, state);
      }
    }

    time_since_last_event = sdl.get_ticks() - last_event_at;

    if (!poll_result && state->button_repeat_active) {
      time_since_last_repeat = sdl.get_ticks() - last_repeat_at;

      if (time_since_last_event > BUTTON_REPEAT_DELAY_MS &&
          time_since_last_repeat > BUTTON_REPEAT_INTERVAL) {
        handle_input(state, event);
        render(config, state);
      }
    }

    if (config->user_inactivity_timeout_ms && !poll_result && time_since_last_event > config->user_inactivity_timeout_ms) {
      timeout(config->user_inactivity_timeout_ms);
    }
  }
}

void goose_setup(Config *config, State *state) {
  set_log_target_by_filepath(config->log_filepath);

  log_event("HONK HONK");
  log_event("Setting starting selection to %d", config->start_at_nth - 1);
  state->selected_index = config->start_at_nth - 1;

  log_event("Reading menu items");
  if (!state->menu_items) {
    state->menu_items = read_lines_from_stdin(MAX_MENU_ITEMS, MAX_LINE_LENGTH);
  }

  if (state->menu_items->count < 1) {
    log_event("No menu items on stdin");
    quit(1);
  }

  log_event("Menu items count=%d", state->menu_items->count);
  log_event("  first =%s", state->menu_items->lines[0]);
  log_event("  last  =%s", state->menu_items->lines[state->menu_items->count - 1]);

  log_event("SDL starting");
  init_sdl(config, state);
}

int main(int argc, char **argv) {
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  Config* config = default_config();
  parse_command_line_options(argc, argv, config);

  State* state = init_state();

  goose_setup(config, state);
  event_loop(config, state);

  log_event("Done");
  cleanup();
  return 0;
}
