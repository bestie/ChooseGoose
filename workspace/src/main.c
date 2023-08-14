#include "SDL/SDL_events.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "config.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MENU_ITEM_COUNT 3
#define MENU_ITEM_HEIGHT 50
#define MENU_ITEM_WIDTH 200

typedef enum { MENU_OPTION_1, MENU_OPTION_2, MENU_OPTION_3 } MenuItem;

#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_SELECT 7
#define BUTTON_START 8
#define BUTTON_MENU 9

SDL_Surface *screen;
SDL_Surface *menuItems[MENU_ITEM_COUNT];
MenuItem selectedOption = MENU_OPTION_1;
TTF_Font *font = NULL;
SDL_Joystick *joystick = NULL;

static int global_config() {
  Config config = { };

  return *config;
}

void log_event(const char *format, ...) {
  // Open a log file for appending
  FILE *logFile = fopen("event_log.txt", "a");
  if (!logFile)
    return;

  // Get the current time
  time_t currentTime;
  struct tm *timeInfo;
  char timeBuffer[20];

  time(&currentTime);
  timeInfo = localtime(&currentTime);
  strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeInfo);

  // Get the process ID
  pid_t processID = getpid();

  // Write the timestamp and process ID to the log
  fprintf(logFile, "[%s][PID: %d] ", timeBuffer, processID);

  // Write the actual log message
  va_list args;
  va_start(args, format);
  vfprintf(logFile, format, args);
  va_end(args);

  fprintf(logFile, "\n"); // Add a newline for better readability
  fclose(logFile);
}

void initSDL() {
  log_event("my_program.log", "SDL initialization started.");
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();
  SDL_ShowCursor(SDL_DISABLE);
  font =
      TTF_OpenFont("assets/font.ttf", 24); // Adjust the path and size as needed
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

void loadMenuItems() {
  fprintf(stderr, "load menu: text color r = `%d`", global_config().text_color.r);
  fprintf(stderr, "load menu: text color g = `%d`", global_config().text_color.g);
  fprintf(stderr, "load menu: text color b = `%d`", global_config().text_color.b);

  SDL_Color unselectedColor = {
    global_config().text_color.r,
    global_config().text_color.g,
    global_config().text_color.b,
  };

  /* printf("setting selectedColor %d %d %d",  */
  /*     global_config().text_selected_color.r, */
  /*     global_config().text_selected_color.g, */
  /*     global_config().text_selected_color.b */
  /*   ); */

  SDL_Color selectedColor = {
      global_config().text_selected_color.r,
      global_config().text_selected_color.g,
      global_config().text_selected_color.b,
  };

  SDL_Surface *tempSurface;

  for (int i = 0; i < MENU_ITEM_COUNT; i++) {
    char text[50];
    snprintf(text, sizeof(text), "Option %d", i + 1);
    if (i == selectedOption) {
      tempSurface = TTF_RenderText_Solid(font, text, selectedColor);
    } else {
      tempSurface = TTF_RenderText_Solid(font, text, unselectedColor);
    }
    menuItems[i] = SDL_DisplayFormat(tempSurface);
    SDL_FreeSurface(tempSurface);
  }
}

void menu_down() {
  if (selectedOption > 0) {
    selectedOption--;
  }
}

void menu_up() {
  if (selectedOption < MENU_ITEM_COUNT - 1) {
    selectedOption++;
  }
}
void menu_confirm() {
  printf("Selected option: %d\n", selectedOption + 1);
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

void render() {
  SDL_FillRect(screen, NULL,
               SDL_MapRGB(screen->format, global_config().background_color.r,
                          global_config().background_color.g,
                          global_config().background_color.b));

  for (int i = 0; i < MENU_ITEM_COUNT; i++) {
    SDL_Rect dest;
    dest.x = (SCREEN_WIDTH - MENU_ITEM_WIDTH) / 2;
    dest.y = (SCREEN_HEIGHT - (MENU_ITEM_COUNT * MENU_ITEM_HEIGHT)) / 2 +
             (i * MENU_ITEM_HEIGHT);
    dest.w = MENU_ITEM_WIDTH;
    dest.h = MENU_ITEM_HEIGHT;

    SDL_BlitSurface(menuItems[i], NULL, screen, &dest);
  }

  SDL_Flip(screen);
}

int main(int argc, char **argv) {
  fprintf(stderr, "Setting defaults\n");
  config_set_defaults(&global_config());
  fprintf(stderr, "Defaults set\n");
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

  /* fprintf(stderr, "text color r = `%d`", global_config().text_color.r); */
  /* fprintf(stderr, "text color g = `%d`", global_config().text_color.g); */
  /* fprintf(stderr, "text color b = `%d`", global_config().text_color.b); */

  while (1) {
    while (SDL_PollEvent(&event)) {
      handleInput(event);
    }

    loadMenuItems();
    render();
  }

  return 0;
}
