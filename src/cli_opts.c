#include "cli_opts.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Color parse_color_from_hex(const char *hex) {
  Color color;
  sscanf(hex, "%2x%2x%2x", &color.r, &color.g, &color.b);
  return color;
}

static Color no_color() {
  Color c = { -1, -1, -1 };
  return c;
}

static bool parsebool(char *string) {
  if (strcmp(string, "true") == 0) {
    return true;
  }
  return false;
}

void config_set_defaults(Config *config) {
  config->screen_width = 640;
  config->screen_height = 480;
  config->bits_per_pixel = 32;
  strcpy(config->title, "");
  strcpy(config->font_filepath, "");
  strcpy(config->background_image_filepath, "");
  config->font_size = 18;
  config->top_padding = 10;
  config->bottom_padding = 10;
  config->left_padding = 20;
  config->right_padding = 0;
  config->start_at_nth = 1;
  config->hide_file_extensions = false;
  config->prefix_with_number = false;
  config->background_color = parse_color_from_hex("00FFFF");
  config->text_color = parse_color_from_hex("000000");
  config->text_selected_color = parse_color_from_hex("FF0000");
  config->text_selected_background_color = no_color();
  config->title_font_size = 21;
  config->user_inactivity_timeout_ms = false;
}

void print_usage() {
  fprintf(stdout, "ChooseGoose is a silly interactive graphical menu program for shell scripters.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Given an input list of new-line separated items on stdin, "
                  "it will render a silly interactive menu and allow the user "
                  "to make a selection.\n");
  fprintf(stdout, "The selection is then written to standard out.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Keybindings:\n");
  fprintf(stdout, "  Up/Down arrow:   keys, menu up/down\n");
  fprintf(stdout, "  Left arrow:      page up\n");
  fprintf(stdout, "  Right arrow:     page down\n");
  fprintf(stdout, "  Enter:           confirm selection\n");
  fprintf(stdout, "  Esc:             exit(0) with no selection\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Joypad button bindings:\n");
  fprintf(stdout, "  Up/Down:         menu up/down\n");
  fprintf(stdout, "  Left/Right       page up/down\n");
  fprintf(stdout, "  Start:           confirm selection\n");
  fprintf(stdout, "  Menu:            exit(0) with no selection\n");
  fprintf(stdout, "  A:               confirm selection\n");
  fprintf(stdout, "  B:               exit(0) with no selection\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Examples:\n");
  fprintf(stdout, "  Play a video file from the current directory\n");
  fprintf(stdout, "    ls -1 *.mp4 | choosegoose --background-color=000000 | xargs vlc --fullscreen \n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  Play a video file from a list\n");
  fprintf(stdout, "    movies.txt | choosegoose --background-color=000000 | xargs vlc --fullscreen \n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  As above, but capture the selection in a bash variable\n");
  fprintf(stdout, "  selection=$(cat movies.txt | choosegoose)\n");
  fprintf(stdout, "  vlc --fullscreen \"$selection\"\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Usage: choosegoose --option=value\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "  --help                                   In pixels. Display this message.\n");
  fprintf(stdout, "  --screen-width=N                         In pixels. Default 640.\n");
  fprintf(stdout, "  --screen-height=N                        In pixels. Default 480.\n");
  fprintf(stdout, "  --bits-per-pixel=N                       In pixels. Default 32.\n");
  fprintf(stdout, "  --title=TITLE                            Title text appears with extra padding and can have a different font size.\n");
  fprintf(stdout, "  --font=FILEPATH                          Path to a custom true type font file used for all text.\n");
  fprintf(stdout, "  --background-image=FILEPATH|DEFAULT      Path to a PNG image or DEFAULT for the built-in goose.\n");
  fprintf(stdout, "  --font-size=N                            In pixels.\n");
  fprintf(stdout, "  --title-font-size=N                      In pixels.\n");
  fprintf(stdout, "  --top-padding=N                          In pixels.\n");
  fprintf(stdout, "  --bottom-padding=N                       In pixels.\n");
  fprintf(stdout, "  --left-padding=N                         In pixels.\n");
  fprintf(stdout, "  --right-padding=N                        In pixels.\n");
  fprintf(stdout, "  --start-at-nth=N                         List item to start menu from; default 1 (first), set to 0 for no initial selection.\n");
  fprintf(stdout, "  --user-inactivity-timeout-ms=N           Quits after N milliseconds if the user has not made an input. Exits with status 124 like GNU timeout.\n");
  fprintf(stdout, "  --hide-file-extensions=true|false        When using files as input and output, extensions can be hidden from the user.\n");
  fprintf(stdout, "  --prefix-with-number=true|false          Menu items are prepended with their list order number starting at 1.\n");
  fprintf(stdout, "  --background-color=RRGGBB                Visible when no background image is set and through transparent PNG regions.\n");
  fprintf(stdout, "  --text-color=RRGGBB                      \n");
  fprintf(stdout, "  --text-selected-color=RRGGBB             A different color for the selected item text.\n");
  fprintf(stdout, "  --text-selected-background-color=RRGGBB  A solid background color for the selected item text.\n");
  fprintf(stdout, "  --log-file=FILEPATH|stderr|stdout        Log debug events to a file, stderr or stdout, though stdout is probably not a good idea.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  Colors are CSS-style, 6 digit hexadecimal but without the leading '#' character\n");
  fprintf(stdout, "  --background-color=000000 for a black background\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Author:  Stephen Best\n");
  fprintf(stdout, "Project: https://github.com/bestie/ChooseGoose\n");
  fprintf(stdout, "\n");
}

void parse_command_line_options(int argc, char **argv, Config *config) {
  static struct option long_options[] = {
      {"help", no_argument, 0, 0},
      {"screen-width", required_argument, 0, 0},
      {"screen-height", required_argument, 0, 0},
      {"bits-per-pixel", required_argument, 0, 0},
      {"title", required_argument, 0, 0},
      {"font", required_argument, 0, 0},
      {"background-image", required_argument, 0, 0},
      {"font-size", required_argument, 0, 0},
      {"top-padding", required_argument, 0, 0},
      {"bottom-padding", required_argument, 0, 0},
      {"left-padding", required_argument, 0, 0},
      {"right-padding", required_argument, 0, 0},
      {"text-selected-background-color", required_argument, 0, 0},
      {"start-at-nth", required_argument, 0, 0},
      {"hide-file-extensions", required_argument, 0, 0},
      {"prefix-with-number", required_argument, 0, 0},
      {"background-color", required_argument, 0, 0},
      {"text-color", required_argument, 0, 0},
      {"text-selected-color", required_argument, 0, 0},
      {"title-font-size", required_argument, 0, 0},
      {"log-file", required_argument, 0, 0},
      {"user-inactivity-timeout-ms", required_argument, 0, 0},
      {0, 0, 0, 0}};

  bool debug = false;
  if (getenv("DEBUG") && strcmp(getenv("DEBUG"), "1") == 0) {
    debug = true;
    fprintf(stderr, "ChooseGoose DEBUG enabled, DEBUG env var is set to `1`");
  }

  int option_index = 0;
  while (getopt_long(argc, argv, "", long_options, &option_index) != -1) {

    if (debug) {
      fprintf(stderr, "ChooseGoose DEBUG option_index: `%d` name: `%s` value: `%s`\n", option_index, long_options[option_index].name, optarg);
    }

    switch (option_index) {
    case 0:
      print_usage();
      exit(EXIT_FAILURE);
    case 1:
      config->screen_width = strtol(optarg, NULL, 10);
      break;
    case 2:
      config->screen_height = strtol(optarg, NULL, 10);
      break;
    case 3:
      config->bits_per_pixel = strtol(optarg, NULL, 10);
      break;
    case 4:
      strncpy(config->title, optarg, 255);
      break;
    case 5:
      strncpy(config->font_filepath, optarg, 255);
      break;
    case 6:
      strncpy(config->background_image_filepath, optarg, 255);
      break;
    case 7:
      config->font_size = strtol(optarg, NULL, 10);
      break;
    case 8:
      config->top_padding = strtol(optarg, NULL, 10);
      break;
    case 9:
      config->bottom_padding = strtol(optarg, NULL, 10);
      break;
    case 10:
      config->left_padding = strtol(optarg, NULL, 10);
      break;
    case 11:
      config->right_padding = strtol(optarg, NULL, 10);
      break;
    case 12:
      config->text_selected_background_color = parse_color_from_hex(optarg);
      break;
    case 13:
      config->start_at_nth = strtol(optarg, NULL, 10);
      break;
    case 14:
      config->hide_file_extensions = parsebool(optarg);
      break;
    case 15:
      config->prefix_with_number = parsebool(optarg);
      break;
    case 16:
      config->background_color = parse_color_from_hex(optarg);
      break;
    case 17:
      config->text_color = parse_color_from_hex(optarg);
      break;
    case 18:
      config->text_selected_color = parse_color_from_hex(optarg);
      break;
    case 19:
      config->title_font_size = strtol(optarg, NULL, 10);
      break;
    case 20:
      strncpy(config->log_filepath, optarg, 255);
      break;
    case 21:
      config->user_inactivity_timeout_ms = strtol(optarg, NULL, 10);
      break;
    default:
      print_usage();
      exit(EXIT_FAILURE);
    }
  }
}
