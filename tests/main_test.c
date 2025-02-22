#include <criterion/criterion.h>
#include <SDL/SDL.h>
#include <unistd.h>

#include "state.h"
#include "cli_opts.h"
#include "main.h"

// Mock SDL functions
int mock_sdl_init(Uint32 flags) {
    printf("Mock SDL_Init called\n");
    return 0;
}

void mock_sdl_quit(void) {
    printf("Mock SDL_Quit called\n");
}

SDL_Surface* mock_sdl_set_video_mode(int w, int h, int bpp, Uint32 flags) {
    printf("Mock SDL_SetVideoMode called\n");
    return NULL;
}

void mock_sdl_wm_set_caption(const char *title, const char *icon) {
    printf("Mock SDL_WM_SetCaption called: %s\n", title);
}

int mock_sdl_enable_key_repeat(int delay, int interval) {
    printf("Mock SDL_EnableKeyRepeat called\n");
    return 0;
}

int interval_ms = 200;

Uint32 mock_sdl_get_ticks(void) {
    static int call_count = 0;
    call_count++;

    printf("Mock SDL_GetTicks called\n");
    return call_count * interval_ms;  // Arbitrary timestamp value
}

int mock_sdl_poll_event(SDL_Event *event) {
    usleep(interval_ms*1000);  // Simulate a delay
    printf("called mock_sdl_poll_event\n");
    static int call_count = 0;
    call_count++;

    switch (call_count) {
      case 1:
        event->type = SDL_KEYDOWN;
        event->key.keysym.sym = SDLK_DOWN;
        break;
      case 2:
        event->type = SDL_KEYDOWN;
        event->key.keysym.sym = SDLK_DOWN;
        break;
      case 3:
        event->type = SDL_KEYDOWN;
        event->key.keysym.sym = SDLK_RETURN;
        break;
    }

    return 1;
}

int mock_sdl_flip(SDL_Surface *screen) {
    printf("Mock SDL_Flip called\n");
    return 0;
}

int mock_sdl_fill_rect(SDL_Surface *dst, SDL_Rect *rect, Uint32 color) {
    printf("Mock SDL_FillRect called\n");
    return 0;
}

void mock_sdl_free_surface(SDL_Surface *surface) {
    printf("Mock SDL_FreeSurface called\n");
}

void mock_sdl_delay(Uint32 ms) {
    printf("Mock SDL_Delay called for %u ms\n", ms);
}

int mock_sdl_blit_surface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
    printf("Mock SDL_BlitSurface called\n");
    return 0;
}

int mock_sdl_show_cursor(int toggle) {
    printf("Mock SDL_ShowCursor called\n");
    return 0;
}

int mock_sdl_num_joysticks(void) {
    printf("Mock SDL_NumJoysticks called\n");
    return 1;  // Simulate one joystick available
}

char* mock_sdl_get_error(void) {
    return "Mock SDL error message";
}

char* mock_sdl_joystick_name(int device_index) {
    printf("Mock SDL_JoystickName called for device %d\n", device_index);
    return "Mock Joystick";
}

SDL_Joystick* mock_sdl_joystick_open(int device_index) {
    printf("Mock SDL_JoystickOpen called for device %d\n", device_index);
    return (SDL_Joystick*)1;  // Simulate a valid pointer
}

int mock_sdl_ttf_init(void) {
    printf("Mock SDL_TTF_Init called\n");
    return 0;
}

SDL_Interface* get_mock_sdl_interface(void) {
  SDL_Interface* interface = malloc(sizeof(SDL_Interface));

  interface->init = mock_sdl_init;
  interface->quit = mock_sdl_quit;
  interface->set_video_mode = mock_sdl_set_video_mode;
  interface->wm_set_caption = mock_sdl_wm_set_caption;
  interface->enable_key_repeat = mock_sdl_enable_key_repeat;
  interface->poll_event = mock_sdl_poll_event;
  interface->get_ticks = mock_sdl_get_ticks;
  interface->flip = mock_sdl_flip;
  interface->fill_rect = mock_sdl_fill_rect;
  interface->free_surface = mock_sdl_free_surface;
  interface->delay = mock_sdl_delay;
  interface->blit_surface = mock_sdl_blit_surface;
  interface->show_cursor = mock_sdl_show_cursor;
  interface->num_joysticks = mock_sdl_num_joysticks;
  interface->get_error = mock_sdl_get_error;
  interface->joystick_name = mock_sdl_joystick_name;
  interface->joystick_open = mock_sdl_joystick_open;
  interface->ttf_init = mock_sdl_ttf_init;

  return interface;
}

void generate_test_menu_items(State* state) {
  state->menu_items = malloc(sizeof(BunchOfLines));
  state->menu_items->count = 3;
  state->menu_items->max_length = 255;
  state->menu_items->lines = malloc(sizeof(char*) * 3);
  state->menu_items->lines[0] = "Item 1";
  state->menu_items->lines[1] = "Item 2";
  state->menu_items->lines[2] = "Item 3";
}

Test(main_tests, test_init) {
  Config* config = default_config();
  config->user_inactivity_timeout_ms = 10;
  State* state = init_state();
  generate_test_menu_items(state);
  set_log_file_pointer(stderr);
  SDL_Interface* sdl = get_sdl_interface();
  sdl->poll_event = mock_sdl_poll_event;
  sdl->get_ticks = mock_sdl_get_ticks;

  goose_setup(config, state);
  event_loop(config, state);

  /*cr_assert_not_null(state, "init_state() returned NULL");*/
  /*cleanup_state(state);*/
}
