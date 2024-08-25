#include <SDL/SDL_events.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "background_image.c"
#include "cli_opts.h"
#include "font.c"

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

SDL_Surface *screen;
SDL_Surface *background_image;
SDL_Surface *title;
TTF_Font *title_font = NULL;
TTF_Font *font = NULL;
SDL_Joystick *joystick = NULL;

typedef struct {
  int count;
  int max_length;
  char **lines;
  char *first;
  char *last;
} BunchOfLines;

BunchOfLines menu_items;
Uint32 background_color;
Config config;

FILE *log_file;
int title_height;
int menu_item_height;
int menu_height;
int menu_max_items;
int font_pixel_height;
int selected_index = 0;
int button_repeat_active = 0;
int button_repeat_first = 1;

void terminate_at_file_extension(char *filename) {
  char *dot = strrchr(filename, '.');
  if (dot) {
    *dot = '\0';
  }
}

void set_log_target() {
  if (config.logging_enabled) {
    if (strlen(config.log_filepath) > 0) {
      log_file = fopen(config.log_filepath, "a");
    }
    if (!log_file) {
      log_file = stderr;
    }
  }
}

void log_event(const char *format, ...) {
  if (!log_file) {
    return;
  }


  FILE *output = log_file;
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

  fprintf(output, "[%s][PID: %d] %s\n", time_buffer, processID, message);
  fflush(output);
}

BunchOfLines read_lines_from_stdin() {
  int max_lines = MAX_MENU_ITEMS;
  int max_line_length = MAX_LINE_LENGTH;

  size_t total_memory =
      max_lines * sizeof(char *) + max_lines * max_line_length;

  char **lines = malloc(total_memory);

  char *line_memory = (char *)(lines + max_lines);
  for (int i = 0; i < max_lines; i++) {
    lines[i] = line_memory + i * max_line_length;
  }

  int lines_i = 0;
  while (lines_i < max_lines && fgets(lines[lines_i], max_line_length, stdin)) {
    lines[lines_i][strcspn(lines[lines_i], "\n")] = '\0';
    lines_i++;
  }

  BunchOfLines bunch = {
      .count = lines_i,
      .max_length = max_line_length,
      .lines = lines,
      .first = lines[0],
      .last = lines[lines_i - 1],
  };

  return bunch;
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

void initSDL() {
  log_event("SDL initialization started.");
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();
  SDL_ShowCursor(SDL_DISABLE);

  title_font = load_font(config.font_filepath, config.title_font_size);
  font = load_font(config.font_filepath, config.font_size);

  font_pixel_height = TTF_FontHeight(font);

  log_event("Font loaded size=%d, font_height=%dpx", config.font_size,
            font_pixel_height);
  log_event("SDL_SetVideoMode(SCREEN_WIDTH=%d, SCREEN_HEIGHT=%d, bpp=%d)",
            config.screen_width, config.screen_height, config.bits_per_pixel);

  screen = SDL_SetVideoMode(config.screen_width, config.screen_height,
                            config.bits_per_pixel, SDL_SWSURFACE);

  SDL_WM_SetCaption(config.title, NULL);
  SDL_EnableKeyRepeat(400, 50);
}

void cleanup() {
  SDL_FreeSurface(background_image);
  SDL_FreeSurface(screen);
  if (joystick) {
    SDL_JoystickClose(joystick);
  }
  if (title_font) {
    TTF_CloseFont(title_font);
  }
  if (font) {
    TTF_CloseFont(font);
  }
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  fclose(log_file);
}

void quit(int exit_code) {
  log_event("Shutting down gracefully.");
  cleanup();
  exit(exit_code);
}

void timeout() {
  log_event("Inactivity timeout reached - %ds", config.user_inactivity_timeout_ms / 1000);
  cleanup();
  exit(124);
}

SDL_Surface *create_text_surface(char *text, Color color, TTF_Font *font) {
  SDL_Color text_color = {color.r, color.g, color.b};

  SDL_Surface *text_surface = TTF_RenderText_Blended(font, text, text_color);
  return text_surface;
}

SDL_Surface *create_menu_item(char *text, int selected) {
  Color text_color;

  if (selected) {
    text_color = config.text_selected_color;
  } else {
    text_color = config.text_color;
  }

  SDL_Surface *text_surface = create_text_surface(text, text_color, font);

  if (selected && config.text_selected_background_color.r != NULL) {
    Color bg = config.text_selected_background_color;
    Uint32 sdl_bg_color = SDL_MapRGB(screen->format, bg.r, bg.g, bg.b);
    int menu_item_width = config.screen_width;
    SDL_Surface *text_bg_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, menu_item_width, font_pixel_height, config.bits_per_pixel, 0,0,0,0);
    SDL_Rect fillRect = { 0, 0, text_bg_surface->w, text_bg_surface->h };
    SDL_FillRect(text_bg_surface, &fillRect, sdl_bg_color);

    SDL_BlitSurface(text_surface, NULL, text_bg_surface, NULL);
    SDL_FreeSurface(text_surface);
    text_surface = text_bg_surface;
  }

  return text_surface;
}

void menu_move_selection(int increment, int cycle) {
  int from = selected_index;
  selected_index = selected_index + increment;
  if (cycle) {
    selected_index = selected_index % menu_items.count;
    if (selected_index < 0) {
      selected_index = menu_items.count - 1;
    }
  }

  if (selected_index < 0) {
    selected_index = 0;
  } else if (selected_index > menu_items.count) {
    selected_index = menu_items.count - 1;
  }

  log_event("Moved to from %d to %d", from, selected_index);
}

void menu_confirm() {
  log_event("Selection confirmed item=%d/%d - `%s`", selected_index,
            menu_items.count, menu_items.lines[selected_index]);
  fprintf(stdout, "%s\n", menu_items.lines[selected_index]);
  quit(0);
}

void enable_repeat() {
  button_repeat_active = 1;
  button_repeat_first = 1;
}

void disable_repeat() { button_repeat_active = 0; }

void handle_dpad(SDL_JoyHatEvent event) {
  // log_event("D-Pad movement: hat %d, value: %d", event.hat, event.value);
  switch (event.value) {
  case SDL_HAT_UP:
    menu_move_selection(-1, 1);
    enable_repeat();
    break;
  case SDL_HAT_DOWN:
    menu_move_selection(1, 1);
    enable_repeat();
    break;
  case SDL_HAT_LEFT:
    menu_move_selection(-menu_max_items, 0);
    enable_repeat();
    break;
  case SDL_HAT_RIGHT:
    menu_move_selection(menu_max_items, 0);
    enable_repeat();
    break;
  case SDL_HAT_CENTERED:
    disable_repeat();
    break;
  default:
    break;
  }
}

void handle_joypad_button(SDL_JoyButtonEvent event) {
  log_event("Joypad button pressed: button %d", event.button);
  switch (event.button) {
  case BUTTON_A:
    menu_confirm();
    break;
  case BUTTON_START:
    menu_confirm();
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

void handle_key_press(SDL_Event event) {
  log_event("Keyboard keypress: value: %d", event.key.keysym.sym);
  switch (event.key.keysym.sym) {
  case SDLK_UP:
    menu_move_selection(-1, 1);
    break;
  case SDLK_DOWN:
    menu_move_selection(1, 1);
    break;
  case SDLK_LEFT:
    menu_move_selection(-menu_max_items, 0);
    break;
  case SDLK_RIGHT:
    menu_move_selection(menu_max_items, 0);
    break;
  case SDLK_RETURN:
    menu_confirm();
    break;
  case SDLK_ESCAPE:
    quit(0);
    break;
  default:
    break;
  }
}

int handleInput(SDL_Event event) {
  int event_handled = 1;

  switch (event.type) {
  case SDL_JOYHATMOTION:
    handle_dpad(event.jhat);
    break;
  case SDL_JOYBUTTONDOWN:
    handle_joypad_button(event.jbutton);
    break;
  case SDL_KEYDOWN:
    handle_key_press(event);
    break;
  default:
    event_handled = 0;
  }
  return event_handled;
}

void set_title() {
  if (strlen(config.title)) {
    title = create_text_surface(config.title, config.text_color, title_font);
    title_height = title->h + MENU_ITEM_PADDING * 2;
  } else {
    title_height = 0;
  }
}

void render() {
  SDL_FillRect(screen, NULL, background_color);
  if (background_image) {
    SDL_BlitSurface(background_image, NULL, screen, NULL);
  }
  if (title) {
    SDL_Rect dest = {config.left_padding, config.top_padding, 0, 0};
    SDL_BlitSurface(title, NULL, screen, &dest);
  }

  int menu_y_offset = config.top_padding + title_height;

  int default_items_above = menu_max_items / 2;
  int default_items_below = menu_max_items - default_items_above - 1;

  int visible_menu_start = selected_index - default_items_above;
  int visible_menu_end = selected_index + default_items_below;

  // Not enough items above cursor, lock window to top of list
  if (visible_menu_start < 0) {
    visible_menu_start = 0;
    visible_menu_end = visible_menu_start + menu_max_items - 1;
  }

  // Not enough items below cursor lock window to bottom of list
  if (visible_menu_end > menu_items.count - 1) {
    visible_menu_end = menu_items.count - 1;
    visible_menu_start = visible_menu_end - menu_max_items + 1;
  }

  // Final constraint check that neither are out of bounds
  if (visible_menu_start < 0 || visible_menu_start > (menu_items.count - 1)) {
    visible_menu_start = 0;
  }
  if (visible_menu_end > menu_items.count - 1 || visible_menu_end < 0) {
    visible_menu_end = menu_items.count - 1;
  }

  int menu_index = 0;

  for (int i = 0; i < menu_max_items; i++) {
    if ((visible_menu_start + i) == visible_menu_end) {
      break;
    }

    menu_index = visible_menu_start + i;

    char text[255];
    if (config.prefix_with_number) {
      sprintf(text, "%3d. %s", menu_index + 1, menu_items.lines[menu_index]);
    } else {
      sprintf(text, "%s", menu_items.lines[menu_index]);
    }
    if (config.hide_file_extensions) {
      terminate_at_file_extension(text);
    }

    int selected_state = selected_index == menu_index;

    SDL_Rect dest;
    dest.x = config.left_padding;
    dest.y = menu_y_offset + i * menu_item_height;
    dest.w = 0;
    dest.h = 0;

    SDL_Surface *menu_item = create_menu_item(text, selected_state);
    SDL_BlitSurface(menu_item, NULL, screen, &dest);
    SDL_FreeSurface(menu_item);
  }

  SDL_Flip(screen);
}

void set_background_image() {
  log_event("Background image file path is `%s`\n",
          config.background_image_filepath);
  if (strcmp(config.background_image_filepath, "none") == 0) {
    return;
  }

  if (strlen(config.background_image_filepath) &&
      access(config.background_image_filepath, R_OK) != -1) {
    log_event("Loading background image `%s`\n",
            config.background_image_filepath);
    background_image = IMG_Load(config.background_image_filepath);
  } else {
    log_event("No background image using default\n");
    SDL_RWops *rw =
        SDL_RWFromMem(default_background_image, default_background_image_len);
    background_image = IMG_Load_RW(rw, 1);
  }
}

void first_render() {
  set_background_image();
  set_title();

  background_color =
      SDL_MapRGB(screen->format, config.background_color.r,
                 config.background_color.g, config.background_color.b);

  menu_item_height = font_pixel_height + MENU_ITEM_PADDING;
  menu_height =
      config.screen_height - (config.top_padding + config.bottom_padding + title_height);
  menu_max_items = menu_height / menu_item_height;

  log_event("top padding= %d", config.top_padding);
  log_event("bottom padding= %d", config.bottom_padding);
  log_event("menu height = %d", menu_height);
  log_event("menu_item_height = %d",  menu_item_height);
  log_event("max_menu_items = %d",  menu_max_items);

  render();
}

void signal_handler(int signal_number) {
  fprintf(stderr, "Caught signal %d\n", signal_number);
  if (signal_number == SIGINT || signal_number == SIGTERM) {
    quit(signal_number);
  }
}

int main(int argc, char **argv) {
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  config_set_defaults(&config);
  parse_command_line_options(argc, argv, &config);
  set_log_target();

  log_event("HONK HONK");
  log_event("Setting starting selection to %d", config.start_at_nth - 1);
  selected_index = config.start_at_nth - 1;

  log_event("Reading menu items");
  menu_items = read_lines_from_stdin();

  if (menu_items.count < 1) {
    log_event("No menu items on stdin");
    quit(1);
  }

  log_event("Menu items count=%d", menu_items.count);
  log_event("  first =%s", menu_items.lines[0]);
  log_event("  last  =%s", menu_items.lines[menu_items.count - 1]);

  log_event("SDL starting");
  initSDL();
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    log_event("Unable to init SDL: %s\n", SDL_GetError());
    quit(1);
  }

  SDL_Joystick *joystick = NULL;
  if (SDL_NumJoysticks() > 0) {
    joystick = SDL_JoystickOpen(0);
    if (joystick == NULL) {
      log_event("Unable to open joystick: %s\n", SDL_GetError());
      quit(1);
    }
  }

  log_event("SDL waiting for event");
  SDL_Event event;
  int poll_result;

  Uint32 last_event_at = 0;
  Uint32 last_repeat_at = 0;
  Uint32 time_since_last_event = 0;
  Uint32 time_since_last_repeat = 0;

  first_render();

  while (1) {
    poll_result = SDL_PollEvent(&event);

    if (poll_result) {
      last_event_at = SDL_GetTicks();
      if (handleInput(event)) {
        render();
      }
    }

    time_since_last_event = SDL_GetTicks() - last_event_at;

    if (!poll_result && button_repeat_active) {
      time_since_last_repeat = SDL_GetTicks() - last_repeat_at;

      if (time_since_last_event > BUTTON_REPEAT_DELAY_MS &&
          time_since_last_repeat > BUTTON_REPEAT_INTERVAL) {
        handleInput(event);
        render();
      }
    }

    if (config.user_inactivity_timeout_ms && !poll_result && time_since_last_event > config.user_inactivity_timeout_ms) {
      timeout();
    }
  }

  return 0;
}
