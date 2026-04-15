#include "test_helpers.h"

// ─── Background color fills the screen ───

// File-scoped pointer so hooks can access state without parameters.
static ConfigAndState* current_cs;

static RGB bg_pixel_captured;

// The on_frame hook fires from inside mock_sdl_flip while the surface is live.
// Capture the bottom-right corner pixel here before SDL_Quit frees the surface.
static void capture_bg_corner(void) {
    SDL_Surface* screen = current_cs->state->screen;
    if (screen && screen->pixels) {
        bg_pixel_captured = get_pixel_rgb(screen, screen->w - 1, screen->h - 1);
    }
}

Test(render_tests, background_color_fills_screen) {
    ConfigAndState* cs = setup_default();
    // Default background is 00FFFF (cyan) — set it to something distinctive
    cs->config->background_color->r = 0x33;
    cs->config->background_color->g = 0x66;
    cs->config->background_color->b = 0x99;

    current_cs = cs;
    memset(&bg_pixel_captured, 0, sizeof(bg_pixel_captured));
    test_hooks.on_frame = capture_bg_corner;

    enqueue_input(make_key_event(SDLK_ESCAPE));
    start_app(cs);

    // bg_pixel_captured was filled during the last frame before SDL_Quit
    // Check bottom-right corner — no menu items render there
    cr_assert(color_matches(bg_pixel_captured, 0x33, 0x66, 0x99),
              "background color at bottom-right: expected (51,102,153) got (%d,%d,%d)",
              bg_pixel_captured.r, bg_pixel_captured.g, bg_pixel_captured.b);
}

// ─── Selected item background color renders at expected position ───

static RGB highlight_pixel_captured;

// Capture the pixel at the expected highlight position during a live frame.
static void capture_highlight_pixel(void) {
    SDL_Surface* screen = current_cs->state->screen;
    if (screen && screen->pixels) {
        // Item 0 is selected by default.
        // Menu starts at (left_padding, top_padding) = (10, 10).
        // The highlight rect is blitted at that position.
        // Check center of first menu item area for the highlight color.
        int x = current_cs->config->left_padding + current_cs->config->menu_item_padding + 2;
        int y = current_cs->config->top_padding + current_cs->config->menu_item_padding;
        highlight_pixel_captured = get_pixel_rgb(screen, x, y);
    }
}

static void set_highlight_color(Config* config) {
    config->text_selected_background_color = malloc(sizeof(Color));
    config->text_selected_background_color->r = 0xFF;
    config->text_selected_background_color->g = 0x00;
    config->text_selected_background_color->b = 0x00;
    // Set background to something different so highlight is distinguishable
    config->background_color->r = 0x00;
    config->background_color->g = 0x00;
    config->background_color->b = 0x00;
}

Test(render_tests, selected_item_has_highlight_background) {
    ConfigAndState* cs = setup_with_config(set_highlight_color);

    current_cs = cs;
    memset(&highlight_pixel_captured, 0, sizeof(highlight_pixel_captured));
    test_hooks.on_frame = capture_highlight_pixel;

    enqueue_input(make_key_event(SDLK_ESCAPE));
    start_app(cs);

    SDL_Surface* screen = cs->state->screen;
    int x = cs->config->left_padding + cs->config->menu_item_padding + 2;
    int y = cs->config->top_padding + cs->config->menu_item_padding;

    // The highlight should be red (0xFF, 0x00, 0x00) — not black background
    cr_assert(highlight_pixel_captured.r > 0,
              "selected item highlight should have non-zero red at (%d,%d), got (%d,%d,%d)",
              x, y, highlight_pixel_captured.r, highlight_pixel_captured.g, highlight_pixel_captured.b);
}
