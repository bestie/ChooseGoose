#include <criterion/criterion.h>
#include "cli_opts.h"

#define program_name "choosegoose"

Config* from_args(int argc, char* argv[]) {
    Config* config = default_config();
    parse_command_line_options(argc, argv, config);
    return config;
}

Test(cli_opts, parse_ints_from_command_line_args) {
    char* argv[] = {
        program_name,
        "--screen-width=801",
        "--screen-height=601",
        "--bits-per-pixel=17",
        "--font-size=25",
        "--top-padding=11",
        "--bottom-padding=11",
        "--left-padding=11",
        "--right-padding=11",
        "--start-at-nth=6",
        "--title-font-size=25",
        "--user-inactivity-timeout-ms=1001",
        "--key-repeat-delay-ms=501",
        "--key-repeat-interval-ms=51",
    };
    Config* config = from_args(14, argv);

    cr_assert_eq(config->screen_width, 801, "Screen width should be 801");
    cr_assert_eq(config->screen_height, 601, "Screen height should be 601");
    cr_assert_eq(config->bits_per_pixel, 17, "Bits per pixel should be 17");
    cr_assert_eq(config->font_size, 25, "Font size should be 25");
    cr_assert_eq(config->top_padding, 11, "Top padding should be 11");
    cr_assert_eq(config->bottom_padding, 11, "Bottom padding should be 11");
    cr_assert_eq(config->left_padding, 11, "Left padding should be 11");
    cr_assert_eq(config->right_padding, 11, "Right padding should be 11");
    cr_assert_eq(config->start_at_nth, 6, "Start at nth should be 6");
    cr_assert_eq(config->title_font_size, 25, "Title font size should be 25");
    cr_assert_eq(config->user_inactivity_timeout_ms, 1001, "User inactivity timeout should be 1001");
    cr_assert_eq(config->key_repeat_delay_ms, 501, "Key repeat delay should be 501");
    cr_assert_eq(config->key_repeat_interval_ms, 51, "Key repeat interval should be 51");
}

Test(cli_opts, parse_booleans_from_command_line_args) {
    char* argv[] = {
        program_name,
        "--hide-file-extensions=true",
    };
    Config* config = from_args(2, argv);

    cr_assert_eq(config->hide_file_extensions, true, "Hide file extensions should be true");

    char* argv2[] = {
        program_name,
        "--hide-file-extensions=false",
    };
    config = from_args(2, argv2);
    cr_assert_eq(config->hide_file_extensions, false, "Hide file extensions should be false");

    char* argv3[] = {
        program_name,
        "--hide-file-extensions=not-a-bool",
    };
    config = from_args(2, argv3);
    cr_assert_eq(config->hide_file_extensions, false, "Hide file extensions should be false");
}

Test(cli_opts, parse_color_from_command_line_args) {
    Config config = *default_config();

    // Simulate command-line arguments
    char *argv[] = {
        program_name,
        "--background-color=FF00FF",
        "--text-color=123456",
        "--text-selected-color=6789AB",
        "--text-selected-background-color=ABCDEF",
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    parse_command_line_options(argc, argv, &config);

    cr_assert_eq(config.background_color.r, 255, "Red component should be 255");
    cr_assert_eq(config.background_color.g, 0, "Green component should be 0");
    cr_assert_eq(config.background_color.b, 255, "Blue component should be 255");

    cr_assert_eq(config.text_color.r, 0x12, "Red component should be 0x12");
    cr_assert_eq(config.text_color.g, 0x34, "Green component should be 0x34");
    cr_assert_eq(config.text_color.b, 0x56, "Blue component should be 0x56");

    cr_assert_eq(config.text_selected_color.r, 0x67, "Red component should be 0x67");
    cr_assert_eq(config.text_selected_color.g, 0x89, "Green component should be 0x89");
    cr_assert_eq(config.text_selected_color.b, 0xAB, "Blue component should be 0xAB");

    cr_assert_eq(config.text_selected_background_color.r, 0xAB, "Red component should be 0xAB");
    cr_assert_eq(config.text_selected_background_color.g, 0xCD, "Green component should be 0xCD");
    cr_assert_eq(config.text_selected_background_color.b, 0xEF, "Blue component should be 0xEF");

}
