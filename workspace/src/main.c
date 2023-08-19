#include "SDL/SDL_events.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "SDL/SDL_video.h"
#include "config.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define BITS_PER_PIXEL 32
#define MENU_ITEM_HEIGHT 25
#define MENU_ITEM_WIDTH 600
#define FONT_SIZE 18

#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_SELECT 7
#define BUTTON_START 8
#define BUTTON_MENU 9

SDL_Surface *screen;
TTF_Font *font = NULL;
SDL_Joystick *joystick = NULL;

int selected_index = 0;
char **menu_items;
int menu_items_count = 0;

void terminate_at_file_extension(char *filename) {
  char *dot = strrchr(filename, '.');
  if (dot) {
    *dot = '\0';
  }
}

void terminate_at_new_line(char *string) {
  char *newline = strchr(string, '\n');
  if (newline) {
    *newline = '\0';
  }
}

char **read_lines_from_stdin(int max_line_length) {
  max_line_length = max_line_length ? max_line_length : 255;
  char **lines = malloc(sizeof(char *));
  lines[0] = malloc(max_line_length * sizeof(char));

  int i = 0;
  while (fgets(lines[i], max_line_length, stdin)) {
    terminate_at_new_line(lines[i]);
    i++;
    lines = realloc(lines, (i + 1) * sizeof(char *));
    lines[i] = malloc(max_line_length * sizeof(char));
  }
  lines[i] = NULL;

  return lines;
}

FILE *log_target() {
  if (isatty(fileno(stderr))) {
    return stderr;
  } else {
    return fopen("event_log.txt", "a");
  }
}

void log_event(const char *format, ...) {
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
  // fclose(output);
}

void initSDL() {
  char *font_path = "assets/font.ttf";
  int font_height;

  log_event("SDL initialization started.");
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();
  SDL_ShowCursor(SDL_DISABLE);
  log_event("Font loading from %s", font_path);
  font = TTF_OpenFont(font_path, FONT_SIZE);
  font_height = TTF_FontHeight(font);
  log_event("Font loaded size=%d, font_height=%dpx", FONT_SIZE, font_height);

  log_event("SDL_SetVideoMode(SCREEN_WIDTH=%d, SCREEN_HEIGHT=%d, bpp=%d)",
            SCREEN_WIDTH, SCREEN_HEIGHT, BITS_PER_PIXEL);
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, BITS_PER_PIXEL,
                            SDL_SWSURFACE);

  SDL_WM_SetCaption("Choose Goose", NULL);
}

void cleanup() {
  if (joystick) {
    SDL_JoystickClose(joystick);
  }
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_Quit();
}

void quit(int exit_code) {
  cleanup();
  exit(exit_code);
}

struct SDL_Surface create_menu_item(Config config, char *text, int selected) {
  SDL_Color unselectedColor;
  SDL_Color selectedColor;

  unselectedColor.r = config.text_color.r;
  unselectedColor.g = config.text_color.g;
  unselectedColor.b = config.text_color.b;
  selectedColor.r = config.text_selected_color.r;
  selectedColor.g = config.text_selected_color.g;
  selectedColor.b = config.text_selected_color.b;

  SDL_Surface *tempSurface;

  if (selected) {
    tempSurface = TTF_RenderText_Solid(font, text, selectedColor);
  } else {
    tempSurface = TTF_RenderText_Solid(font, text, unselectedColor);
  }
  return *SDL_DisplayFormat(tempSurface);
}

void menu_move_selection(int increment) {
  log_event("Moving from %d by %d with %d menu items", selected_index,
            increment, menu_items_count);
  selected_index = (selected_index + increment) % menu_items_count;
  if (selected_index < 0) {
    selected_index = menu_items_count - 1;
  }
  log_event("Moved to %d with %d menu items", selected_index);
}

void menu_confirm() {
  log_event("Selection confirmed item=%d/%d - `%s`", selected_index,
            menu_items_count, menu_items[selected_index]);
  quit(0);
}

void handle_dpad(SDL_JoyHatEvent event) {
  log_event("D-Pad movement: hat %d, value: %d", event.hat, event.value);
  switch (event.value) {
  case SDL_HAT_UP:
    menu_move_selection(-1);
    break;
  case SDL_HAT_DOWN:
    menu_move_selection(1);
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
    menu_move_selection(-1);
    break;
  case SDLK_DOWN:
    menu_move_selection(1);
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

void handleInput(SDL_Event event) {
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
  }
}

void render(Config config) {
  SDL_FillRect(screen, NULL,
               SDL_MapRGB(screen->format, config.background_color.r,
                          config.background_color.g,
                          config.background_color.b));

  int v_padding = 10;
  int visible_menu_item_count =
      (SCREEN_HEIGHT - (v_padding * 2)) / MENU_ITEM_HEIGHT;

  for (int i = 0; i < visible_menu_item_count; i++) {
    if (menu_items[i] == NULL)
      break;

    char *text = menu_items[i];
    SDL_Rect dest;
    dest.x = 10;
    dest.y = v_padding + (MENU_ITEM_HEIGHT * i);
    dest.w = MENU_ITEM_WIDTH;
    dest.h = MENU_ITEM_HEIGHT;

    SDL_Surface menu_item = create_menu_item(config, text, i == selected_index);
    SDL_BlitSurface(&menu_item, NULL, screen, &dest);
  }

  SDL_Flip(screen);
}

int main(int argc, char **argv) {
  Config config;

  config_set_defaults(&config);
  /* config_load("./config.txt", &global_config); */

  log_event("Starting up");

  log_event("Reading menu items");
  menu_items = read_lines_from_stdin(0);
  while (menu_items[menu_items_count]) {
    menu_items_count++;
  }
  log_event("Read menu items count=%d", menu_items_count);

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

  while (1) {
    while (SDL_PollEvent(&event)) {
      handleInput(event);
    }
    render(config);
  }

  return 0;
}
