#include "test_helpers.h"

// ─── Wrap around: up from first item goes to last ───

Test(navigation_tests, wrap_around_up_from_first) {
    ConfigAndState* cs = setup_default();

    // Up arrow from Item 0 → wraps to Item 19
    enqueue_input(make_key_event(SDLK_UP));
    enqueue_input(make_key_event(SDLK_RETURN));
    start_app(cs);

    cr_assert_str_eq(captured_stdout, "Item 19\n");
}

// ─── Wrap around: down from last item goes to first ───

Test(navigation_tests, wrap_around_down_from_last) {
    ConfigAndState* cs = setup_default();
    cs->config->start_at_nth = 20; // start at last item (1-indexed)

    // Down arrow from Item 19 → wraps to Item 0
    enqueue_input(make_key_event(SDLK_DOWN));
    enqueue_input(make_key_event(SDLK_RETURN));
    start_app(cs);

    cr_assert_str_eq(captured_stdout, "Item 0\n");
}

// ─── Page down (right arrow) jumps by screen page ───

Test(navigation_tests, page_down_jumps_by_page) {
    ConfigAndState* cs = setup_default();

    // Right arrow pages down by menu_max_items (14 with default 640x480 + 27px font)
    enqueue_input(make_key_event(SDLK_RIGHT));
    enqueue_input(make_key_event(SDLK_RETURN));
    start_app(cs);

    // menu_max_items is calculated during first_render. With 20 items and 14 visible,
    // right arrow moves selection to index 14 (Item 14), clamped to count.
    cr_assert_str_eq(captured_stdout, "Item 14\n");
}

// ─── Single item menu: enter selects it ───

Test(navigation_tests, single_item_menu) {
    const char* items[] = { "OnlyChoice" };
    ConfigAndState* cs = setup_with_items(items, 1);

    enqueue_input(make_key_event(SDLK_RETURN));
    start_app(cs);

    cr_assert_str_eq(captured_stdout, "OnlyChoice\n");
}

// ─── start_at_nth positions selection correctly ───

Test(navigation_tests, start_at_nth_positions_selection) {
    ConfigAndState* cs = setup_default();
    cs->config->start_at_nth = 5; // 1-indexed, so starts at Item 4

    enqueue_input(make_key_event(SDLK_RETURN));
    start_app(cs);

    cr_assert_str_eq(captured_stdout, "Item 4\n");
}

// ─── Joypad D-pad down + A button selects ───

Test(navigation_tests, joypad_dpad_down_and_a_button) {
    ConfigAndState* cs = setup_default();

    enqueue_input(make_joyhat_event(SDL_HAT_DOWN));
    // D-pad centered to stop repeat
    enqueue_input(make_joyhat_event(SDL_HAT_CENTERED));
    // A button (button 0) to confirm
    enqueue_input(make_joybutton_event(0));
    start_app(cs);

    cr_assert_str_eq(captured_stdout, "Item 1\n");
}

// ─── Joypad B button exits without selection ───

Test(navigation_tests, joypad_b_button_exits) {
    ConfigAndState* cs = setup_default();

    enqueue_input(make_joybutton_event(1)); // B button
    start_app(cs);

    cr_assert_str_eq(captured_stdout, "");
}

// ─── Navigate down past visible area triggers viewport scroll ───

Test(navigation_tests, scrolling_past_visible_area) {
    ConfigAndState* cs = setup_default();

    // Navigate down 15 times (past the ~14 visible items)
    for (int i = 0; i < 15; i++) {
        enqueue_input(make_key_event(SDLK_DOWN));
    }
    enqueue_input(make_key_event(SDLK_RETURN));
    start_app(cs);

    cr_assert_str_eq(captured_stdout, "Item 15\n");

    // Verify that the rendered text includes items near the selection,
    // not just the original viewport
    cr_assert(text_was_rendered("Item 15"),
              "Item 15 should be rendered after scrolling");
}
