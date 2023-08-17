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

char **read_lines_from_stdin() {
  int i = 0;
  int max_len = 255;
  char **lines = malloc(sizeof(char *));
  lines[0] = malloc(max_len * sizeof(char));

  while (fgets(lines[i], max_len, stdin)) {
    terminate_at_new_line(lines[i]);
    i++;
    lines = realloc(lines, (i + 1) * sizeof(char *));
    lines[i] = malloc(max_len * sizeof(char));
  }
  lines[i] = 0;

  return lines;
}

void log_event(const char *format, ...) {
  // Open a log file for appending
  FILE *logFile = fopen("event_log.txt", "a");
  if (!logFile)
    return;

  time_t currentTime;
  struct tm *timeInfo;
  char timeBuffer[20];

  time(&currentTime);
  timeInfo = localtime(&currentTime);
  strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeInfo);

  // Get the process ID
  pid_t processID = getpid();

  fprintf(logFile, "[%s][PID: %d] ", timeBuffer, processID);

  va_list args;
  va_start(args, format);
  vfprintf(logFile, format, args);
  va_end(args);

  fprintf(logFile, "\n");
  fclose(logFile);
}

void initSDL() {
  log_event("my_program.log", "SDL initialization started.");
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();
  SDL_ShowCursor(SDL_DISABLE);
  font = TTF_OpenFont("assets/font.ttf", FONT_SIZE);
  int height;
  height = TTF_FontHeight(font);
  fprintf(stderr, "font height = %d", height);
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_SWSURFACE);
  SDL_WM_SetCaption("Simple Menu", NULL);
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

void menu_down() {
  if (selected_index > 0) {
    selected_index--;
  }
}

void menu_up() {
  // if (selected_index < sizeof(menu_items) - 1) {
  selected_index++;
  // }
}

void menu_confirm() {
  printf("Selected option: %s\n", menu_items[selected_index]);
  quit(0);
}

void handle_dpad(SDL_JoyHatEvent event) {
  log_event("D-Pad movement: hat %d, value: %d", event.hat, event.value);
  switch (event.value) {
  case SDL_HAT_UP:
    menu_down();
    break;
  case SDL_HAT_DOWN:
    menu_up();
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
    menu_down();
    break;
  case SDLK_DOWN:
    menu_up();
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
    char *text = menu_items[i];
    SDL_Rect dest;
    dest.x = 10;
    dest.y = v_padding + (MENU_ITEM_HEIGHT * i);
    // dest.x = (SCREEN_WIDTH - MENU_ITEM_WIDTH) / 2;
    // dest.y = (SCREEN_HEIGHT - (MENU_ITEM_COUNT * MENU_ITEM_HEIGHT)) / 2 +
    //          (i * MENU_ITEM_HEIGHT);
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
  initSDL();
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    log_event("Unable to init SDL: %s\n", SDL_GetError());
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    quit(1);
  }

  SDL_Joystick *joystick = NULL;
  if (SDL_NumJoysticks() > 0) {
    joystick = SDL_JoystickOpen(0);
    if (joystick == NULL) {
      log_event("Unable to open joystick: %s\n", SDL_GetError());
      fprintf(stderr, "Unable to open joystick: %s\n", SDL_GetError());
      quit(1);
    }
  }

  log_event("Waiting for event");
  SDL_Event event;

  menu_items = read_lines_from_stdin();
  selected_index = 0;

  while (1) {
    while (SDL_PollEvent(&event)) {
      handleInput(event);
    }
    render(config);
  }

  return 0;
}
