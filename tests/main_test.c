#include <criterion/criterion.h>
#include <SDL/SDL_events.h>
#include <SDL/SDL_events.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
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

int mock_sdl_enable_key_repeat(int delay, int interval) {
    printf("Mock SDL_EnableKeyRepeat called\n");
    return 0;
}

static int interval_ms = 200;
static int tick_clock = 0;
static int get_ticks_count = 0;

typedef void (*TestHookFn)(void);

static void noop(void) {
}

typedef struct {
    TestHookFn pre_tick;
    TestHookFn on_poll;
    TestHookFn on_frame;
} TestHooks;

static TestHooks test_hooks = { .pre_tick = noop, .on_frame = noop, .on_poll = noop };

Uint32 mock_sdl_get_ticks(void) {
    test_hooks.pre_tick();

    get_ticks_count++;
    tick_clock = get_ticks_count * interval_ms;  // Arbitrary timestamp value

    usleep(interval_ms * 1000);

    return tick_clock;
}

typedef struct {
    SDL_Event events[256];
    int count;
    int index;
} EventQueue;

static EventQueue input_q = { .count = 0, .index = 0 };

typedef struct {
    SDL_Event up_arrow;
    SDL_Event down_arrow;
    SDL_Event left_arrow;
    SDL_Event right_arrow;
    SDL_Event enter_key;
    SDL_Event esc_key;
} InputValues;

static const InputValues INPUTS = {
    .up_arrow = {
        .key = { .type = SDL_KEYDOWN, .keysym = { .sym = SDLK_UP } }
    },
    .down_arrow = {
        .key = { .type = SDL_KEYDOWN, .keysym = { .sym = SDLK_DOWN } }
    },
    .left_arrow = {
        .key = { .type = SDL_KEYDOWN, .keysym = { .sym = SDLK_LEFT } }
    },
    .right_arrow = {
        .key = { .type = SDL_KEYDOWN, .keysym = { .sym = SDLK_RIGHT } }
    },
    .enter_key = {
        .key = { .type = SDL_KEYDOWN, .keysym = { .sym = SDLK_RETURN } }
    },
    .esc_key = {
        .key = { .type = SDL_KEYDOWN, .keysym = { .sym = SDLK_ESCAPE } }
    },
};

int mock_sdl_poll_event(SDL_Event *event) {
    printf("Mock SDL_PollEvent called %d\n", input_q.index);

    if (input_q.index >= input_q.count) {
        printf("Mock SDL_PollEvent: No More inputs!!!");
        return 0;
    }

    SDL_Event next_event = input_q.events[input_q.index];
    printf("👇 sending next key event %d\n", input_q.index);

    event->type = SDL_KEYDOWN;
    event->key.keysym.sym = next_event.key.keysym.sym;

    input_q.index++;
    return 1;
}

static int frame_count = 0;

int mock_sdl_flip(SDL_Surface *screen) {
    SDL_Flip(screen);

    frame_count++;

    test_hooks.on_frame();
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

SDL_Surface* text_surfaces[1024];
char* text_surface_texts[1024];

SDL_Surface* mock_ttf_rendertext_blended(TTF_Font *font, const char *text, SDL_Color fg) {
    SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, fg);
    int i=0;
    while(text_surfaces[i] != NULL) i++;
    text_surfaces[i] = text_surface;
    text_surface_texts[i] = strdup(text);
    return text_surface;
}

SDL_Interface* get_mock_sdl_interface(void) {
    SDL_Interface* interface = malloc(sizeof(SDL_Interface));

    interface->init = mock_sdl_init;
    interface->quit = mock_sdl_quit;
    interface->set_video_mode = mock_sdl_set_video_mode;
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
    interface->ttf_rendertext_blended = mock_ttf_rendertext_blended;

    return interface;
}

void generate_test_menu_items(State* state, int item_count) {
    state->menu_items = malloc(sizeof(BunchOfLines));
    state->menu_items->max_length = 255;

    state->menu_items->count = item_count;
    state->menu_items->lines = malloc(sizeof(char*) * item_count);

    for(int i=0; i < item_count; i++) { 
        char* str = malloc(sizeof(char*) * state->menu_items->max_length);
        sprintf(str, "Item %d", i);
        state->menu_items->lines[i] = str;
    }
}

// Config and state struct
typedef struct ConfigAndState {
    Config* config;
    State* state;
} ConfigAndState;

char captured_stdout[255];

void* start_app(void* arg) {
    ConfigAndState* config_and_state = arg;
    Config* config = config_and_state->config;
    State* state = config_and_state->state;

    goose_setup(config, state);

    int pipefd[2];
    pipe(pipefd);
    set_output(fdopen(pipefd[1], "w"));

    event_loop(config, state);
    cleanup();

    FILE* pipe_read = fdopen(pipefd[0], "r");

    fgets(captured_stdout, sizeof(captured_stdout), pipe_read);
    printf("buf baby = %s\n", captured_stdout);

    return NULL;
}

void tick_tick_boom(void) {
    printf("Tick tick %d 💣\n", tick_clock);
}

void count_frame(void) {
    printf("🖼 rendered frame n = %d\n", frame_count);
}

void enqueue_input(SDL_Event event) {
    input_q.events[input_q.count] = event;
    input_q.count++;
}

ConfigAndState* setup(void) {
    ConfigAndState* config_and_state = malloc(sizeof(ConfigAndState));

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

    config_and_state->config = config;
    config_and_state->state = state;
    return config_and_state;
}

Test(main_tests, test_init) {
    ConfigAndState* config_and_state = setup();

    enqueue_input(INPUTS.down_arrow);
    enqueue_input(INPUTS.down_arrow);
    enqueue_input(INPUTS.enter_key);

    test_hooks.pre_tick = tick_tick_boom;
    test_hooks.on_frame = count_frame;

    start_app(config_and_state);

    cr_assert_str_eq(captured_stdout, "Item 2\n");
}

Test(main_tests, test_escape_quit) {
    ConfigAndState* config_and_state = setup();

    enqueue_input(INPUTS.down_arrow);
    enqueue_input(INPUTS.esc_key);

    test_hooks.pre_tick = tick_tick_boom;
    test_hooks.on_frame = count_frame;

    start_app(config_and_state);

    cr_assert_str_eq(captured_stdout, "");
}
