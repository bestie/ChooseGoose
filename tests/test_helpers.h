#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <criterion.h>
#include <string.h>
#include <unistd.h>

#include "state.h"
#include "cli_opts.h"
#include "main.h"

// ─── Globals shared across test files ───
// These are defined in main_test.c, declared extern here for other test files.

typedef void (*TestHookFn)(void);
typedef struct {
    TestHookFn pre_tick;
    TestHookFn on_poll;
    TestHookFn on_frame;
} TestHooks;

typedef struct {
    SDL_Event events[256];
    int count;
    int index;
} EventQueue;

typedef struct ConfigAndState {
    Config* config;
    State* state;
} ConfigAndState;

extern TestHooks test_hooks;
extern EventQueue input_q;
extern char captured_stdout[255];
extern SDL_Surface* text_surfaces[1024];
extern char* text_surface_texts[1024];
extern int frame_count;

// Defined in main_test.c
extern SDL_Interface* get_mock_sdl_interface(void);
extern void generate_test_menu_items(State* state, int item_count);
void* start_app(void* arg);

// ─── filter_query is a global defined in main.c ───
extern char filter_query[128];

// ─── Pixel inspection ───

typedef struct { Uint8 r, g, b; } RGB;

static inline RGB get_pixel_rgb(SDL_Surface *surface, int x, int y) {
    RGB black = {0, 0, 0};
    if (!surface || !surface->pixels) return black;
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return black;

    Uint8 *pixels = (Uint8 *)surface->pixels;
    Uint8 *pixel = pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
    Uint32 pixel_value;

    switch (surface->format->BytesPerPixel) {
        case 4: pixel_value = *(Uint32 *)pixel; break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                pixel_value = pixel[0] << 16 | pixel[1] << 8 | pixel[2];
            else
                pixel_value = pixel[0] | pixel[1] << 8 | pixel[2] << 16;
            break;
        default: return black;
    }

    RGB result;
    SDL_GetRGB(pixel_value, surface->format, &result.r, &result.g, &result.b);
    return result;
}

static inline int color_matches(RGB pixel, Uint8 r, Uint8 g, Uint8 b) {
    return pixel.r == r && pixel.g == g && pixel.b == b;
}

// Check that a pixel at (x, y) on the surface matches the expected color.
// Suitable for checking background fill regions where we know the exact color.
static inline void assert_pixel_color(SDL_Surface *surface, int x, int y,
                                       Uint8 r, Uint8 g, Uint8 b, const char *msg) {
    RGB pixel = get_pixel_rgb(surface, x, y);
    cr_assert(color_matches(pixel, r, g, b),
              "%s: expected (%d,%d,%d) at (%d,%d) got (%d,%d,%d)",
              msg, r, g, b, x, y, pixel.r, pixel.g, pixel.b);
}

// ─── Rendered text tracking ───

// Search text_surface_texts[] for an exact match.
static inline int text_was_rendered(const char *expected) {
    for (int i = 0; i < 1024 && text_surface_texts[i] != NULL; i++) {
        if (strcmp(text_surface_texts[i], expected) == 0) return 1;
    }
    return 0;
}

// Count how many rendered text entries contain `substring`.
static inline int rendered_text_containing(const char *substring) {
    int count = 0;
    for (int i = 0; i < 1024 && text_surface_texts[i] != NULL; i++) {
        if (strstr(text_surface_texts[i], substring) != NULL) count++;
    }
    return count;
}

// ─── State reset ───
// Must be called before each test to prevent cross-test pollution.

static inline void reset_test_state(void) {
    // Reset input queue
    input_q.count = 0;
    input_q.index = 0;

    // Reset frame counter
    frame_count = 0;

    // Reset rendered text tracker
    for (int i = 0; i < 1024; i++) {
        text_surfaces[i] = NULL;
        if (text_surface_texts[i]) {
            free(text_surface_texts[i]);
            text_surface_texts[i] = NULL;
        }
    }

    // Reset filter query (global in main.c)
    memset(filter_query, 0, sizeof(filter_query));

    // Reset captured stdout
    memset(captured_stdout, 0, sizeof(captured_stdout));

    // Reset test hooks to no-ops
    test_hooks.pre_tick = NULL;
    test_hooks.on_poll = NULL;
    test_hooks.on_frame = NULL;
}

// ─── Event builders ───

static inline SDL_Event make_key_event(SDLKey key) {
    SDL_Event e;
    memset(&e, 0, sizeof(e));
    e.type = SDL_KEYDOWN;
    e.key.keysym.sym = key;
    return e;
}

static inline SDL_Event make_joyhat_event(Uint8 value) {
    SDL_Event e;
    memset(&e, 0, sizeof(e));
    e.type = SDL_JOYHATMOTION;
    e.jhat.hat = 0;
    e.jhat.value = value;
    return e;
}

static inline SDL_Event make_joybutton_event(Uint8 button) {
    SDL_Event e;
    memset(&e, 0, sizeof(e));
    e.type = SDL_JOYBUTTONDOWN;
    e.jbutton.button = button;
    return e;
}

static inline void enqueue_input(SDL_Event event) {
    input_q.events[input_q.count] = event;
    input_q.count++;
}

// ─── Setup helpers ───

// Standard setup: 20 items named "Item 0" .. "Item 19", default config, 2s timeout.
// Mocks only poll_event, get_ticks, flip — everything else is real SDL via dummy driver.
static inline ConfigAndState* setup_default(void) {
    reset_test_state();

    ConfigAndState* cs = malloc(sizeof(ConfigAndState));
    Config* config = default_config();
    config->user_inactivity_timeout_ms = 2000;
    State* state = init_state();
    generate_test_menu_items(state, 20);
    set_log_file_pointer(stderr);

    SDL_Interface* sdl = get_sdl_interface();
    SDL_Interface* mock_sdl = get_mock_sdl_interface();
    sdl->poll_event = mock_sdl->poll_event;
    sdl->get_ticks = mock_sdl->get_ticks;
    sdl->flip = mock_sdl->flip;

    cs->config = config;
    cs->state = state;
    return cs;
}

// Setup with a custom config modifier function.
static inline ConfigAndState* setup_with_config(void (*configure)(Config*)) {
    ConfigAndState* cs = setup_default();
    configure(cs->config);
    return cs;
}

// Setup with custom item names (NULL-terminated array of strings).
static inline ConfigAndState* setup_with_items(const char* items[], int count) {
    reset_test_state();

    ConfigAndState* cs = malloc(sizeof(ConfigAndState));
    Config* config = default_config();
    config->user_inactivity_timeout_ms = 2000;
    State* state = init_state();

    state->menu_items = malloc(sizeof(BunchOfLines));
    state->menu_items->max_length = 255;
    state->menu_items->count = count;
    state->menu_items->lines = malloc(sizeof(char*) * count);
    for (int i = 0; i < count; i++) {
        state->menu_items->lines[i] = strdup(items[i]);
    }

    set_log_file_pointer(stderr);

    SDL_Interface* sdl = get_sdl_interface();
    SDL_Interface* mock_sdl = get_mock_sdl_interface();
    sdl->poll_event = mock_sdl->poll_event;
    sdl->get_ticks = mock_sdl->get_ticks;
    sdl->flip = mock_sdl->flip;

    cs->config = config;
    cs->state = state;
    return cs;
}

#endif // TEST_HELPERS_H
