#include "test_helpers.h"

// ─── Typing a letter shows the filter bar ───

Test(filter_tests, typing_shows_filter_bar) {
    const char* items[] = { "Alpha", "Beta", "Gamma", "Delta" };
    ConfigAndState* cs = setup_with_items(items, 4);

    // Type 'a', then confirm
    enqueue_input(make_key_event(SDLK_a));
    enqueue_input(make_key_event(SDLK_RETURN));
    start_app(cs);

    // The filter bar renders as " > a_"
    cr_assert(text_was_rendered(" > a_"),
              "filter bar should show ' > a_'");
}

// ─── Filter reduces visible items ───

Test(filter_tests, filter_reduces_visible_items) {
    const char* items[] = { "Apple", "Banana", "Apricot", "Cherry" };
    ConfigAndState* cs = setup_with_items(items, 4);

    // Type 'ap' to filter to Apple and Apricot, then enter to select first match
    enqueue_input(make_key_event(SDLK_a));
    enqueue_input(make_key_event(SDLK_p));
    enqueue_input(make_key_event(SDLK_RETURN));
    start_app(cs);

    cr_assert_str_eq(captured_stdout, "Apple\n");
}

// ─── Filter then navigate selects correct item ───

Test(filter_tests, filter_then_navigate_selects_filtered_item) {
    const char* items[] = { "Apple", "Banana", "Apricot", "Cherry" };
    ConfigAndState* cs = setup_with_items(items, 4);

    // Type 'ap', down arrow (to Apricot), enter
    enqueue_input(make_key_event(SDLK_a));
    enqueue_input(make_key_event(SDLK_p));
    enqueue_input(make_key_event(SDLK_DOWN));
    enqueue_input(make_key_event(SDLK_RETURN));
    start_app(cs);

    cr_assert_str_eq(captured_stdout, "Apricot\n");
}

// ─── Backspace removes last character from filter ───

Test(filter_tests, backspace_removes_last_filter_char) {
    const char* items[] = { "Apple", "Banana", "Apricot", "Cherry" };
    ConfigAndState* cs = setup_with_items(items, 4);

    // Type 'ch', backspace (back to 'c'), then 'h' again, enter → Cherry
    enqueue_input(make_key_event(SDLK_c));
    enqueue_input(make_key_event(SDLK_h));
    enqueue_input(make_key_event(SDLK_BACKSPACE));
    // After backspace, filter is "c" — matches Cherry and Apricot (has 'c')
    // Type 'h' to narrow to "ch" → Cherry
    enqueue_input(make_key_event(SDLK_h));
    enqueue_input(make_key_event(SDLK_RETURN));
    start_app(cs);

    cr_assert_str_eq(captured_stdout, "Cherry\n");
}

// ─── Filter is case-insensitive ───

Test(filter_tests, filter_is_case_insensitive) {
    const char* items[] = { "UPPER", "lower", "MiXeD" };
    ConfigAndState* cs = setup_with_items(items, 3);

    // Type 'mix' (lowercase), should match 'MiXeD'
    enqueue_input(make_key_event(SDLK_m));
    enqueue_input(make_key_event(SDLK_i));
    enqueue_input(make_key_event(SDLK_x));
    enqueue_input(make_key_event(SDLK_RETURN));
    start_app(cs);

    cr_assert_str_eq(captured_stdout, "MiXeD\n");
}
