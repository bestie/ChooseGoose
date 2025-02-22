#include <criterion/criterion.h>
#include "cli_opts.h"

Test(cli_opts, parse_color_through_command_line) {
    Config config;
    /*config_set_defaults(&config);*/

    // Simulate command-line arguments
    char *argv[] = {
        "program_name",
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
