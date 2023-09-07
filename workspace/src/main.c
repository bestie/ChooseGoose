#include <SDL/SDL_events.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "SDL/SDL_keysym.h"
#include "SDL/SDL_video.h"
#include "config.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define BITS_PER_PIXEL 32
#define MENU_ITEM_WIDTH 600
#define MENU_ITEM_PADDING 4

#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_SELECT 7
#define BUTTON_START 8
#define BUTTON_MENU 9

SDL_Surface *screen;
SDL_Surface *background_image;
TTF_Font *font = NULL;
SDL_Joystick *joystick = NULL;

typedef struct {
  int count;
  int max_length;
  char **lines;
  int last_index;
  char *first;
  char *last;
} BunchOfLines;

BunchOfLines menu_items;
Uint32 background_color;
Config config;

int menu_item_height;
int menu_height;
int menu_max_items;
int font_pixel_height;
int selected_index = 0;
int button_repeat_active = 0;

void terminate_at_file_extension(char *filename) {
  char *dot = strrchr(filename, '.');
  if (dot) {
    *dot = '\0';
  }
}

FILE *log_target() {
  // if (isatty(fileno(stderr))) {
  //   return stderr;
  // } else {
    return fopen("events.log", "a");
  // }
}

void log_event(const char *format, ...) {
  if (!config.logging_enabled) {
    return;
  }
  FILE *output = log_target();
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

  if (!isatty(fileno(log_target()))) {
    fclose(output);
  }
}

BunchOfLines read_lines_from_stdin(int max_line_length) {
  fprintf(log_target(), "reading stdin ...\n");
  int max_lines = 4096;
  max_line_length = max_line_length ? max_line_length : 256;

  char **lines =
      malloc(max_lines * sizeof(char *) + max_lines * max_line_length);
  if (!lines) {
    fprintf(log_target(), "mallioc failed \n");
  }

  int lines_i = 0;
  lines[lines_i] = (char *)(lines + (max_line_length * (lines_i + 1)));

  while (fgets(lines[lines_i], max_line_length, stdin)) {
    int buffbaby_size = strlen(lines[lines_i]);
    fprintf(log_target(), "Read stdin reads=%d bytes=%d: %s\n", lines_i,
            buffbaby_size, lines[lines_i]);
    lines[lines_i][buffbaby_size - 1] = '\0';
    lines_i++;
    lines[lines_i] = (char *)(lines + (max_line_length * (lines_i + 1)));
  }
  fprintf(log_target(), "out of loop %d", lines_i);

  lines[lines_i] = NULL;

  BunchOfLines bunch = {
      lines_i,       max_line_length, lines,
      (lines_i - 1), lines[0],        lines[lines_i - 1],
  };

  return bunch;
}

void initSDL() {
  log_event("SDL initialization started.");
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();
  SDL_ShowCursor(SDL_DISABLE);
  log_event("Font loading from %s", config.font_filepath);
  font = TTF_OpenFont(config.font_filepath, config.font_size);
  font_pixel_height = TTF_FontHeight(font);
  log_event("Font loaded size=%d, font_height=%dpx", config.font_size,
            font_pixel_height);

  log_event("SDL_SetVideoMode(SCREEN_WIDTH=%d, SCREEN_HEIGHT=%d, bpp=%d)",
            SCREEN_WIDTH, SCREEN_HEIGHT, BITS_PER_PIXEL);
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, BITS_PER_PIXEL,
                            SDL_SWSURFACE);

  SDL_WM_SetCaption(config.title, NULL);
  SDL_EnableKeyRepeat(400, 50);
}

void cleanup() {
  SDL_FreeSurface(background_image);
  SDL_FreeSurface(screen);
  if (joystick) {
    SDL_JoystickClose(joystick);
  }
  if (font) {
    TTF_CloseFont(font);
  }
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
}

void quit(int exit_code) {
  log_event("Shutting down gracefully.");
  cleanup();
  log_event("Clean up successful.");
  exit(exit_code);
}

SDL_Surface *create_text_surface(char *text, Color color) {
  SDL_Color text_color = {color.r, color.g, color.b};

  SDL_Surface *text_surface = TTF_RenderText_Blended(font, text, text_color);
  return text_surface;
}

SDL_Surface *create_menu_item(char *text, int selected) {
  Color color;
  if (selected) {
    color = config.text_selected_color;
  } else {
    color = config.text_color;
  }

  return create_text_surface(text, color);
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

  // log_event("Moved to from %d to %d", from, selected_index);
}

void menu_confirm() {
  log_event("Selection confirmed item=%d/%d - `%s`", selected_index,
            menu_items.count, menu_items.lines[selected_index]);
  fprintf(stdout, "%s\n", menu_items.lines[selected_index]);
  quit(0);
}

void enable_repeat() {
  button_repeat_active = 1;
}

void disable_repeat() {
  button_repeat_active = 0;
}

void wait_for_next_repeat() {
  if(button_repeat_active) {
    usleep(100 * 1000);
  }
}

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
    log_event("dpad centered");
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
    quit(0);
    break;
  case BUTTON_MENU:
    quit(0);
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

void render() {
  SDL_FillRect(screen, NULL, background_color);
  if (background_image) {
    SDL_BlitSurface(background_image, NULL, screen, NULL);
  }

  int menu_y_offset = config.top_padding;

  if (strlen(config.title)) {
    int header_padding = (config.top_padding / 2) + MENU_ITEM_PADDING;

    SDL_Surface *title_surface =
        create_text_surface(config.title, config.text_color);
    SDL_Rect dest = {config.left_padding, header_padding, SCREEN_WIDTH,
                     menu_item_height};
    SDL_BlitSurface(title_surface, NULL, screen, &dest);

    menu_height -= title_surface->h;
    menu_y_offset += title_surface->h;
    SDL_FreeSurface(title_surface);
  }

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

  int counter = 0;
  for (int i = 0; i < menu_max_items; i++) {
    counter++;
    menu_index = visible_menu_start + i;
    if (menu_items.lines[menu_index] == NULL)
      break;

    char text[255];
    if (config.prefix_with_number) {
      sprintf(text, "%3d. %s", menu_index + 1, menu_items.lines[menu_index]);
    } else {
      sprintf(text, "%s", menu_items.lines[menu_index]);
    }

    int selected_state = selected_index == menu_index;

    SDL_Rect dest;
    dest.x = config.left_padding;
    dest.y = menu_y_offset + i * menu_item_height;
    dest.w = SCREEN_WIDTH;
    dest.h = 0;

    SDL_Surface *menu_item = create_menu_item(text, selected_state);
    SDL_BlitSurface(menu_item, NULL, screen, &dest);
    SDL_FreeSurface(menu_item);
  }

  SDL_Flip(screen);
}

void first_render() {
  if (strlen(config.background_image_filepath)) {
    background_image = IMG_Load(config.background_image_filepath);
  }
  background_color =
      SDL_MapRGB(screen->format, config.background_color.r,
                 config.background_color.g, config.background_color.b);

  menu_item_height = font_pixel_height + MENU_ITEM_PADDING;
  menu_height = SCREEN_HEIGHT - (config.top_padding + config.bottom_padding);
  menu_max_items = menu_height / menu_item_height;

  log_event("menu height = %d, menu_max_itemis=%d", menu_height,
            menu_max_items);
  render();
}

int main(int argc, char **argv) {
  log_event("Starting up");

  config_set_defaults(&config);
  int result = parse_config_yaml_file(&config, "config.yaml");
  if (!result) {
    log_event("Failed to parse config");
    quit(1);
  }

  log_event("Setting starting selection to %d", config.start_at_nth - 1);
  selected_index = config.start_at_nth - 1;

  log_event("Reading menu items");
  menu_items = read_lines_from_stdin(0);
  log_event("Menu items count=%d", menu_items.count);
  log_event("  first=%s", menu_items.lines[0]);
  log_event("   last=%s", menu_items.lines[menu_items.count - 1]);

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

  first_render();
  while (1) {
    poll_result = SDL_PollEvent(&event);
    if (poll_result || button_repeat_active) {
      wait_for_next_repeat();
      if (handleInput(event)) {
        log_event("User input event type=%d", event.type);
        render();
      }
    }
  }

  return 0;
}
